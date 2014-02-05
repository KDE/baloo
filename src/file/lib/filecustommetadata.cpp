/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "filecustommetadata.h"
#include "xattrdetector.h"
#include "db.h"
#include "filemapping.h"
#include <KDebug>

#include <attr/xattr.h>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>

using namespace Baloo;

static XattrDetector g_detector;

void Baloo::setCustomFileMetaData(const QString& url, const QString& key, const QString& value)
{
    if (g_detector.isSupported(url)) {
        const QByteArray p = QFile::encodeName(url);
        const QByteArray k = key.toUtf8();
        const QByteArray v = value.toUtf8();

        int r = setxattr(p.constData(), k.constData(), v.constData(), v.size(), 0);
        if (r == -1) {
            kError() << "Could not store xattr for" << url << key << value;
            return;
        }
    }
    else {
        QSqlDatabase mapDb = fileMappingDb();
        FileMapping fileMap(url);
        if (!fileMap.fetch(mapDb)) {
            if (!fileMap.create(mapDb))
                return;
        }

        QSqlDatabase db = fileMetadataDb();
        QSqlQuery q(db);
        q.prepare("insert or replace into files (id, property, value) VALUES (?, ?, ?)");
        q.addBindValue(fileMap.id());
        q.addBindValue(key);
        q.addBindValue(value);

        if (!q.exec()) {
            kError() << url << key << value << "Error:" << q.lastError().text();
        }
    }
}

QString Baloo::customFileMetaData(const QString& url, const QString& key)
{
    if (g_detector.isSupported(url)) {
        const QByteArray p = QFile::encodeName(url);
        const QByteArray k = key.toUtf8();

        int size = getxattr(p.constData(), k.constData(), 0, 0);
        QByteArray arr(size, Qt::Uninitialized);
        getxattr(p.constData(), k.constData(), arr.data(), size);

        return QString::fromUtf8(arr);
    }
    else {
        QSqlDatabase mapDb = fileMappingDb();
        FileMapping fileMap(url);
        if (!fileMap.fetch(mapDb)) {
            return QString();
        }

        QSqlDatabase db = fileMetadataDb();
        QSqlQuery q(db);
        q.prepare("select value from files where id = ? and property = ?");
        q.addBindValue(fileMap.id());
        q.addBindValue(key);

        if (!q.exec()) {
            kError() << url << key << "Error:" << q.lastError().text();
        }

        if (q.next()) {
            return q.value(0).toString();
        }

        return QString();
    }
}
