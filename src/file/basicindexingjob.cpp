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

#include "basicindexingjob.h"
#include "database.h"

#include <QTimer>
#include <QFileInfo>
#include <QDateTime>

#include <KDebug>

using namespace Baloo;

BasicIndexingJob::BasicIndexingJob(QSqlDatabase* db, const FileMapping& file, const QString& mimetype)
    : m_sqlDb(db)
    , m_file(file)
    , m_mimetype(mimetype)
{
}

BasicIndexingJob::~BasicIndexingJob()
{
}

bool BasicIndexingJob::index()
{
    if (m_file.id() == 0) {
        if (!m_file.create(*m_sqlDb)) {
            kError() << "Cannot create fileMapping for" << m_file.url();
            return false;
        }
    }

    QFileInfo fileInfo(m_file.url());

    Xapian::Document doc;
    doc.add_term('M' + m_mimetype.toStdString());

    std::string fileName = fileInfo.fileName().toStdString();

    Xapian::TermGenerator termGen;
    termGen.set_document(doc);
    termGen.index_text(fileName, 1000);
    termGen.index_text(fileName, 1000, "F");

    // Modified Date
    QDateTime mod = fileInfo.lastModified();
    doc.add_boolean_term("DT_M" + fileInfo.lastModified().toString(Qt::ISODate).toStdString());

    const QString year = "DT_MY" + QString::number(mod.date().year());
    const QString month = "DT_MM" + QString::number(mod.date().month());
    const QString day = "DT_MD" + QString::number(mod.date().day());
    doc.add_boolean_term(year.toStdString());
    doc.add_boolean_term(month.toStdString());
    doc.add_boolean_term(day.toStdString());

    // Types
    QList<QByteArray> types = typesForMimeType(m_mimetype);
    types << QByteArray("File");
    if (fileInfo.isDir()) {
        types << QByteArray("Folder");

        // This is an optimization for folders. They do not need to go through
        // file indexing, so there are no indexers for folders
        doc.add_boolean_term("Z2");
    }
    else {
        doc.add_boolean_term("Z1");
    }

    Q_FOREACH (const QByteArray& arr, types) {
        QByteArray a = 'T' + arr.toLower();
        doc.add_boolean_term(a.constData());
    }

    m_id = m_file.id();
    m_doc = doc;
    return true;
}

QList<QByteArray> BasicIndexingJob::typesForMimeType(const QString& mimeType) const
{
    QList<QByteArray> types;

    // Basic types
    if (mimeType.contains(QLatin1String("audio")))
        types << "Audio";
    if (mimeType.contains(QLatin1String("video")))
        types << "Video";
    if (mimeType.contains(QLatin1String("image")))
        types << "Image";
    if (mimeType.contains(QLatin1String("document")))
        types << "Document";
    if (mimeType.contains(QLatin1String("text")))
        types << "Document";
    //if (mimeType.contains(QLatin1String("font")))
    //    types << "Font";

    if (mimeType.contains(QLatin1String("powerpoint"))) {
        types << "Presentation";
        types << "Document";
    }
    if (mimeType.contains(QLatin1String("excel"))) {
        types << "Spreadsheet";
        types << "Document";
    }
    //if (mimeType.contains(QLatin1String("text/html")))
    //    types << "Html";

    static QMultiHash<QString, QByteArray> typeMapper;
    if (typeMapper.isEmpty()) {
        // Microsoft
        typeMapper.insert(QLatin1String("application/msword"), "Document");
        typeMapper.insert(QLatin1String("application/vnd.ms-powerpoint"), "Document");
        typeMapper.insert(QLatin1String("application/vnd.ms-excel"), "Document");

        // Office 2007
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
                          "Document");
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.presentationml.presentation"),
                          "Document");
        typeMapper.insert(QLatin1String("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
                          "Document");

        // Open document formats - http://en.wikipedia.org/wiki/OpenDocument_technical_specification
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.text"), "Document");
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.presentation"), "Document");
        typeMapper.insert(QLatin1String("application/vnd.oasis.opendocument.spreadsheet"), "Document");

        // Others
        typeMapper.insert(QLatin1String("application/pdf"), "Document");
        typeMapper.insert(QLatin1String("application/postscript"), "Document");
        typeMapper.insert(QLatin1String("application/x-dvi"), "Document");
        typeMapper.insert(QLatin1String("application/rtf"), "Document");

        // Ebooks
        typeMapper.insert(QLatin1String("application/epub+zip"), "Document");
        typeMapper.insert(QLatin1String("application/x-mobipocket-ebook"), "Document");

        // Archives - http://en.wikipedia.org/wiki/List_of_archive_formats
        typeMapper.insert(QLatin1String("application/x-tar"), "Archive");
        typeMapper.insert(QLatin1String("application/x-bzip2"), "Archive");
        typeMapper.insert(QLatin1String("application/x-gzip"), "Archive");
        typeMapper.insert(QLatin1String("application/x-lzip"), "Archive");
        typeMapper.insert(QLatin1String("application/x-lzma"), "Archive");
        typeMapper.insert(QLatin1String("application/x-lzop"), "Archive");
        typeMapper.insert(QLatin1String("application/x-compress"), "Archive");
        typeMapper.insert(QLatin1String("application/x-7z-compressed"), "Archive");
        typeMapper.insert(QLatin1String("application/x-ace-compressed"), "Archive");
        typeMapper.insert(QLatin1String("application/x-astrotite-afa"), "Archive");
        typeMapper.insert(QLatin1String("application/x-alz-compressed"), "Archive");
        typeMapper.insert(QLatin1String("application/vnd.android.package-archive"), "Archive");
        typeMapper.insert(QLatin1String("application/x-arj"), "Archive");
        typeMapper.insert(QLatin1String("application/vnd.ms-cab-compressed"), "Archive");
        typeMapper.insert(QLatin1String("application/x-cfs-compressed"), "Archive");
        typeMapper.insert(QLatin1String("application/x-dar"), "Archive");
        typeMapper.insert(QLatin1String("application/x-lzh"), "Archive");
        typeMapper.insert(QLatin1String("application/x-lzx"), "Archive");
        typeMapper.insert(QLatin1String("application/x-rar-compressed"), "Archive");
        typeMapper.insert(QLatin1String("application/x-stuffit"), "Archive");
        typeMapper.insert(QLatin1String("application/x-stuffitx"), "Archive");
        typeMapper.insert(QLatin1String("application/x-gtar"), "Archive");
        typeMapper.insert(QLatin1String("application/zip"), "Archive");

        // Special images
        typeMapper.insert(QLatin1String("image/vnd.microsoft.icon"), "Icon");
        typeMapper.insert(QLatin1String("image/svg+xml"), "Image");

        // Fonts
        typeMapper.insert(QLatin1String("application/vnd.ms-fontobject"), "Font");
        typeMapper.insert(QLatin1String("application/vnd.ms-opentype"), "Font");
    }

    types << typeMapper.values(mimeType);
    return types;
}

