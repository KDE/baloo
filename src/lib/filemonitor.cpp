/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "filemonitor.h"

#include <QSet>
#include <QString>
#include <QStringList>
#include <QDBusConnection>

using namespace Baloo;

class FileMonitor::Private {
public:
    QSet<QString> m_files;
};

FileMonitor::FileMonitor(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    QDBusConnection con = QDBusConnection::sessionBus();
    con.connect(QString(), QStringLiteral("/files"), QStringLiteral("org.kde"),
                QStringLiteral("changed"), this, SLOT(slotFileMetaDataChanged(QStringList)));
}

FileMonitor::~FileMonitor()
{
    delete d;
}

void FileMonitor::addFile(const QString& fileUrl)
{
    QString f = fileUrl;
    if (f.endsWith(QLatin1Char('/')))
        f = f.mid(0, f.length()-1);

    d->m_files.insert(f);
}

void FileMonitor::addFile(const QUrl& url)
{
    const QString localFile = url.toLocalFile();
    if (localFile.size())
        addFile(localFile);
}

void FileMonitor::setFiles(const QStringList& fileList)
{
    d->m_files = fileList.toSet();
}

QStringList FileMonitor::files() const
{
    return QStringList(d->m_files.toList());
}

void FileMonitor::clear()
{
    d->m_files.clear();
}

void FileMonitor::slotFileMetaDataChanged(const QStringList& fileUrls)
{
    Q_FOREACH (const QString& url, fileUrls) {
        if (d->m_files.contains(url)) {
            Q_EMIT fileMetaDataChanged(url);
        }
    }
}
