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

#include <QUrl>
#include <QDebug>

#include <KLocalizedString>
#include <KUser>
#include <kio/job.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>

#include "file.h"
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
    uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Tag"));
    uds.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("tag"));

    return uds;
}
}

void TagsProtocol::listDir(const QUrl& url)
{
    qDebug() << url;

    QString tag;
    QString fileUrl;
    QStringList paths;

    ParseResult result = parseUrl(url, tag, fileUrl);

    TagListJob* job = new TagListJob();
    job->exec();

    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl: {
        qDebug() << "Root Url";

        for (QString resultTag : job->tags()) {
            if (resultTag.contains(QLatin1Char('/'))) {
                resultTag = resultTag.section(QLatin1Char('/'), 0, 0, QString::SectionSkipEmpty);
            }
            if (paths.contains(resultTag, Qt::CaseInsensitive)) {
                continue;
            } else {
                paths.insert(0, resultTag);
            }
            listEntry(createUDSEntryForTag(resultTag));
        }

        finished();
        return;
    }

    case TagUrl: {
        for (QString resultTag : job->tags()) {
            if (resultTag.startsWith(tag, Qt::CaseInsensitive) && resultTag.contains(QLatin1Char('/'))) {
                resultTag.remove(0, (tag.size() + 1));
                resultTag = resultTag.section(QLatin1Char('/'), 0, 0, QString::SectionSkipEmpty);
                if (paths.contains(resultTag, Qt::CaseInsensitive)) {
                    continue;
                } else {
                    paths.insert(0, resultTag);
                }
                listEntry(createUDSEntryForTag(resultTag));
            }
        }

        Query q;
        q.setSortingOption(Query::SortNone);
        q.setSearchString(QStringLiteral("tag=\"%1\"").arg(tag));

        ResultIterator it = q.exec();
        while (it.next()) {
            const QUrl url = QUrl::fromLocalFile(it.filePath());
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

            listEntry(uds);
        }

        finished();
        return;
    }

    case FileUrl:
        qDebug() << "File URL : " << fileUrl;
        ForwardingSlaveBase::listDir(QUrl::fromLocalFile(fileUrl));
        return;
    }
}

void TagsProtocol::stat(const QUrl& url)
{
    qDebug() << url;

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
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));

        uds.insert(KIO::UDSEntry::UDS_ICON_OVERLAY_NAMES, QStringLiteral("tag"));
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Tag"));

        uds.insert(KIO::UDSEntry::UDS_NAME, QStringLiteral("."));
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

void TagsProtocol::copy(const QUrl& src, const QUrl& dest, int, KIO::JobFlags)
{
    qDebug() << src << dest;

    if (src.scheme() != QLatin1String("file")) {
        error(KIO::ERR_UNSUPPORTED_ACTION, src.toString());
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
        error(KIO::ERR_UNSUPPORTED_ACTION, src.toString());
        return;

    case FileUrl:
        /*
         * FIXME: Do we really need to support copy operations?
        Baloo::FileFetchJob* job = new Baloo::FileFetchJob(fileUrl);
        job->exec();
        Baloo::File file = job->file();

        file.addTag(tag);
        Baloo::FileModifyJob* mjob = new Baloo::FileModifyJob(file);
        mjob->exec();
        */

        finished();
        return;
    }
}


void TagsProtocol::get(const QUrl& url)
{
    qDebug() << url;

    QString tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, url.toString());
        return;

    case FileUrl:
        ForwardingSlaveBase::get(QUrl::fromLocalFile(fileUrl));
        return;
    }
}


void TagsProtocol::put(const QUrl& url, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions);
    Q_UNUSED(flags);

    error(KIO::ERR_UNSUPPORTED_ACTION, url.toString());
    return;
}


void TagsProtocol::rename(const QUrl& src, const QUrl& dest, KIO::JobFlags flags)
{
    qDebug() << src << dest;
    if (src.isLocalFile()) {
        error(KIO::ERR_CANNOT_DELETE_ORIGINAL, src.toString());
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
        error(KIO::ERR_UNSUPPORTED_ACTION, src.toString());
        return;

    case FileUrl: {
        // Yes, this is weird, but it is required
        // It is required cause the dest url is of the form tags:/tag/file_url_with_new_filename
        // So we extract the new fileUrl from the 'src', and apply the new file to the dest
        QString destUrl = fileUrl;
        int lastIndex = destUrl.lastIndexOf(QDir::separator());
        destUrl.resize(lastIndex + 1);
        destUrl.append(dest.fileName());

        ForwardingSlaveBase::rename(QUrl(fileUrl), QUrl(destUrl), flags);
        return;
    }
    }
}

void TagsProtocol::del(const QUrl& url, bool isfile)
{
    Q_UNUSED(url)
    Q_UNUSED(isfile);
    /*

    Tag tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, url.toString());
        return;

    case TagUrl: {
        TagRemoveJob* job = new TagRemoveJob(tag);
        job->exec();

        finished();
        return;
    }

    case FileUrl: {
        qDebug() << "Removing file url : " << fileUrl;
        // FIXME: FUCK!!!!!!
        TagRelation rel(tag, );
        TagRelationRemoveJob* job = new TagRelationRemoveJob(T);
        job->exec();

        if (job->error()) {
            qWarning() << job->errorString();
            error(KIO::ERR_CANNOT_DELETE, job->errorString());
        } else {
            finished();
        }
        return;
    }
    }*/
}


void TagsProtocol::mimetype(const QUrl& url)
{
    qDebug() << url;

    QString tag;
    QString fileUrl;

    ParseResult result = parseUrl(url, tag, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        mimeType(QStringLiteral("inode/directory"));
        finished();
        return;

    case FileUrl:
        ForwardingSlaveBase::mimetype(QUrl::fromLocalFile(fileUrl));
        return;
    }
}

// The ForwardingSlaveBase functions are always called with a file:// url
// In this case we just set the newUrl = url
bool TagsProtocol::rewriteUrl(const QUrl& url, QUrl& newURL)
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


TagsProtocol::ParseResult TagsProtocol::parseUrl(const QUrl& url, QString& tag, QString& fileUrl, bool)
{
    // Isolate the path and sanitize it by removing any beginning and trailing slashes
    QString path = url.path();
    while (path.startsWith(QLatin1Char('/')))
        path.remove(0, 1);
    while (path.endsWith(QLatin1Char('/')))
        path.chop(1);

    // If the resulting string is empty, the result is the root url
    if (path.isEmpty())
        return RootUrl;

    // If the path doesn't end with a trailing slash, the result is a tag
    if (!url.path().endsWith(QLatin1Char('/'))) {
        tag = path;
        fileUrl.clear();

        return TagUrl;
    } else {
        tag = path;
        QString fileName = url.fileName();
        fileUrl = decodeFileUrl(fileName);

        return FileUrl;
    }
}


extern "C"
{
    Q_DECL_EXPORT int kdemain(int argc, char** argv)
    {
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("kio_tags"));
        Baloo::TagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        return 0;
    }
}

