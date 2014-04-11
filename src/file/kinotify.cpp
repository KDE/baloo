/* This file is part of the KDE libraries
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2012 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kinotify.h"
#include "optimizedbytearray.h"

#include <QSocketNotifier>
#include <QHash>
#include <QDirIterator>
#include <QFile>
#include <QLinkedList>
#include <QTimer>
#include <QPair>

#include <kdebug.h>

#include <sys/inotify.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>


namespace
{
const int EVENT_STRUCT_SIZE = sizeof(struct inotify_event);

// we need one event to fit into the buffer, the problem is that the name
// is a variable length array
const int EVENT_BUFFER_SIZE = EVENT_STRUCT_SIZE + 1024 * 16;

QByteArray stripTrailingSlash(const QByteArray& path)
{
    QByteArray p(path);
    if (p.endsWith('/'))
        p.truncate(p.length() - 1);
    return p;
}

QByteArray concatPath(const QByteArray& p1, const QByteArray& p2)
{
    QByteArray p(p1);
    if (p.isEmpty() || p[p.length() - 1] != '/')
        p.append('/');
    p.append(p2);
    return p;
}
}

class KInotify::Private
{
public:
    Private(KInotify* parent)
        : userLimitReachedSignaled(false),
          m_inotifyFd(-1),
          m_notifier(0),
          q(parent) {
    }

    ~Private() {
        close();
        while (!dirIterators.isEmpty())
            delete dirIterators.takeFirst();
    }

    QHash<int, QPair<QByteArray, WatchFlags> > cookies;
    QTimer cookieExpireTimer;
    // This variable is set to true if the watch limit is reached, and reset when it is raised
    bool userLimitReachedSignaled;

    // url <-> wd mappings
    // Read the documentation fo OptimizedByteArray to understand why have a cache
    QHash<int, OptimizedByteArray> watchPathHash;
    QHash<OptimizedByteArray, int> pathWatchHash;
    QSet<QByteArray> pathCache;

    /// A list of all the current dirIterators
    QLinkedList<QDirIterator*> dirIterators;

    unsigned char eventBuffer[EVENT_BUFFER_SIZE];

    // FIXME: only stored from the last addWatch call
    WatchEvents mode;
    WatchFlags flags;

    int inotify() {
        if (m_inotifyFd < 0) {
            open();
        }
        return m_inotifyFd;
    }

    void close() {
        qDebug();
        delete m_notifier;
        m_notifier = 0;

        ::close(m_inotifyFd);
        m_inotifyFd = -1;
    }

    bool addWatch(const QString& path) {
        WatchEvents newMode = mode;
        WatchFlags newFlags = flags;
        //Encode the path
        if (!q->filterWatch(path, newMode, newFlags)) {
            return false;
        }
        // we always need the unmount event to maintain our path hash
        const int mask = newMode | newFlags | EventUnmount | FlagExclUnlink;

        const QByteArray encpath = QFile::encodeName(path);
        int wd = inotify_add_watch(inotify(), encpath.data(), mask);
        if (wd > 0) {
//             qDebug() << "Successfully added watch for" << path << watchPathHash.count();
            OptimizedByteArray normalized(stripTrailingSlash(encpath), pathCache);
            watchPathHash.insert(wd, normalized);
            pathWatchHash.insert(normalized, wd);
            return true;
        } else {
            qDebug() << "Failed to create watch for" << path;
            //If we could not create the watch because we have hit the limit, try raising it.
            if (errno == ENOSPC) {
                //If we can't, fall back to signalling
                qDebug() << "User limit reached. Count: " << watchPathHash.count();
                userLimitReachedSignaled = true;
                Q_EMIT q->watchUserLimitReached(path);
            }
            return false;
        }
    }

    void removeWatch(int wd) {
        qDebug() << wd << watchPathHash[wd].toByteArray();
        pathWatchHash.remove(watchPathHash.take(wd));
        inotify_rm_watch(inotify(), wd);
    }

    /**
     * Add one watch and call oneself asynchronously
     */
    bool _k_addWatches() {
        bool addedWatchSuccessfully = false;

        //Do nothing if the inotify user limit has been signaled.
        //This means that we will not empty the dirIterators while
        //waiting for authentication.
        if (userLimitReachedSignaled) {
            return false;
        }

        if (!dirIterators.isEmpty()) {
            QDirIterator* it = dirIterators.front();
            if (it->hasNext()) {
                QString dirPath = it->next();
                if (addWatch(dirPath)) {
                    // IMPORTANT: We do not follow system links. Ever.
                    QDirIterator* iter = new QDirIterator(dirPath, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
                    dirIterators.prepend(iter);
                    addedWatchSuccessfully = true;
                }
            } else {
                delete dirIterators.takeFirst();
            }
        }

        // asynchronously add the next batch
        if (!dirIterators.isEmpty()) {
            QMetaObject::invokeMethod(q, "_k_addWatches", Qt::QueuedConnection);
        }
        else {
            Q_EMIT q->installedWatches();
        }

        return addedWatchSuccessfully;
    }

private:
    void open() {
        qDebug();
        m_inotifyFd = inotify_init();
        delete m_notifier;
        if (m_inotifyFd > 0) {
            fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
            qDebug() << "Successfully opened connection to inotify:" << m_inotifyFd;
            m_notifier = new QSocketNotifier(m_inotifyFd, QSocketNotifier::Read);
            connect(m_notifier, SIGNAL(activated(int)), q, SLOT(slotEvent(int)));
        }
    }

    int m_inotifyFd;
    QSocketNotifier* m_notifier;

    KInotify* q;
};


KInotify::KInotify(QObject* parent)
    : QObject(parent),
      d(new Private(this))
{
    // 1 second is more than enough time for the EventMoveTo event to occur
    // after the EventMoveFrom event has occurred
    d->cookieExpireTimer.setInterval(1000);
    d->cookieExpireTimer.setSingleShot(true);
    connect(&d->cookieExpireTimer, SIGNAL(timeout()), this, SLOT(slotClearCookies()));
}


KInotify::~KInotify()
{
    delete d;
}


bool KInotify::available() const
{
    if (d->inotify() > 0) {
        // trueg: Copied from KDirWatch.
        struct utsname uts;
        int major, minor, patch = 0;
        if (uname(&uts) < 0) {
            return false; // *shrug*
        } else if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) != 3) {
            //Kernels > 3.0 can in principle have two-number versions.
            if (sscanf(uts.release, "%d.%d", &major, &minor) != 2)
                return false; // *shrug*
        } else if (major * 1000000 + minor * 1000 + patch < 2006014) { // <2.6.14
            qDebug() << "Can't use INotify, Linux kernel too old";
            return false;
        }

        return true;
    } else {
        return false;
    }
}


bool KInotify::watchingPath(const QString& path) const
{
    const QByteArray p(stripTrailingSlash(QFile::encodeName(path)));
    return d->pathWatchHash.contains(OptimizedByteArray(p, d->pathCache));
}

void KInotify::resetUserLimit()
{
    d->userLimitReachedSignaled = false;
}

bool KInotify::addWatch(const QString& path, WatchEvents mode, WatchFlags flags)
{
    qDebug() << path;

    d->mode = mode;
    d->flags = flags;
    //If the inotify user limit has been signaled,
    //just queue this folder for watching.
    if (d->userLimitReachedSignaled) {
        QDirIterator* iter = new QDirIterator(path, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        d->dirIterators.prepend(iter);
        return false;
    }

    if (!(d->addWatch(path)))
        return false;
    QDirIterator* iter = new QDirIterator(path, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    d->dirIterators.prepend(iter);
    return d->_k_addWatches();
}


bool KInotify::removeWatch(const QString& path)
{
    // Stop all of the dirIterators which contain path
    QMutableLinkedListIterator<QDirIterator*> iter(d->dirIterators);
    while (iter.hasNext()) {
        QDirIterator* dirIter = iter.next();
        if (dirIter->path().startsWith(path)) {
            iter.remove();
            delete dirIter;
        }
    }

    // Remove all the watches
    QByteArray encodedPath(QFile::encodeName(path));
    QHash<int, OptimizedByteArray>::iterator it = d->watchPathHash.begin();
    while (it != d->watchPathHash.end()) {
        if (it.value().toByteArray().startsWith(encodedPath)) {
            inotify_rm_watch(d->inotify(), it.key());
            d->pathWatchHash.remove(it.value());
            it = d->watchPathHash.erase(it);
        } else {
            ++it;
        }
    }
    return true;
}


bool KInotify::filterWatch(const QString& path, WatchEvents& modes, WatchFlags& flags)
{
    Q_UNUSED(path);
    Q_UNUSED(modes);
    Q_UNUSED(flags);
    return true;
}


void KInotify::slotEvent(int socket)
{
    // read at least one event
    const int len = read(socket, d->eventBuffer, EVENT_BUFFER_SIZE);
    int i = 0;
    while (i < len && len - i >= EVENT_STRUCT_SIZE) {
        const struct inotify_event* event = (struct inotify_event*)&d->eventBuffer[i];

        QByteArray path;

        // Overflow happens sometimes if we process the events too slowly
        if (event->wd < 0 && (event->mask & EventQueueOverflow)) {
            qWarning() << "Inotify - too many event - Overflowed";
            return;
        }

        // the event name only contains an interesting value if we get an event for a file/folder inside
        // a watched folder. Otherwise we should ignore it
        if (event->mask & (EventDeleteSelf | EventMoveSelf)) {
            path = d->watchPathHash.value(event->wd).toByteArray();
        } else {
            // we cannot use event->len here since it contains the size of the buffer and not the length of the string
            const QByteArray eventName = QByteArray::fromRawData(event->name, qstrnlen(event->name, event->len));
            const QByteArray hashedPath = d->watchPathHash.value(event->wd).toByteArray();
            path = concatPath(hashedPath, eventName);
        }

        Q_ASSERT(!path.isEmpty() || event->mask & EventIgnored);
        Q_ASSERT(path != "/" || event->mask & EventIgnored  || event->mask & EventUnmount);

        // now signal the event
        if (event->mask & EventAccess) {
//            qDebug() << path << "EventAccess";
            Q_EMIT accessed(QFile::decodeName(path));
        }
        if (event->mask & EventAttributeChange) {
//            qDebug() << path << "EventAttributeChange";
            Q_EMIT attributeChanged(QFile::decodeName(path));
        }
        if (event->mask & EventCloseWrite) {
//            qDebug() << path << "EventCloseWrite";
            Q_EMIT closedWrite(QFile::decodeName(path));
        }
        if (event->mask & EventCloseRead) {
//            qDebug() << path << "EventCloseRead";
            Q_EMIT closedRead(QFile::decodeName(path));
        }
        if (event->mask & EventCreate) {
//            qDebug() << path << "EventCreate";
            if (event->mask & IN_ISDIR) {
                // FIXME: store the mode and flags somewhere
                addWatch(path, d->mode, d->flags);
            }
            Q_EMIT created(QFile::decodeName(path), event->mask & IN_ISDIR);
        }
        if (event->mask & EventDeleteSelf) {
            qDebug() << path << "EventDeleteSelf";
            d->removeWatch(event->wd);
            Q_EMIT deleted(QFile::decodeName(path), event->mask & IN_ISDIR);
        }
        if (event->mask & EventDelete) {
//            qDebug() << path << "EventDelete";
            // we watch all folders recursively. Thus, folder removing is reported in DeleteSelf.
            if (!(event->mask & IN_ISDIR))
                Q_EMIT deleted(QFile::decodeName(path), false);
        }
        if (event->mask & EventModify) {
//            qDebug() << path << "EventModify";
            Q_EMIT modified(QFile::decodeName(path));
        }
        if (event->mask & EventMoveSelf) {
//            qDebug() << path << "EventMoveSelf";
            qWarning() << "EventMoveSelf: THIS CASE IS NOT HANDLED PROPERLY!";
        }
        if (event->mask & EventMoveFrom) {
//            qDebug() << path << "EventMoveFrom";
            d->cookies[event->cookie] = qMakePair(path, WatchFlags(event->mask));
            d->cookieExpireTimer.start();
        }
        if (event->mask & EventMoveTo) {
            // check if we have a cookie for this one
            if (d->cookies.contains(event->cookie)) {
                const QByteArray oldPath = d->cookies.take(event->cookie).first;

                // update the path cache
                if (event->mask & IN_ISDIR) {
                    OptimizedByteArray optimOldPath(oldPath, d->pathCache);
                    QHash<OptimizedByteArray, int>::iterator it = d->pathWatchHash.find(optimOldPath);
                    if (it != d->pathWatchHash.end()) {
                        qDebug() << oldPath << path;
                        const int wd = it.value();
                        OptimizedByteArray optimPath(path, d->pathCache);
                        d->watchPathHash[wd] = optimPath;
                        d->pathWatchHash.erase(it);
                        d->pathWatchHash.insert(optimPath, wd);
                    }
                }
//                qDebug() << oldPath << "EventMoveTo" << path;
                Q_EMIT moved(QFile::decodeName(oldPath), QFile::decodeName(path));
            } else {
                qDebug() << "No cookie for move information of" << path << "simulating new file event";
                Q_EMIT created(path, event->mask & IN_ISDIR);

                // also simulate a closed write since that is what triggers indexing of files in the file watcher
                if (!(event->mask & IN_ISDIR)) {
                    Q_EMIT closedWrite(path);
                }
            }
        }
        if (event->mask & EventOpen) {
//            qDebug() << path << "EventOpen";
            Q_EMIT opened(QFile::decodeName(path));
        }
        if (event->mask & EventUnmount) {
//            qDebug() << path << "EventUnmount. removing from path hash";
            if (event->mask & IN_ISDIR) {
                d->removeWatch(event->wd);
            }
            // This is present because a unmount event is sent by inotify after unmounting, by
            // which time the watches have already been removed.
            if (path != "/") {
                Q_EMIT unmounted(QFile::decodeName(path));
            }
        }
        if (event->mask & EventQueueOverflow) {
            // This should not happen since we grab all events as soon as they arrive
            qDebug() << path << "EventQueueOverflow";
//            Q_EMIT queueOverflow();
        }
        if (event->mask & EventIgnored) {
//             qDebug() << path << "EventIgnored";
        }

        i += EVENT_STRUCT_SIZE + event->len;
    }

    if (len < 0) {
        qDebug() << "Failed to read event.";
    }
}

void KInotify::slotClearCookies()
{
    QHashIterator<int, QPair<QByteArray, WatchFlags> > it(d->cookies);
    while (it.hasNext()) {
        it.next();
        removeWatch(it.value().first);
        Q_EMIT deleted(QFile::decodeName(it.value().first), it.value().second & IN_ISDIR);
    }

    d->cookies.clear();
}

#include "moc_kinotify.cpp"
