/*
   Copyright 2012 Vishesh Handa <me@vhanda.in>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kio_tags.h"

#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/DataManagement>

#include <Nepomuk2/Vocabulary/NFO>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Vocabulary/NUAO>

#include <KUrl>
#include <kio/global.h>
#include <klocale.h>
#include <kio/job.h>
#include <KUser>
#include <KDebug>
#include <KLocale>
#include <kio/netaccess.h>
#include <KComponentData>

#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Node>

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

#include <sys/types.h>
#include <unistd.h>
#include <Nepomuk2/Variant>

using namespace Nepomuk2;
using namespace Soprano::Vocabulary;

TagsProtocol::TagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket)
    : KIO::ForwardingSlaveBase("tags", pool_socket, app_socket)
{
}

TagsProtocol::~TagsProtocol()
{
}


namespace
{
//FIXME: Find a nice icon for tags?
KIO::UDSEntry createUDSEntryForTag(const Tag& tag)
{
    //
    // WARNING: This could cause some problems, the user will choose the display name, but
    // the url will show the nao:identifier.
    // Most of the times the nao:identifier and genericLabel are the same, however, there
    // are a few cases when they aren't. What to do?
    //
    const QDateTime creationDt = tag.property(NAO::created()).toDateTime();
    const QDateTime modDt = tag.property(NAO::lastModified()).toDateTime();

    KIO::UDSEntry uds;
    uds.insert(KIO::UDSEntry::UDS_NAME, tag.property(NAO::identifier()).toString());
    uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, tag.genericLabel());
    uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
    uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Tag"));
    uds.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, modDt.toTime_t());
    uds.insert(KIO::UDSEntry::UDS_CREATION_TIME, creationDt.toTime_t());
    uds.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    uds.insert(KIO::UDSEntry::UDS_NEPOMUK_URI, tag.uri().toString());
    uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QLatin1String("feed-subscribe"));

    return uds;
}
}

void TagsProtocol::listDir(const KUrl& url)
{
    kDebug() << url;

    QList<Tag> tags;
    QUrl fileUrl;

    ParseResult result = parseUrl(url, tags, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl: {
        kDebug() << "Root Url";

        QLatin1String query("select distinct ?r where { ?r a nao:Tag . ?f a nfo:FileDataObject ; nao:hasTag ?r . }");
        Soprano::Model* model = ResourceManager::instance()->mainModel();
        Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
        while (it.next()) {
            Tag tag(it[0].uri());
            listEntry(createUDSEntryForTag(tag), false);

            m_allTags << tag;
        }

        listEntry(KIO::UDSEntry(), true);
        finished();
        return;
    }

    case TagUrl: {
        // Get the N3
        kDebug() << "Tag URL: " << tags;
        QStringList tagN3;
        QSet<QUrl> tagUris;
        foreach(const Tag & tag, tags) {
            tagUris << tag.uri();
            tagN3.append(Soprano::Node::resourceToN3(tag.uri()));
        }

        // Fetch all the files
        QString query = QString::fromLatin1("select ?r ?url where { ?r a nfo:FileDataObject; "
                                            "nie:url ?url; nao:hasTag %1 .}")
                        .arg(tagN3.join(","));

        Soprano::Model* model = ResourceManager::instance()->mainModel();
        Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparqlNoInference);
        QList<QUrl> fileUris;
        while (it.next()) {
            // Fetch info about each of these files
            KUrl fileUri = it[0].uri();
            KUrl fileUrl = it[1].uri();

            fileUris << fileUri;

            QString localUrl = fileUrl.toLocalFile();

            // Somehow stat the file
            KIO::UDSEntry uds;
            if (KIO::StatJob* job = KIO::stat(fileUrl, KIO::HideProgressInfo)) {
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
            uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, fileUrl.fileName());
            //FIXME: Should we be setting the UDS_URL?
            //uds.insert( KIO::UDSEntry::UDS_URL, fileUrl.url() );
            uds.insert(KIO::UDSEntry::UDS_TARGET_URL, fileUrl.url());
            uds.insert(KIO::UDSEntry::UDS_LOCAL_PATH, localUrl);
            uds.insert(KIO::UDSEntry::UDS_NEPOMUK_URI, fileUri.url());

            listEntry(uds, false);
        }

        // Get all the tags the files are tagged with
        QSet<QUrl> allTags;
        foreach(const QUrl & fileUri, fileUris) {
            // Fetch all the tags
            QString query = QString::fromLatin1("select distinct ?t where { %1 nao:hasTag ?t . }")
                            .arg(Soprano::Node::resourceToN3(fileUri));

            Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparqlNoInference);
            while (it.next()) {
                allTags.insert(it[0].uri());
            }
        }

        // Remove already listed tags from tagUris
        QSet<QUrl> tagsToList = allTags.subtract(tagUris);

        // Emit the total number of files
        totalSize(tagsToList.size() + fileUris.size());

        // List each of the tags
        foreach(const QUrl & tagUri, tagsToList) {
            listEntry(createUDSEntryForTag(Tag(tagUri)), false);
        }

        listEntry(KIO::UDSEntry(), true);
        finished();
    }

    case FileUrl:
        kDebug() << "File URL : " << fileUrl;
        ForwardingSlaveBase::listDir(fileUrl);
        return;
    }
}

void TagsProtocol::stat(const KUrl& url)
{
    kDebug() << url;

    QList<Tag> tags;
    QUrl fileUrl;

    ParseResult result = parseUrl(url, tags, fileUrl);
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
        statEntry(createUDSEntryForTag(tags.last()));
        finished();
        return;
    }

    case FileUrl:
        ForwardingSlaveBase::get(fileUrl);
        return;
    }
}

void TagsProtocol::mkdir(const KUrl& url, int permissions)
{
    kDebug() << url;

    Q_UNUSED(permissions);

    QList<Tag> tags;
    QUrl fileUrl;

    ParseResult result = parseUrl(url, tags, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
        return;

    case TagUrl: {
        QString errorString = QString::fromLatin1("Tag %1 already exists").arg(url.fileName());
        error(KIO::ERR_COULD_NOT_MKDIR, errorString);
        return;
    }

    // WE think it is a fileUrl since the last tag doesn't exist
    case FileUrl: {
        const QString label = url.fileName();
        Tag tag(label);
        tag.setLabel(label);

        finished();
        return;
    }
    }
}

void TagsProtocol::copy(const KUrl& src, const KUrl& dest, int permissions, KIO::JobFlags flags)
{
    kDebug() << src << dest;

    if (src.scheme() == QLatin1String("file")) {
        QList<Tag> tags;
        QUrl fileUrl;

        ParseResult result = parseUrl(dest, tags, fileUrl);
        switch (result) {
        case InvalidUrl:
            return;

        case RootUrl:
        case TagUrl:
            error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
            return;

            // It's a file url, cause the filename doesn't exist as a tag
        case FileUrl:
            QVariantList tagUris;
            foreach(const Tag & tag, tags)
            tagUris << tag.uri();

            KJob* job = Nepomuk2::addProperty(QList<QUrl>() << src, NAO::hasTag(), tagUris);
            job->exec();
            finished();
            return;
        }
    }

    QList<Tag> tags;
    QUrl fileUrl;

    ParseResult result = parseUrl(dest, tags, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
        return;

    case FileUrl:
        ForwardingSlaveBase::copy(src, fileUrl, permissions, flags);
        return;
    }
}


void TagsProtocol::get(const KUrl& url)
{
    kDebug() << url;

    QList<Tag> tags;
    QUrl fileUrl;

    ParseResult result = parseUrl(url, tags, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
        return;

    case FileUrl:
        ForwardingSlaveBase::get(fileUrl);
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

    QList<Tag> srcTags;
    QUrl fileUrl;

    ParseResult srcResult = parseUrl(src, srcTags, fileUrl);
    switch (srcResult) {
    case InvalidUrl:
        return;

    case RootUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
        return;

    case FileUrl: {
        // Yes, this is weird, but it is required
        // It is required cause the dest url is of the form tags:/tag1/tag2/file_url_with_new_filename
        // So we extract the new fileUrl from the 'src', and apply the new file to the dest
        KUrl destUrl(fileUrl);
        destUrl.setFileName(dest.fileName());

        ForwardingSlaveBase::rename(fileUrl, destUrl, flags);
        return;
    }

    case TagUrl: {
        Tag fromTag = srcTags.last();

        QString path = dest.path();
        QStringList tagNames = path.split('/');
        if (tagNames.isEmpty()) {
            error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
            return;
        }

        QString toIdentifier = tagNames.last();
        fromTag.setProperty(NAO::identifier(), toIdentifier);
        fromTag.setProperty(NAO::prefLabel(), toIdentifier);

        finished();
    }
    }
}

void TagsProtocol::del(const KUrl& url, bool isfile)
{
    Q_UNUSED(isfile);

    QList<Tag> tags;
    QUrl fileUrl;

    ParseResult result = parseUrl(url, tags, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
        error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
        return;

    case TagUrl: {
        tags.last().remove();

        finished();
        return;
    }

    case FileUrl: {
        kDebug() << "Removing file url : " << fileUrl;
        const QUrl tagUri = tags.last().uri();
        KJob* job = Nepomuk2::removeProperty(QList<QUrl>() << fileUrl, NAO::hasTag(), QVariantList() << tagUri);
        job->exec();
        if (job->error()) {
            kError() << job->errorString();
            error(KIO::ERR_CANNOT_DELETE, job->errorString());
        } else {
            finished();
        }
        return;
    }
    }
}


void Nepomuk2::TagsProtocol::mimetype(const KUrl& url)
{
    kDebug() << url;

    QList<Tag> tags;
    QUrl fileUrl;

    ParseResult result = parseUrl(url, tags, fileUrl);
    switch (result) {
    case InvalidUrl:
        return;

    case RootUrl:
    case TagUrl:
        mimeType("inode/directory");
        finished();
        return;

    case FileUrl:
        ForwardingSlaveBase::mimetype(fileUrl);
        return;
    }
}

QUrl Nepomuk2::TagsProtocol::decodeFileUrl(const QString& urlString)
{
    return QUrl::fromEncoded(QByteArray::fromPercentEncoding(urlString.toAscii(), '_'));
}

QString Nepomuk2::TagsProtocol::encodeFileUrl(const QUrl& url)
{
    return QString::fromAscii(url.toEncoded().toPercentEncoding(QByteArray(), QByteArray(""), '_'));
}


Nepomuk2::TagsProtocol::ParseResult Nepomuk2::TagsProtocol::parseUrl(const KUrl& url, QList< Tag >& tags, QUrl& fileUrl, bool ignoreErrors)
{
    QString path = url.path();
    if (path.isEmpty() || path == QLatin1String("/"))
        return RootUrl;

    QString fileName = url.fileName(KUrl::ObeyTrailingSlash);
    QString dir = url.directory(KUrl::ObeyTrailingSlash);

    QStringList tagNames = dir.split('/', QString::SkipEmptyParts);
    if (!fileName.isEmpty()) {
        Soprano::Model* model = ResourceManager::instance()->mainModel();
        QString query = QString::fromLatin1("ask where { ?r a nao:Tag ; nao:identifier %1 . }")
                        .arg(Soprano::Node::literalToN3(fileName));

        if (model->executeQuery(query, Soprano::Query::QueryLanguageSparql).boolValue()) {
            tagNames << fileName;
        } else {
            fileUrl = decodeFileUrl(fileName);
        }
    }

    tags.clear();
    foreach(const QString & tagName, tagNames) {
        QUrl uri = Tag(tagName).uri();
        if (uri.isEmpty() && !ignoreErrors) {
            QString text = QString::fromLatin1("Tag %1 does not exist").arg(tagName);
            error(KIO::ERR_CANNOT_ENTER_DIRECTORY, text);
            return InvalidUrl;
        } else if (!uri.isEmpty()) {
            tags << uri;
        }
    }

    if (!fileUrl.isEmpty())
        return FileUrl;

    return TagUrl;
}

bool Nepomuk2::TagsProtocol::rewriteUrl(const KUrl& url, KUrl& newURL)
{
    if (url.scheme() != QLatin1String("file"))
        return false;

    newURL = url;
    return true;
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

        Nepomuk2::TagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }
}

