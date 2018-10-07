/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012-2014  Vishesh Handa <me@vhanda.in>
 * Copyright (C) 2017-2018  James D. Smith <smithjd15@gmail.com>
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
#include <QLoggingCategory>

#include <KLocalizedString>
#include <KUser>
#include <KFileMetaData/UserMetaData>
#include <KIO/Job>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>

#include "file.h"
#include "taglistjob.h"

#include "query.h"
#include "term.h"

Q_LOGGING_CATEGORY(KIO_TAGS, "kf5.kio.kio_tags")

using namespace Baloo;

TagsProtocol::TagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket)
    : KIO::ForwardingSlaveBase("tags", pool_socket, app_socket)
{
}

TagsProtocol::~TagsProtocol()
{
}

void TagsProtocol::listDir(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
        case FileUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "list() invalid url";
            error(KIO::ERR_CANNOT_ENTER_DIRECTORY, result.decodedUrl);
            return;
        case TagUrl:
            listEntries(result.pathUDSResults);
    }

    finished();
}

void TagsProtocol::stat(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "stat() invalid url";
            error(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
            return;
        case FileUrl:
            ForwardingSlaveBase::stat(result.fileUrl);
            return;
        case TagUrl:
            for (const KIO::UDSEntry& entry : qAsConst(result.pathUDSResults)) {
                if (entry.stringValue(KIO::UDSEntry::UDS_EXTRA) == result.tag) {
                    statEntry(entry);
                }
            }
    }

    finished();
}

void TagsProtocol::copy(const QUrl& src, const QUrl& dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions);
    Q_UNUSED(flags);

    ParseResult srcResult = parseUrl(src);
    ParseResult dstResult = parseUrl(dest, QList<ParseFlags>() << ChopLastSection << LazyValidation);

    if (srcResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << srcResult.decodedUrl << "copy() invalid src url";
        error(KIO::ERR_DOES_NOT_EXIST, srcResult.decodedUrl);
        return;
    } else if (dstResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << dstResult.decodedUrl << "copy() invalid dest url";
        error(KIO::ERR_DOES_NOT_EXIST, dstResult.decodedUrl);
        return;
    }

    auto rewriteTags = [] (KFileMetaData::UserMetaData& md, const QString& newTag) {
        qCDebug(KIO_TAGS) << md.filePath() << "adding tag" << newTag;
        QStringList tags = md.tags();
        tags.append(newTag);
        md.setTags(tags);
    };

    if (srcResult.metaData.tags().contains(dstResult.tag)) {
        qCWarning(KIO_TAGS) << srcResult.fileUrl.toLocalFile() << "file already has tag" << dstResult.tag;
        infoMessage(i18n("File %1 already has tag %2", srcResult.fileUrl.toLocalFile(), dstResult.tag));
    } else if (dstResult.urlType == TagUrl) {
        rewriteTags(srcResult.metaData, dstResult.tag);
    }

    finished();
}

void TagsProtocol::get(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "get() invalid url";
            error(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
            return;
        case FileUrl:
            ForwardingSlaveBase::get(result.fileUrl);
            return;
        case TagUrl:
            error(KIO::ERR_UNSUPPORTED_ACTION, result.decodedUrl);
            return;
    }
}

void TagsProtocol::rename(const QUrl& src, const QUrl& dest, KIO::JobFlags flags)
{
    Q_UNUSED(flags);

    ParseResult srcResult = parseUrl(src);
    ParseResult dstResult;

    if (srcResult.urlType == FileUrl) {
        dstResult = parseUrl(dest, QList<ParseFlags>() << ChopLastSection);
    } else if (srcResult.urlType == TagUrl) {
        dstResult = parseUrl(dest, QList<ParseFlags>() << LazyValidation);
    }

    if (srcResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << srcResult.decodedUrl << "rename() invalid src url";
        error(KIO::ERR_DOES_NOT_EXIST, srcResult.decodedUrl);
        return;
    } else if (dstResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << dstResult.decodedUrl << "rename() invalid dest url";
        error(KIO::ERR_DOES_NOT_EXIST, dstResult.decodedUrl);
        return;
    }

    auto rewriteTags = [] (KFileMetaData::UserMetaData& md, const QString& oldTag, const QString& newTag) {
        qCDebug(KIO_TAGS) << md.filePath() << "swapping tag" << oldTag << "with" << newTag;
        QStringList tags = md.tags();
        tags.removeAll(oldTag);
        tags.append(newTag);
        md.setTags(tags);
    };

    if (srcResult.metaData.tags().contains(dstResult.tag)) {
        qCWarning(KIO_TAGS) << srcResult.fileUrl.toLocalFile() << "file already has tag" << dstResult.tag;
        infoMessage(i18n("File %1 already has tag %2", srcResult.fileUrl.toLocalFile(), dstResult.tag));
    } else if (srcResult.urlType == FileUrl) {
        rewriteTags(srcResult.metaData, srcResult.tag, dstResult.tag);
    } else if (srcResult.urlType == TagUrl) {
        ResultIterator it = srcResult.query.exec();
        while (it.next()) {
            KFileMetaData::UserMetaData md(it.filePath());
            if (it.filePath() == srcResult.fileUrl.toLocalFile()) {
                rewriteTags(md, srcResult.tag, dstResult.tag);
            } else if (srcResult.fileUrl.isEmpty()) {
                const auto tags = md.tags();
                for (const QString& tag : tags) {
                    if (tag == srcResult.tag || (tag.startsWith(srcResult.tag + QLatin1Char('/')))) {
                        QString newTag = tag;
                        newTag.replace(srcResult.tag, dstResult.tag, Qt::CaseInsensitive);
                        rewriteTags(md, tag, newTag);
                    }
                }
            }
        }
    }

    finished();
}

void TagsProtocol::del(const QUrl& url, bool isfile)
{
    Q_UNUSED(isfile);

    ParseResult result = parseUrl(url);

    auto rewriteTags = [] (KFileMetaData::UserMetaData& md, const QString& tag) {
        qCDebug(KIO_TAGS) << md.filePath() << "removing tag" << tag;
        QStringList tags = md.tags();
        tags.removeAll(tag);
        md.setTags(tags);
    };

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "del() invalid url";
            error(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
            return;
        case FileUrl:
        case TagUrl:
            ResultIterator it = result.query.exec();
            while (it.next()) {
                KFileMetaData::UserMetaData md(it.filePath());
                if (it.filePath() == result.fileUrl.toLocalFile()) {
                    rewriteTags(md, result.tag);
                } else if (result.fileUrl.isEmpty()) {
                    const auto tags = md.tags();
                    for (const QString &tag : tags) {
                        if ((tag == result.tag) || (tag.startsWith(result.tag + QLatin1Char('/'), Qt::CaseInsensitive))) {
                            rewriteTags(md, tag);
                        }
                    }
                }
            }
    }

    finished();
}

void TagsProtocol::mimetype(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "mimetype() invalid url";
            error(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
            return;
        case FileUrl:
            ForwardingSlaveBase::mimetype(result.fileUrl);
            return;
        case TagUrl:
            mimeType(QStringLiteral("inode/directory"));
    }

    finished();
}

void TagsProtocol::mkdir(const QUrl& url, int permissions)
{
    Q_UNUSED(permissions);

    ParseResult result = parseUrl(url, QList<ParseFlags>() << LazyValidation);

    switch(result.urlType) {
        case InvalidUrl:
        case FileUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "mkdir() invalid url";
            error(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
            return;
        case TagUrl:
            m_unassignedTags << result.tag;
    }

    finished();
}

bool TagsProtocol::rewriteUrl(const QUrl& url, QUrl& newURL)
{
    Q_UNUSED(url);
    Q_UNUSED(newURL);

    return false;
}

TagsProtocol::ParseResult TagsProtocol::parseUrl(const QUrl& url, const QList<ParseFlags> &flags)
{
    TagsProtocol::ParseResult result;
    result.decodedUrl = QUrl::fromPercentEncoding(url.toString().toUtf8());

    auto createUDSEntryForTag = [] (const QString& tagSection, const QString& tag) {
        KIO::UDSEntry uds;
        uds.reserve(9);
        uds.fastInsert(KIO::UDSEntry::UDS_NAME, tagSection);
        uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
        uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
        uds.fastInsert(KIO::UDSEntry::UDS_USER, KUser().loginName());
        uds.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("tag"));
        uds.fastInsert(KIO::UDSEntry::UDS_EXTRA, tag);

        QString displayType;
        if (tagSection == tag) {
            displayType = i18n("Tag");
        } else if (!tag.isEmpty()) {
            displayType = i18n("Tag Fragment");
        } else {
            displayType = i18n("All Tags");
        }

        uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_TYPE, displayType);

        QString displayName = i18n("All Tags");
        if (!tag.isEmpty() && ((tagSection == QStringLiteral(".")) || (tagSection == QStringLiteral("..")))) {
            displayName = tag.section(QLatin1Char('/'), -1);
        } else {
            displayName = tagSection;
        }

        uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayName);

        return uds;
    };

    TagListJob* tagJob = new TagListJob();
    if (!tagJob->exec()) {
        qCWarning(KIO_TAGS) << "tag fetch failed:" << tagJob->errorString();
        return result;
    }

    if (url.isLocalFile()) {
        result.urlType = FileUrl;
        result.fileUrl = url;
        result.metaData = KFileMetaData::UserMetaData(url.toLocalFile());
    } else if (url.scheme() == QLatin1String("tags")) {
        bool validTag = flags.contains(LazyValidation);

        // Determine the tag from the URL.
        result.tag = result.decodedUrl;
        result.tag.remove(url.scheme() + QLatin1Char(':'));
        result.tag = QDir::cleanPath(result.tag);
        while (result.tag.startsWith(QLatin1Char('/'))) {
            result.tag.remove(0, 1);
        }

        // Extract any local file path from the URL.
        QString tag = result.tag.section(QDir::separator(), 0, -2);
        QString fileName = result.tag.section(QDir::separator(), -1, -1);
        int pos = 0;

        // Extract and remove any multiple filename suffix from the file name.
        QRegularExpression regexp(QStringLiteral("\\s\\((\\d+)\\)$"));
        QRegularExpressionMatch regMatch = regexp.match(fileName);
        if (regMatch.hasMatch()) {
            pos = regMatch.captured(1).toInt();

            fileName.remove(regexp);
        }

        Query q;
        q.setSearchString(QStringLiteral("tag=\"%1\" AND filename=\"%2\"").arg(tag, fileName));
        ResultIterator it = q.exec();

        int i = 0;
        while (it.next()) {
            result.fileUrl = QUrl::fromLocalFile(it.filePath());
            result.metaData = KFileMetaData::UserMetaData(it.filePath());

            if (i == pos) {
                break;
            } else {
                i++;
            }
        }

        if (!result.fileUrl.isEmpty() || flags.contains(ChopLastSection)) {
            result.tag = result.tag.section(QDir::separator(), 0, -2);
        }

        validTag = validTag || result.tag.isEmpty();

        if (!result.tag.isEmpty()) {
            // Create a query to find files that may be in the operation's scope.
            QString query = result.tag;
            query.prepend("tag:");
            query.replace(' ', " AND tag:");
            query.replace('/', " AND tag:");
            result.query.setSearchString(query);

            qCDebug(KIO_TAGS) << result.decodedUrl << "url query:" << query;
        }

        // Create the tag directory entries.
        int index = result.tag.count(QLatin1Char('/')) + (result.tag.isEmpty() ? 0 : 1);
        QStringList tagPaths;

        const QStringList tags = QStringList() << tagJob->tags() << m_unassignedTags;
        for (const QString& tag : tags) {
            if (result.tag.isEmpty() || (tag.startsWith(result.tag, Qt::CaseInsensitive))) {
                QString tagSection = tag.section(QLatin1Char('/'), index, index, QString::SectionSkipEmpty);
                if (!tagPaths.contains(tagSection, Qt::CaseInsensitive) && !tagSection.isEmpty()) {
                    result.pathUDSResults << createUDSEntryForTag(tagSection, tag);
                    tagPaths << tagSection;
                }
            }

            validTag = validTag || tag.startsWith(result.tag, Qt::CaseInsensitive);
        }

        if (validTag && result.fileUrl.isEmpty()) {
            result.urlType = TagUrl;
        } else if (validTag && !result.fileUrl.isEmpty()) {
            result.urlType = FileUrl;
        }
    }

    if (result.urlType == FileUrl) {
        return result;
    } else {
        result.pathUDSResults << createUDSEntryForTag(QStringLiteral("."), result.tag);
    }

    // The root tag url has no file entries.
    if (result.tag.isEmpty()) {
        return result;
    } else {
        result.pathUDSResults << createUDSEntryForTag(QStringLiteral(".."), result.tag);
    }

    // Query for any files associated with the tag.
    Query q;
    q.setSearchString(QStringLiteral("tag=\"%1\"").arg(result.tag));
    ResultIterator it = q.exec();
    QList<QString> resultNames;
    while (it.next()) {
        const QUrl& match = QUrl::fromLocalFile(it.filePath());

        // Somehow stat the file.
        KIO::UDSEntry uds;
        KIO::StatJob* job = KIO::stat(match, KIO::HideProgressInfo);
        // We do not want to wait for the event loop to delete the job.
        QScopedPointer<KIO::StatJob> sp(job);
        job->setAutoDelete(false);
        if (job->exec()) {
            uds = job->statResult();
        } else {
            continue;
        }

        uds.fastInsert(KIO::UDSEntry::UDS_TARGET_URL, match.toString());
        uds.fastInsert(KIO::UDSEntry::UDS_ICON_OVERLAY_NAMES, QStringLiteral("tag"));

        if (resultNames.contains(match.fileName())) {
            uds.replace(KIO::UDSEntry::UDS_NAME, match.fileName() + QStringLiteral(" (%1)").arg(resultNames.count(match.fileName())));
        }

        qCDebug(KIO_TAGS) << result.tag << "adding file:" << uds.stringValue(KIO::UDSEntry::UDS_NAME);

        resultNames << match.fileName();
        result.pathUDSResults << uds;
    }

    return result;
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

