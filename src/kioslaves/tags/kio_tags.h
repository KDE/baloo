/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#ifndef BALOO_KIO_TAGS_H_
#define BALOO_KIO_TAGS_H_

#include <KFileMetaData/UserMetaData>
#include <KIO/ForwardingSlaveBase>

#include "query.h"

namespace Baloo
{

class TagsProtocol : public KIO::ForwardingSlaveBase
{
    Q_OBJECT
public:
    TagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket);
    ~TagsProtocol() Q_DECL_OVERRIDE;

    enum UrlType {
        InvalidUrl,
        FileUrl,
        TagUrl
    };
    Q_ENUM(UrlType)

    /**
     * List all files and folders tagged with the corresponding tag, along with
     * additional tags that can be used to filter the results
     */
    void listDir(const QUrl& url) Q_DECL_OVERRIDE;

    /**
     * Will be forwarded for files.
     */
    void get(const QUrl& url) Q_DECL_OVERRIDE;

    /**
     * Files and folders can be copied to the virtual folders resulting
     * is assignment of the corresponding tag.
     */
    void copy(const QUrl& src, const QUrl& dest, int permissions, KIO::JobFlags flags) Q_DECL_OVERRIDE;

    /**
     * File renaming will be forwarded.
     * Folder renaming results in renaming of the tag.
     */
    void rename(const QUrl& src, const QUrl& dest, KIO::JobFlags flags) Q_DECL_OVERRIDE;

    /**
     * File deletion means remocing the tag
     * Folder deletion will result in deletion of the tag.
     */
    void del(const QUrl& url, bool isfile) Q_DECL_OVERRIDE;

    /**
     * Files will be forwarded.
     * Tags will be created as virtual folders.
     */
    void mimetype(const QUrl& url) Q_DECL_OVERRIDE;

    /**
     * Virtual folders will be created.
     */
    void mkdir(const QUrl& url, int permissions) Q_DECL_OVERRIDE;

    /**
     * Files will be forwarded.
     * Tags will be created as virtual folders.
     */
    void stat(const QUrl& url) Q_DECL_OVERRIDE;
protected:
    bool rewriteUrl(const QUrl& url, QUrl& newURL) Q_DECL_OVERRIDE;

private:
    enum ParseFlags {
        ChopLastSection,
        LazyValidation
    };

    struct ParseResult {
        UrlType urlType = InvalidUrl;
        QString decodedUrl;
        QString tag;
        QUrl fileUrl;
        KFileMetaData::UserMetaData metaData = KFileMetaData::UserMetaData(QString());
        Query query;
        KIO::UDSEntryList pathUDSResults;
    };

    ParseResult parseUrl(const QUrl& url, const QList<ParseFlags> &flags = QList<ParseFlags>());
    QStringList m_unassignedTags;
};
}

#endif // BALOO_KIO_TAGS_H_
