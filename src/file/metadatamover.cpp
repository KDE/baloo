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
#include "filewatch.h"
#include "filemapping.h"
#include "database.h"
#include <xapiandocument.h>

#include <QTimer>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>

#include <QDebug>

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

    m_db->xapianDatabase()->commit();
}

void MetadataMover::removeFileMetadata(const QString& file)
{
    Q_ASSERT(!file.isEmpty() && file != QLatin1String("/"));
    removeMetadata(file);

    m_db->xapianDatabase()->commit();
}


void MetadataMover::removeMetadata(const QString& url)
{
    Q_ASSERT(!url.isEmpty());

    FileMapping file(url);
    file.fetch(m_db->xapianDatabase());

    if (file.id()) {
        m_db->xapianDatabase()->deleteDocument(file.id());
        // FIXME: Is this signal still really required?
        //        We are just deleting it right now
        Q_EMIT fileRemoved(file.id());
    }
}


void MetadataMover::updateMetadata(const QString& from, const QString& to)
{
    qDebug() << from << "->" << to;
    Q_ASSERT(!from.isEmpty() && !to.isEmpty());
    Q_ASSERT(from[from.size()-1] != QLatin1Char('/'));
    Q_ASSERT(to[to.size()-1] != QLatin1Char('/'));

    FileMapping fromFile(from);
    if (fromFile.fetch(m_db->xapianDatabase())) {
        XapianDocument doc = m_db->xapianDatabase()->document(fromFile.id());

        // Change the file path
        doc.addValue(3, to);
        doc.removeTermStartsWith("P");
        doc.addBoolTerm(to, "P");

        // Change the file name
        const QStringRef fromFileName = from.midRef(from.lastIndexOf('/'));
        const QStringRef toFileName = to.midRef(to.lastIndexOf('/'));

        if (fromFileName != toFileName) {
            QStringList terms = doc.fetchTermsStartsWith("F");
            doc.removeTermStartsWith("F");

            for (const QString& term: terms) {
                doc.removeTerm(term.mid(1)); // 1 is to remove the F
            }

            QString filename = toFileName.toString();
            doc.indexText(filename, 1000);
            doc.indexText(filename, QLatin1String("F"), 1000);
        }

        m_db->xapianDatabase()->replaceDocument(fromFile.id(), doc);
    }
    else {
        //
        // If we have no metadata yet we need to tell the file indexer so it can
        // create the metadata in case the target folder is configured to be indexed.
        //
        Q_EMIT movedWithoutData(to);
        return;
    }

    //
    // Iterate over all terms starting with the path and change them
    //
    Xapian::Database* db = m_db->xapianDatabase()->db();

    auto it = db->allterms_begin(("P" + from + "/").toUtf8().constData());
    auto end = db->allterms_end(("P" + from + "/").toUtf8().constData());

    for (; it != end; it++) {
        std::string term = *it;

        Xapian::Enquire enquire(*db);
        enquire.set_query(Xapian::Query(term));
        enquire.set_weighting_scheme(Xapian::BoolWeight());

        Xapian::MSet mset = enquire.get_mset(0, 1);
        Xapian::MSetIterator miter = mset.begin();
        Q_ASSERT(miter != mset.end());

        int id = *miter;

        const QString path = QString::fromUtf8(term.c_str()).mid(1);
        const QString newPath = to + path.mid(from.length());

        XapianDocument doc = m_db->xapianDatabase()->document(id);
        doc.addValue(3, newPath);
        doc.removeTermStartsWith("P");
        doc.addBoolTerm(newPath, "P");

        m_db->xapianDatabase()->replaceDocument(id, doc);
    }
}
