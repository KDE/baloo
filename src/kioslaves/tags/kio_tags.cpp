/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012-2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kio_tags.h"

#include <KUrl>
#include <kio/global.h>
#include <klocale.h>
#include <kio/job.h>
#include <KUser>
#include <KDebug>
#include <KLocale>
#include <kio/netaccess.h>
#include <KComponentData>

#include <QCoreApplication>
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <sys/types.h>
#include <unistd.h>

#include "file.h"
#include "filemodifyjob.h"
#include "filefetchjob.h"
#include "taglistjob.h"

#include "query.h"
#include "term.h"

using namespace Baloo;

TagsProtocol::TagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket)
    : KIO::ForwardingSlaveBase("tags", pool_socket, app_socket)
{
}

TagsProtocol::~TagsProtocol()
{
}


namespace
{
KIO::UDSEntry createUDSEntryForTag(const QString& tag)
{
    KIO::UDSEntry uds;
    uds.insert(KIO::UDSEntry::UDS_NAME, tag);
    uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, tag);
    uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
    uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Tag"));
    uds.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QLatin1String("feed-subscribe"));

    return uds;
}
}

void TagsProtocol::listDir(const KUrl& url)
{
    kDebug() << url;

    QString tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl: {
        kDebug() << "Root Url";

        TagListJob* job = new TagListJob();
        job->exec();

        Q_FOREACH (const QString& tag, job->tags()) {
            listEntry(createUDSEntryForTag(tag), false);
        }

        listEntry(KIO::UDSEntry(), true);
        finished();
        return;
    }

    case TagUrl: {
        Query q;
        q.addType(QLatin1String("File"));
        q.setTerm(Term(QLatin1String("tag"), tag));

        ResultIterator it = q.exec();
        while (it.next()) {
            const KUrl url = it.url();
            const QString fileUrl = url.toLocalFile();

            // Somehow stat the file
            KIO::UDSEntry uds;
            if (KIO::StatJob* job = KIO::stat(url, KIO::HideProgressInfo)) {
                // we do not want to wait for the event loop to delete the job
                QScopedPointer<KIO::StatJob> sp(job);
                job->setAutoDelete(false);
                if (job->exec()) {
                    uds = job->statResult();
                } else {
                    continue;
                }
            }

            uds.insert(KIO::UDSEntry::UDS_NAME, encodeFileUrl(fileUrl));
            uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, url.fileName());
            //uds.insert(KIO::UDSEntry::UDS_URL, url);
            uds.insert(KIO::UDSEntry::UDS_TARGET_URL, url.url());
            uds.insert(KIO::UDSEntry::UDS_LOCAL_PATH, fileUrl);

            listEntry(uds, false);
        }

        listEntry(KIO::UDSEntry(), true);
        finished();
    }

    case FileUrl:
        kDebug() << "File URL : " << fileUrl;
        ForwardingSlaveBase::listDir(QUrl::fromLocalFile(fileUrl));
        return;
    }
}

void TagsProtocol::stat(const KUrl& url)
{
    kDebug() << url;

    QString tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl: {
        KIO::UDSEntry uds;
        uds.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
        uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));

        uds.insert(KIO::UDSEntry::UDS_ICON_OVERLAY_NAMES, QLatin1String("feed-subscribe"));
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Tag"));

        uds.insert(KIO::UDSEntry::UDS_NAME, QLatin1String("."));
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("All Tags"));

        statEntry(uds);
        finished();
        return;
    }

    case TagUrl: {
        statEntry(createUDSEntryForTag(tag));
        finished();
        return;
    }

    case FileUrl:
        ForwardingSlaveBase::get(QUrl::fromLocalFile(fileUrl));
        return;
    }
}

void TagsProtocol::copy(const KUrl& src, const KUrl& dest, int permissions, KIO::JobFlags flags)
{
    kDebug() << src << dest;

    if (src.scheme() != QLatin1String("file")) {
        error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
        return;
    }

    QString tag;
    QString fileUrl;

    ParseResult result = parseUrl(dest, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
        return;

    case FileUrl:
        Baloo::FileFetchJob* job = new Baloo::FileFetchJob(fileUrl);
        job->exec();
        Baloo::File file = job->file();

        file.addTag(tag);
        Baloo::FileModifyJob* mjob = new Baloo::FileModifyJob(file);
        mjob->exec();

        finished();
        return;
    }
}


void TagsProtocol::get(const KUrl& url)
{
    kDebug() << url;

    QString tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
        return;

    case FileUrl:
        ForwardingSlaveBase::get(QUrl::fromLocalFile(fileUrl));
        return;
    }
}


void TagsProtocol::put(const KUrl& url, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions);
    Q_UNUSED(flags);

    error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
    return;
}


void TagsProtocol::rename(const KUrl& src, const KUrl& dest, KIO::JobFlags flags)
{
    kDebug() << src << dest;
    if (src.isLocalFile()) {
        error(KIO::ERR_CANNOT_DELETE_ORIGINAL, src.prettyUrl());
        return;
    }

    QString srcTag;
    QString fileUrl;

    ParseResult srcResult = parseUrl(src, srcTag, fileUrl);
    switch (srcResult) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
        return;

    case FileUrl: {
        // Yes, this is weird, but it is required
        // It is required cause the dest url is of the form tags:/tag/file_url_with_new_filename
        // So we extract the new fileUrl from the 'src', and apply the new file to the dest
        KUrl destUrl(fileUrl);
        destUrl.setFileName(dest.fileName());

        ForwardingSlaveBase::rename(fileUrl, destUrl, flags);
        return;
    }
    }
}

void TagsProtocol::del(const KUrl& url, bool isfile)
{
    Q_UNUSED(isfile);
    /*

    Tag tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
        return;

    case TagUrl: {
        TagRemoveJob* job = new TagRemoveJob(tag);
        job->exec();

        finished();
        return;
    }

    case FileUrl: {
        kDebug() << "Removing file url : " << fileUrl;
        // FIXME: FUCK!!!!!!
        TagRelation rel(tag, );
        TagRelationRemoveJob* job = new TagRelationRemoveJob(T);
        job->exec();

        if (job->error()) {
            kError() << job->errorString();
            error(KIO::ERR_CANNOT_DELETE, job->errorString());
        } else {
            finished();
        }
        return;
    }
    }*/
}


void TagsProtocol::mimetype(const KUrl& url)
{
    kDebug() << url;

    QString tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        mimeType(QLatin1String("inode/directory"));
        finished();
        return;

    case FileUrl:
        ForwardingSlaveBase::mimetype(QUrl::fromLocalFile(fileUrl));
        return;
    }
}

// The ForwardingSlaveBase functions are always called with a file:// url
// In this case we just set the newUrl = url
bool TagsProtocol::rewriteUrl(const KUrl& url, KUrl& newURL)
{
    if (url.scheme() != QLatin1String("file"))
        return false;

    newURL = url;
    return true;
}

QString TagsProtocol::decodeFileUrl(const QString& urlString)
{
    return QString::fromUtf8(QByteArray::fromPercentEncoding(urlString.toUtf8(), '_'));
}

QString TagsProtocol::encodeFileUrl(const QString& url)
{
    return QString::fromUtf8(url.toUtf8().toPercentEncoding(QByteArray(), QByteArray(), '_'));
}


TagsProtocol::ParseResult TagsProtocol::parseUrl(const KUrl& url, QString& tag, QString& fileUrl, bool ignoreErrors)
{
    QString path = url.path();
    if (path.isEmpty() || path == QLatin1String("/"))
        return RootUrl;

    QStringList names = path.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if (names.size() == 0)  {
        return RootUrl;
    }

    if (names.size() == 1) {
        tag = names[0];
        fileUrl.clear();

        return TagUrl;
    }
    else {
        tag = names[0];
        QString fileName = url.fileName(KUrl::ObeyTrailingSlash);
        fileUrl = decodeFileUrl(fileName);

        return FileUrl;
    }
}


extern "C"
{
    KDE_EXPORT int kdemain(int argc, char** argv)
    {
        // necessary to use other kio slaves
        KComponentData("kio_tags");
        QCoreApplication app(argc, argv);

        if (argc != 4) {
            kError() << "Usage: kio_tags protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        Baloo::TagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }
}

