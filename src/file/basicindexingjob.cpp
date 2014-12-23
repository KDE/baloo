/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2014 Vishesh Handa <me@vhanda.in>
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

#include "basicindexingjob.h"
#include "database.h"
#include "xapiandocument.h"

#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

#include <KFileMetaData/TypeInfo>
#include <KFileMetaData/UserMetaData>
#include <QDebug>

using namespace Baloo;

BasicIndexingJob::BasicIndexingJob(const FileMapping& file, const QString& mimetype,
                                   bool onlyBasicIndexing)
    : m_file(file)
    , m_mimetype(mimetype)
    , m_onlyBasicIndexing(onlyBasicIndexing)
    , m_id(0)
{
}

BasicIndexingJob::~BasicIndexingJob()
{
}

bool BasicIndexingJob::index()
{
    QFileInfo fileInfo(m_file.url());

    XapianDocument doc;
    doc.addBoolTerm(m_mimetype, QLatin1String("M"));
    doc.indexText(fileInfo.fileName(), 1000);
    doc.indexText(fileInfo.fileName(), QLatin1String("F"), 1000);

    // Modified Date
    QDateTime mod = fileInfo.lastModified();
    const QString dtm = mod.toString(Qt::ISODate);

    doc.addBoolTerm(dtm, QLatin1String("DT_M"));
    doc.addBoolTerm(mod.date().year(), QLatin1String("DT_MY"));
    doc.addBoolTerm(mod.date().month(), QLatin1String("DT_MM"));
    doc.addBoolTerm(mod.date().day(), QLatin1String("DT_MD"));

    const QByteArray timeTStr = QByteArray::number(mod.toTime_t());
    doc.addValue(0, timeTStr);
    doc.addValue(1, QByteArray::number(mod.date().toJulianDay()));
    doc.addValue(2, QByteArray::number(fileInfo.created().toMSecsSinceEpoch()));

    // Store the URL
    QByteArray urlArr = m_file.url().toUtf8();
    doc.addValue(3, urlArr);

    if (urlArr.size() > 240) {
        QByteArray p1 = urlArr.mid(0, 240);
        QByteArray p2 = urlArr.mid(240);

        doc.addBoolTerm(p1, "P1");
        doc.addBoolTerm(p2, "P2");
    }
    else {
        doc.addBoolTerm(urlArr, "P-");
    }

    // Types
    QVector<KFileMetaData::Type::Type> tList = typesForMimeType(m_mimetype);
    Q_FOREACH (KFileMetaData::Type::Type type, tList) {
        QString tstr = KFileMetaData::TypeInfo(type).name().toLower();
        doc.addBoolTerm(tstr, QLatin1String("T"));
    }

    if (fileInfo.isDir()) {
        doc.addBoolTerm(QStringLiteral("Tfolder"));

        // This is an optimization for folders. They do not need to go through
        // file indexing, so there are no indexers for folders
        doc.addBoolTerm(QLatin1String("Z2"));
    }
    else if (m_onlyBasicIndexing) {
        // This is to prevent indexing if option in config is set to do so
        doc.addBoolTerm(QLatin1String("Z2"));
    }
    else {
        doc.addBoolTerm(QLatin1String("Z1"));
    }

    indexXAttr(m_file.url(), doc);

    m_id = m_file.id();
    m_doc = doc.doc();
    return true;
}

bool BasicIndexingJob::indexXAttr(const QString& url, XapianDocument& doc)
{
    bool modified = false;

    KFileMetaData::UserMetaData userMetaData(url);

    QStringList tags = userMetaData.tags();
    if (!tags.isEmpty()) {
        Q_FOREACH (const QString& tag, tags) {
            doc.indexText(tag, QStringLiteral("TA"));
            doc.addBoolTerm(QStringLiteral("TAG-") + tag);
        }

        modified = true;
    }

    int rating = userMetaData.rating();
    if (rating) {
        doc.addBoolTerm(QString::number(rating), QStringLiteral("R"));
        modified = true;
    }

    QString comment = userMetaData.userComment();
    if (!comment.isEmpty()) {
        doc.indexText(comment, QStringLiteral("C"));
        modified = true;
    }

    return modified;
}

QVector<KFileMetaData::Type::Type> BasicIndexingJob::typesForMimeType(const QString& mimeType)
{
    using namespace KFileMetaData;
    QVector<Type::Type> types;

    // Basic types
    if (mimeType.contains(QLatin1String("audio")))
        types << Type::Audio;
    if (mimeType.contains(QLatin1String("video")))
        types << Type::Video;
    if (mimeType.contains(QLatin1String("image")))
        types << Type::Image;
    if (mimeType.contains(QLatin1String("document")))
        types << Type::Document;
    if (mimeType.contains(QLatin1String("text")))
        types << Type::Text;
    //if (mimeType.contains(QLatin1String("font")))
    //    types << "Font";

    if (mimeType.contains(QLatin1String("powerpoint"))) {
        types << Type::Presentation;
        types << Type::Document;
    }
    if (mimeType.contains(QLatin1String("excel"))) {
        types << Type::Spreadsheet;
        types << Type::Document;
    }
    //if (mimeType.contains(QLatin1String("text/html")))
    //    types << "Html";

    static QMultiHash<QString, Type::Type> typeMapper;
    if (typeMapper.isEmpty()) {
        // Microsoft
        typeMapper.insert(QLatin1String("text/plain"), Type::Document);
        typeMapper.insert(QLatin1String("application/msword"), Type::Document);
        typeMapper.insert(QLatin1String("application/x-scribus"), Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.ms-powerpoint"), Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.ms-powerpoint"), Type::Presentation);
        typeMapper.insert(QLatin1String("application/vnd.ms-excel"), Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.ms-excel"), Type::Spreadsheet);

        // Office 2007
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
                          Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.presentationml.presentation"),
                          Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.presentationml.presentation"),
                          Type::Presentation);
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
                          Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
                          Type::Spreadsheet);

        // Open document formats - http://en.wikipedia.org/wiki/OpenDocument_technical_specification
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.text"), Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.presentation"), Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.presentation"), Type::Presentation);
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.spreadsheet"), Type::Document);
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.spreadsheet"), Type::Spreadsheet);

        // Others
        typeMapper.insert(QLatin1String("application/pdf"), Type::Document);
        typeMapper.insert(QLatin1String("application/postscript"), Type::Document);
        typeMapper.insert(QLatin1String("application/x-dvi"), Type::Document);
        typeMapper.insert(QLatin1String("application/rtf"), Type::Document);

        // Ebooks
        typeMapper.insert(QLatin1String("application/epub+zip"), Type::Document);
        typeMapper.insert(QLatin1String("application/x-mobipocket-ebook"), Type::Document);

        // Archives - http://en.wikipedia.org/wiki/List_of_archive_formats
        typeMapper.insert(QLatin1String("application/x-tar"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-bzip2"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-gzip"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-lzip"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-lzma"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-lzop"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-compress"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-7z-compressed"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-ace-compressed"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-astrotite-afa"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-alz-compressed"), Type::Archive);
        typeMapper.insert(QLatin1String("application/vnd.android.package-archive"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-arj"), Type::Archive);
        typeMapper.insert(QLatin1String("application/vnd.ms-cab-compressed"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-cfs-compressed"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-dar"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-lzh"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-lzx"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-rar-compressed"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-stuffit"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-stuffitx"), Type::Archive);
        typeMapper.insert(QLatin1String("application/x-gtar"), Type::Archive);
        typeMapper.insert(QLatin1String("application/zip"), Type::Archive);

        // Special images
        /*
        typeMapper.insert(QLatin1String("image/vnd.microsoft.icon"), "Icon");
        typeMapper.insert(QLatin1String("image/svg+xml"), "Image");

        // Fonts
        typeMapper.insert(QLatin1String("application/vnd.ms-fontobject"), "Font");
        typeMapper.insert(QLatin1String("application/vnd.ms-opentype"), "Font");
        */
    }

    types << typeMapper.values(mimeType).toVector();
    return types;
}

