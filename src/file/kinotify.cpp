/* This file is part of the KDE libraries
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2012-2014 Vishesh Handa <vhanda@kde.org>

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
#include "fileindexerconfig.h"
#include "filtereddiriterator.h"
#include "baloodebug.h"

#include <QSocketNotifier>
#include <QHash>
#include <QDirIterator>
#include <QFile>
#include <QLinkedList>
#include <QTimer>
#include <QPair>

#include <sys/inotify.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

namespace
{
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
    if (p.isEmpty() || (!p2.isEmpty() && p[p.length() - 1] != '/'))
        p.append('/');
    p.append(p2);
    return p;
}
}

class KInotify::Private
{
public:
    Private(KInotify* parent)
        : userLimitReachedSignaled(false)
        , config(nullptr)
        , m_dirIter(nullptr)
        , m_inotifyFd(-1)
        , m_notifier(nullptr)
        , q(parent) {
    }

    ~Private() {
        close();
        delete m_dirIter;
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

    Baloo::FileIndexerConfig* config;
    QStringList m_paths;
    Baloo::FilteredDirIterator* m_dirIter;

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
        delete m_notifier;
        m_notifier = nullptr;

        ::close(m_inotifyFd);
        m_inotifyFd = -1;
    }

    bool addWatch(const QString& path) {
        WatchEvents newMode = mode;
        WatchFlags newFlags = flags;

        // we always need the unmount event to maintain our path hash
        const int mask = newMode | newFlags | EventUnmount | FlagExclUnlink;

        const QByteArray encpath = QFile::encodeName(path);
        int wd = inotify_add_watch(inotify(), encpath.data(), mask);
        if (wd > 0) {
//             qCDebug(BALOO) << "Successfully added watch for" << path << watchPathHash.count();
            OptimizedByteArray normalized(stripTrailingSlash(encpath), pathCache);
            watchPathHash.insert(wd, normalized);
            pathWatchHash.insert(normalized, wd);
            return true;
        } else {
            qCDebug(BALOO) << "Failed to create watch for" << path << strerror(errno);
            //If we could not create the watch because we have hit the limit, try raising it.
            if (errno == ENOSPC) {
                //If we can't, fall back to signalling
                qCDebug(BALOO) << "User limit reached. Count: " << watchPathHash.count();
                userLimitReachedSignaled = true;
                Q_EMIT q->watchUserLimitReached(path);
            }
            return false;
        }
    }

    void removeWatch(int wd) {
        pathWatchHash.remove(watchPathHash.take(wd));
        inotify_rm_watch(inotify(), wd);
    }

    /**
     * Add one watch and call oneself asynchronously
     */
    bool _k_addWatches() {
        bool addedWatchSuccessfully = false;

        //Do nothing if the inotify user limit has been signaled.
        if (userLimitReachedSignaled) {
            return false;
        }

        // It is much faster to add watches in batches instead of adding each one
        // asynchronously. Try out the inotify test to compare results.
        for (int i = 0; i < 1000; i++) {
            if (!m_dirIter || m_dirIter->next().isEmpty()) {
                if (!m_paths.isEmpty()) {
                    delete m_dirIter;
                    m_dirIter = new Baloo::FilteredDirIterator(config, m_paths.takeFirst(), Baloo::FilteredDirIterator::DirsOnly);
                } else {
                    delete m_dirIter;
                    m_dirIter = nullptr;
                    break;
                }
            } else {
                QString path = m_dirIter->filePath();
                addedWatchSuccessfully = addWatch(path);
            }
        }

        // asynchronously add the next batch
        if (m_dirIter) {
            QMetaObject::invokeMethod(q, "_k_addWatches", Qt::QueuedConnection);
        }
        else {
            Q_EMIT q->installedWatches();
        }

        return addedWatchSuccessfully;
    }

private:
    void open() {
        m_inotifyFd = inotify_init();
        delete m_notifier;
        if (m_inotifyFd > 0) {
            fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
            m_notifier = new QSocketNotifier(m_inotifyFd, QSocketNotifier::Read);
            connect(m_notifier, &QSocketNotifier::activated, q, &KInotify::slotEvent);
        } else {
            Q_ASSERT_X(0, "kinotify", "Failed to initialize inotify");
        }
    }

    int m_inotifyFd;
    QSocketNotifier* m_notifier;

    KInotify* q;
};


KInotify::KInotify(Baloo::FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->config = config;
    // 1 second is more than enough time for the EventMoveTo event to occur
    // after the EventMoveFrom event has occurred
    d->cookieExpireTimer.setInterval(1000);
    d->cookieExpireTimer.setSingleShot(true);
    connect(&d->cookieExpireTimer, &QTimer::timeout, this, &KInotify::slotClearCookies);
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
            qCDebug(BALOO) << "Can't use INotify, Linux kernel too old";
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
//    qCDebug(BALOO) << path;

    d->mode = mode;
    d->flags = flags;
    // If the inotify user limit has been signaled,
    // just queue this folder for watching.
    if (d->m_dirIter || d->userLimitReachedSignaled) {
        d->m_paths << path;
        return false;
    }

    d->m_dirIter = new Baloo::FilteredDirIterator(d->config, path, Baloo::FilteredDirIterator::DirsOnly);
    return d->_k_addWatches();
}


bool KInotify::removeWatch(const QString& path)
{
    // Stop all of the iterators which contain path
    QMutableListIterator<QString> iter(d->m_paths);
    while (iter.hasNext()) {
        if (iter.next().startsWith(path)) {
            iter.remove();
        }
    }
    if (d->m_dirIter) {
        if (d->m_dirIter->filePath().startsWith(path)) {
            delete d->m_dirIter;
            d->m_dirIter = nullptr;
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


void KInotify::slotEvent(int socket)
{
    int avail;
    if (ioctl(socket, FIONREAD, &avail) == EINVAL) {
        qCDebug(BALOO) << "Did not receive an entire inotify event.";
        return;
    }

    char* buffer = (char*)malloc(avail);

    const int len = read(socket, buffer, avail);
    Q_ASSERT(len == avail);

    int i = 0;
    while (i < len) {
        const struct inotify_event* event = (struct inotify_event*)&buffer[i];

        QByteArray path;

        // Overflow happens sometimes if we process the events too slowly
        if (event->wd < 0 && (event->mask & EventQueueOverflow)) {
            qWarning() << "Inotify - too many event - Overflowed";
            free(buffer);
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
//            qCDebug(BALOO) << path << "EventAccess";
            Q_EMIT accessed(QFile::decodeName(path));
        }
        if (event->mask & EventAttributeChange) {
//            qCDebug(BALOO) << path << "EventAttributeChange";
            Q_EMIT attributeChanged(QFile::decodeName(path));
        }
        if (event->mask & EventCloseWrite) {
//            qCDebug(BALOO) << path << "EventCloseWrite";
            Q_EMIT closedWrite(QFile::decodeName(path));
        }
        if (event->mask & EventCloseRead) {
//            qCDebug(BALOO) << path << "EventCloseRead";
            Q_EMIT closedRead(QFile::decodeName(path));
        }
        if (event->mask & EventCreate) {
//            qCDebug(BALOO) << path << "EventCreate";
            if (event->mask & IN_ISDIR) {
                // FIXME: store the mode and flags somewhere
                addWatch(QString::fromUtf8(path), d->mode, d->flags);
            }
            Q_EMIT created(QFile::decodeName(path), event->mask & IN_ISDIR);
        }
        if (event->mask & EventDeleteSelf) {
//            qCDebug(BALOO) << path << "EventDeleteSelf";
            d->removeWatch(event->wd);
            Q_EMIT deleted(QFile::decodeName(path), true);
        }
        if (event->mask & EventDelete) {
//            qCDebug(BALOO) << path << "EventDelete";
            // we watch all folders recursively. Thus, folder removing is reported in DeleteSelf.
            if (!(event->mask & IN_ISDIR))
                Q_EMIT deleted(QFile::decodeName(path), false);
        }
        if (event->mask & EventModify) {
//            qCDebug(BALOO) << path << "EventModify";
            Q_EMIT modified(QFile::decodeName(path));
        }
        if (event->mask & EventMoveSelf) {
//            qCDebug(BALOO) << path << "EventMoveSelf";
            qWarning() << "EventMoveSelf: THIS CASE IS NOT HANDLED PROPERLY!";
        }
        if (event->mask & EventMoveFrom) {
//            qCDebug(BALOO) << path << "EventMoveFrom";
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
//                        qCDebug(BALOO) << oldPath << path;
                        const int wd = it.value();
                        OptimizedByteArray optimPath(path, d->pathCache);
                        d->watchPathHash[wd] = optimPath;
                        d->pathWatchHash.erase(it);
                        d->pathWatchHash.insert(optimPath, wd);
                    }
                }
//                qCDebug(BALOO) << oldPath << "EventMoveTo" << path;
                Q_EMIT moved(QFile::decodeName(oldPath), QFile::decodeName(path));
            } else {
//                qCDebug(BALOO) << "No cookie for move information of" << path << "simulating new file event";
                Q_EMIT created(QString::fromUtf8(path), event->mask & IN_ISDIR);
            }
        }
        if (event->mask & EventOpen) {
//            qCDebug(BALOO) << path << "EventOpen";
            Q_EMIT opened(QFile::decodeName(path));
        }
        if (event->mask & EventUnmount) {
//            qCDebug(BALOO) << path << "EventUnmount. removing from path hash";
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
            qCDebug(BALOO) << path << "EventQueueOverflow";
//            Q_EMIT queueOverflow();
        }
        if (event->mask & EventIgnored) {
//             qCDebug(BALOO) << path << "EventIgnored";
        }

        i += sizeof(struct inotify_event) + event->len;
    }

    if (len < 0) {
        qCDebug(BALOO) << "Failed to read event.";
    }

    free(buffer);
}

void KInotify::slotClearCookies()
{
    QHashIterator<int, QPair<QByteArray, WatchFlags> > it(d->cookies);
    while (it.hasNext()) {
        it.next();
        removeWatch(QString::fromUtf8(it.value().first));
        Q_EMIT deleted(QFile::decodeName(it.value().first), it.value().second & IN_ISDIR);
    }

    d->cookies.clear();
}

#include "moc_kinotify.cpp"
