/*
   Copyright (C) 2008-2010 by Sebastian Trueg <trueg at kde.org>
   Copyright (C) 2012-2013 by Vishesh Handa <me@vhanda.in>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kio_search.h"

#include "query.h"
#include "resultiterator.h"

#include <KUser>
#include <KDebug>
#include <KAboutData>
#include <KApplication>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KCmdLineArgs>
#include <KMimeType>

#include <KIO/Job>
#include <KIO/NetAccess>
#include <kde_file.h>

using namespace Baloo;

namespace
{

KIO::UDSEntry statSearchFolder(const KUrl& url)
{
    KIO::UDSEntry uds;
    uds.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
    uds.insert(KIO::UDSEntry::UDS_ICON_OVERLAY_NAMES, QLatin1String("nepomuk"));
    uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Query folder"));
    uds.insert(KIO::UDSEntry::UDS_URL, url.url());

    QString title = url.queryItemValue(QLatin1String("title"));
    if (title.size()) {
        uds.insert(KIO::UDSEntry::UDS_NAME, title);
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, title);
    }

    return uds;
}

bool isRootUrl(const KUrl& url)
{
    const QString path = url.path(KUrl::RemoveTrailingSlash);
    return (!url.hasQuery() &&
            (path.isEmpty() || path == QLatin1String("/")));
}

}

SearchProtocol::SearchProtocol(const QByteArray& poolSocket, const QByteArray& appSocket)
    : KIO::SlaveBase("baloosearch", poolSocket, appSocket)
{
}


SearchProtocol::~SearchProtocol()
{
}

void SearchProtocol::listDir(const KUrl& url)
{
    // list the root folder
    if (isRootUrl(url)) {
        listEntry(KIO::UDSEntry(), true);
        finished();
    }

    Query q = Query::fromSearchUrl(url);
    ResultIterator it = q.exec();

    while (it.next()) {
        KIO::UDSEntry uds;
        const KUrl url(it.url());

        if (url.isLocalFile()) {
            // Code from kdelibs/kioslaves/file.cpp
            KDE_struct_stat statBuf;
            if (KDE_stat(QFile::encodeName(url.toLocalFile()).data(), &statBuf) == 0) {
                uds.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, statBuf.st_mtime);
                uds.insert(KIO::UDSEntry::UDS_ACCESS_TIME, statBuf.st_atime);
                uds.insert(KIO::UDSEntry::UDS_SIZE, statBuf.st_size);
                uds.insert(KIO::UDSEntry::UDS_USER, statBuf.st_uid);
                uds.insert(KIO::UDSEntry::UDS_GROUP, statBuf.st_gid);

                mode_t type = statBuf.st_mode & S_IFMT;
                mode_t access = statBuf.st_mode & 07777;

                uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, type);
                uds.insert(KIO::UDSEntry::UDS_ACCESS, access);
            }
            else {
                continue;
            }
        } else {
            // not a local file
            KIO::StatJob* job = KIO::stat(url, KIO::HideProgressInfo);
            // we do not want to wait for the event loop to delete the job
            QScopedPointer<KIO::StatJob> sp(job);
            job->setAutoDelete(false);
            if (KIO::NetAccess::synchronousRun(job, 0)) {
                uds = job->statResult();
            } else {
                continue;
            }
        }

        uds.insert(KIO::UDSEntry::UDS_NAME, it.text());
        uds.insert(KIO::UDSEntry::UDS_URL, url.url());

        // set the local path so that KIO can handle the rest
        if (url.isLocalFile())
            uds.insert(KIO::UDSEntry::UDS_LOCAL_PATH, url.toLocalFile());

        listEntry(uds, false);
    }

    listEntry(KIO::UDSEntry(), true);
    finished();
}


void SearchProtocol::mimetype(const KUrl&)
{
    mimeType(QLatin1String("inode/directory"));
    finished();
}


void SearchProtocol::stat(const KUrl& url)
{
    // the root folder
    if (isRootUrl(url)) {
        //
        // stat the root path
        //
        KIO::UDSEntry uds;
        uds.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Desktop Queries"));
        uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QString::fromLatin1("nepomuk"));
        uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));

        statEntry(uds);
        finished();
    }

    // kDebug() << "Stat search folder" << url;
    statEntry(statSearchFolder(url));
    finished();

    /*else {
        error(KIO::ERR_CANNOT_ENTER_DIRECTORY, url.prettyUrl());
        return;
    }*/
}

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char** argv)
    {
        // necessary to use other kio slaves
        KComponentData comp("kio_baloosearch");
        QCoreApplication app(argc, argv);

        kDebug() << "Starting baloosearch slave " << getpid();

        Baloo::SearchProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kDebug() << "baloosearch slave Done";

        return 0;
    }
}
