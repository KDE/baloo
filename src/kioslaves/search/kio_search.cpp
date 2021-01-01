/*
    SPDX-FileCopyrightText: 2008-2010 Sebastian Trueg <trueg at kde.org>
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Stefan Brüns <bruns@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kio_search.h"
#include "../common/udstools.h"

#include "query.h"
#include "resultiterator.h"
#include <sys/stat.h>

#include <QUrl>
#include <QUrlQuery>
#include <KUser>
#include <QCoreApplication>
#include <KLocalizedString>

using namespace Baloo;

namespace
{

KIO::UDSEntry statSearchFolder(const QUrl& url)
{
    KIO::UDSEntry uds;
    uds.reserve(9);
#ifdef Q_OS_WIN
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, _S_IREAD );
#else
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IXUSR);
#endif
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

    UdsFactory udsf;

    while (it.next()) {
        KIO::UDSEntry uds = udsf.createUdsEntry(it.filePath());
        if (uds.count()) {
	    listEntry(uds);
        }
    }

    KIO::UDSEntry uds;
    uds.reserve(5);
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, QStringLiteral("."));
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
#ifdef Q_OS_WIN
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, _S_IREAD );
#else
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IXUSR);
#endif
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
