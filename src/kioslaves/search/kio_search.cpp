/*
    SPDX-FileCopyrightText: 2008-2010 Sebastian Trueg <trueg at kde.org>
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kio_search.h"

#include "query.h"
#include "resultiterator.h"
#include "idutils.h"

#include <QUrl>
#include <QUrlQuery>
#include <KUser>
#include <QCoreApplication>
#include <KLocalizedString>

#include <KIO/Job>

using namespace Baloo;

namespace
{

KIO::UDSEntry statSearchFolder(const QUrl& url)
{
    KIO::UDSEntry uds;
    uds.reserve(9);
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.fastInsert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    uds.fastInsert(KIO::UDSEntry::UDS_ICON_OVERLAY_NAMES, QStringLiteral("baloo"));
    uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Search Folder"));
    uds.fastInsert(KIO::UDSEntry::UDS_URL, url.url());

    QUrlQuery query(url);
    QString title = query.queryItemValue(QStringLiteral("title"), QUrl::FullyDecoded);
    if (!title.isEmpty()) {
        uds.fastInsert(KIO::UDSEntry::UDS_NAME, title);
        uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, title);
    }

    return uds;
}

}

SearchProtocol::SearchProtocol(const QByteArray& poolSocket, const QByteArray& appSocket)
    : KIO::SlaveBase("baloosearch", poolSocket, appSocket)
{
}


SearchProtocol::~SearchProtocol()
{
}

void SearchProtocol::listDir(const QUrl& url)
{
    Query q = Query::fromSearchUrl(url);

    q.setSortingOption(Query::SortNone);
    ResultIterator it = q.exec();

    while (it.next()) {
        KIO::UDSEntry uds;
        uds.reserve(10);
        const QString filePath(it.filePath());

        // Code from kdelibs/kioslaves/file.cpp
        QT_STATBUF statBuf;
        const QByteArray ba = QFile::encodeName(filePath);
        if (filePathToStat(ba, statBuf) == 0) {
            uds.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, statBuf.st_mtime);
            uds.fastInsert(KIO::UDSEntry::UDS_ACCESS_TIME, statBuf.st_atime);
            uds.fastInsert(KIO::UDSEntry::UDS_SIZE, statBuf.st_size);
#ifndef Q_OS_WIN
            uds.fastInsert(KIO::UDSEntry::UDS_USER, getUserName(KUserId(statBuf.st_uid)));
            uds.fastInsert(KIO::UDSEntry::UDS_GROUP, getGroupName(KGroupId(statBuf.st_gid)));
#else
#pragma message("TODO: st_uid and st_gid are always zero, use GetSecurityInfo to find the owner")
#endif

            mode_t type = statBuf.st_mode & S_IFMT;
            mode_t access = statBuf.st_mode & 07777;

            uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, type);
            uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, access);
        }
        else {
            continue;
        }

        QUrl url = QUrl::fromLocalFile(filePath);
        uds.fastInsert(KIO::UDSEntry::UDS_NAME, url.fileName());
        uds.fastInsert(KIO::UDSEntry::UDS_URL, url.url());
        uds.fastInsert(KIO::UDSEntry::UDS_LOCAL_PATH, filePath);

        listEntry(uds);
    }

    KIO::UDSEntry uds;
    uds.reserve(5);
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, QStringLiteral("."));
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.fastInsert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    listEntry(uds);

    finished();
}


void SearchProtocol::mimetype(const QUrl&)
{
    mimeType(QStringLiteral("inode/directory"));
    finished();
}


void SearchProtocol::stat(const QUrl& url)
{
    statEntry(statSearchFolder(url));
    finished();
}

QString SearchProtocol::getUserName(const KUserId &uid) const
{
    if (Q_UNLIKELY(!uid.isValid())) {
        return QString();
    }
    if (!mUsercache.contains(uid)) {
        KUser user(uid);
        QString name = user.loginName();
        if (name.isEmpty()) {
            name = uid.toString();
        }
        mUsercache.insert(uid, name);
        return name;
    }
    return mUsercache[uid];
}

QString SearchProtocol::getGroupName(const KGroupId &gid) const
{
    if (Q_UNLIKELY(!gid.isValid())) {
        return QString();
    }
    if (!mGroupcache.contains(gid)) {
        KUserGroup group(gid);
        QString name = group.name();
        if (name.isEmpty()) {
            name = gid.toString();
        }
        mGroupcache.insert(gid, name);
        return name;
    }
    return mGroupcache[gid];
}


extern "C"
{
    Q_DECL_EXPORT int kdemain(int argc, char** argv)
    {
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("kio_baloosearch"));
        Baloo::SearchProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        return 0;
    }
}
