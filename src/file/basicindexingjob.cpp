/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "basicindexingjob.h"
#include "database.h"

#include <QTimer>
#include <QVariant>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlQuery>

#include <KDebug>

using namespace Baloo;

BasicIndexingJob::BasicIndexingJob(Database* m_db, const FileMapping& file,
                                   const QString& mimetype, QObject* parent)
    : KJob(parent)
    , m_db(m_db)
    , m_file(file)
    , m_mimetype(mimetype)
{
}

BasicIndexingJob::~BasicIndexingJob()
{
}

void BasicIndexingJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void BasicIndexingJob::doStart()
{
    if (m_file.id() == 0) {
        if (!m_file.create(m_db->sqlDatabase())) {
            setError(1);
            setErrorText("Cannot create fileMapping for" + QString::number(m_file.id()));
            emitResult();
            return;
        }
    }

    QFileInfo fileInfo(m_file.url());

    // FIXME: We will need a termGenerator
    // FIXME: Fetch the type?

    Xapian::Document doc;
    doc.add_term('M' + m_mimetype.toStdString());
    doc.add_term('F' + fileInfo.fileName().toStdString());

    // Modified Date
    QDateTime mod = fileInfo.lastModified();
    doc.add_term("DT_M" + fileInfo.lastModified().toString(Qt::ISODate).toStdString());

    const QString year = "DT_MY" + QString::number(mod.date().year());
    const QString month = "DT_MM" + QString::number(mod.date().month());
    const QString day = "DT_MD" + QString::number(mod.date().day());
    doc.add_term(year.toStdString());
    doc.add_term(month.toStdString());
    doc.add_term(day.toStdString());

    // Indexing Level 1
    doc.add_term("Z1");

    // Types
    QList<QByteArray> types = typesForMimeType(m_mimetype);
    types << QByteArray("File");

    Q_FOREACH (const QByteArray& arr, types) {
        QByteArray a = 'T' + arr.toLower();
        doc.add_boolean_term(a.constData());
    }

    Q_EMIT newDocument(m_file.id(), doc);
    emitResult();
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
    //if (mimeType.contains(QLatin1String("text")))
    //    types << "Document";
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

