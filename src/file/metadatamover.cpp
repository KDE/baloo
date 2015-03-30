/* This file is part of the KDE Project
   Copyright (c) 2009-2011 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2013-2014 Vishesh Handa <vhanda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "metadatamover.h"
#include "database.h"
#include "basicindexingjob.h"
#include "idutils.h"

#include <QDebug>
#include <QFile>

using namespace Baloo;

MetadataMover::MetadataMover(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
}


MetadataMover::~MetadataMover()
{
}


void MetadataMover::moveFileMetadata(const QString& from, const QString& to)
{
//    qDebug() << from << to;
    Q_ASSERT(!from.isEmpty() && from != QLatin1String("/"));
    Q_ASSERT(!to.isEmpty() && to != QLatin1String("/"));

    // We do NOT get deleted messages for overwritten files! Thus, we
    // have to remove all metadata for overwritten files first.
    removeMetadata(to);

    // and finally update the old statements
    updateMetadata(from, to);

    m_db->commit();
    m_db->transaction(Database::ReadWrite);
}

void MetadataMover::removeFileMetadata(const QString& file)
{
    Q_ASSERT(!file.isEmpty() && file != QLatin1String("/"));
    removeMetadata(file);

    // FIXME: We do not want to commit this so often!!
    m_db->commit();
    m_db->transaction(Database::ReadWrite);
    qDebug();
}


void MetadataMover::removeMetadata(const QString& url)
{
    Q_ASSERT(!url.isEmpty());

    int i = url.lastIndexOf('/');
    const QString dirPath = url.mid(0, i);
    const QString filename = url.mid(i + 1);

    quint64 parentId = filePathToId(QFile::encodeName(dirPath));
    quint64 id = m_db->documentId(parentId, QFile::encodeName(filename));

    if (!id) {
        return;
    }

    m_db->removeDocument(id);
}

void MetadataMover::updateMetadata(const QString& from, const QString& to)
{
    qDebug() << from << "->" << to;
    Q_ASSERT(!from.isEmpty() && !to.isEmpty());
    Q_ASSERT(from[from.size()-1] != QLatin1Char('/'));
    Q_ASSERT(to[to.size()-1] != QLatin1Char('/'));

    QByteArray toPath = QFile::encodeName(to);
    quint64 id = filePathToId(toPath);
    if (!m_db->hasDocument(id)) {
        //
        // If we have no metadata yet we need to tell the file indexer so it can
        // create the metadata in case the target folder is configured to be indexed.
        //
        qDebug() << "Moved without data";
        Q_EMIT movedWithoutData(to);
        return;
    }

    BasicIndexingJob job(toPath, QString(), true);
    job.index();
    m_db->renameFilePath(id, job.document());

    // Possible scenarios
    // 1. file moves to the same device - id is preserved
    // 2. file moves to a different device - id is not preserved

}
