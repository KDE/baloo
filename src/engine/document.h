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

class WriteTransaction;
class TermGeneratorTest;

/**
 * A document represents an indexed file to be stored in the Baloo engine.
 *
 * It is a large collection of words along with their respective positions.
 * One typically never needs to have all of this in memory except when creating the
 * Document for indexing.
 *
 * This is why Documents can be created and saved into the database, but not fetched.
 */
class BALOO_ENGINE_EXPORT Document
{
public:
    Document();

    void addTerm(const QByteArray& term);
    void addPositionTerm(const QByteArray& term, int position = 0);

    void addXattrTerm(const QByteArray& term);
    void addXattrPositionTerm(const QByteArray& term, int position = 0);

    void addFileNameTerm(const QByteArray& term);
    void addFileNamePositionTerm(const QByteArray& term, int position = 0);

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

    void setMTime(quint32 val) { m_mTime = val; }
    void setCTime(quint32 val) { m_cTime = val; }

    void setData(const QByteArray& data);

private:
    quint64 m_id;

    struct TermData {
        QVector<uint> positions;
    };
    QMap<QByteArray, TermData> m_terms;
    QMap<QByteArray, TermData> m_xattrTerms;
    QMap<QByteArray, TermData> m_fileNameTerms;

    QByteArray m_url;
    bool m_contentIndexing;

    quint32 m_mTime;
    quint32 m_cTime;
    QByteArray m_data;

    friend class WriteTransaction;
    friend class TermGeneratorTest;
};

inline QDebug operator<<(QDebug dbg, const Document &doc) {
    dbg << doc.id() << doc.url();
    return dbg;
}

}

#endif // BALOO_DOCUMENT_H
