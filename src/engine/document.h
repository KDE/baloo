/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#ifndef BALOO_DOCUMENT_H
#define BALOO_DOCUMENT_H

#include "engine_export.h"
#include <QByteArray>
#include <QDebug>
#include <QVector>

namespace Baloo {

class Database;
class TermGeneratorTest;

/**
 * A document represents a file to be indexed in the Baloo engine.
 *
 * It's a large collection of words along with their respective poistions and frequencies.
 * One typically never needs to have all of this in memory except when creating the
 * Document for indexing.
 *
 * This is why Documents can be created and saved into the database, but not fetched.
 */
class BALOO_ENGINE_EXPORT Document
{
public:
    Document();

    void addTerm(const QByteArray& term, int wdfInc = 1);
    void addBoolTerm(const QByteArray& term);
    void addPositionTerm(const QByteArray& term, int position = 0, int wdfInc = 1);

    quint64 id() const;
    void setId(quint64 id);

    QByteArray url() const;
    void setUrl(const QByteArray& url);

    /**
     * This flag is used to signify if the file needs its contents to be indexed.
     * It defaults to false
     */
    void setContentIndexing(bool val);
    bool contentIndexing() const;

    void addValue(int slotNum, const QByteArray& arr);

    void setData(const QByteArray& data);

private:
    quint64 m_id;

    struct TermData {
        int wdf;
        QVector<uint> positions;

        TermData() : wdf(0) {}
    };
    QMap<QByteArray, TermData> m_terms;

    QByteArray m_url;
    bool m_contentIndexing;

    QMap<quint64, QByteArray> m_slots;
    QByteArray m_data;

    friend class Database;
    friend class TermGeneratorTest;
};

inline QDebug operator<<(QDebug dbg, const Document &doc) {
    dbg << doc.id() << doc.url();
    return dbg;
}

}

#endif // BALOO_DOCUMENT_H
