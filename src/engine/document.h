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

/**
 * A document represents a file which is being indexed inside Baloo's engine.
 */
class BALOO_ENGINE_EXPORT Document
{
public:
    Document();

    void addTerm(const QByteArray& term);
    void addPositionTerm(const QByteArray& term, int position = 0, int wdfInc = 1);

    void indexText(const QString& text);
    void indexText(const QString& text, const QByteArray& prefix);

    uint id() const;
    void setId(uint id);

    QByteArray url() const;
    void setUrl(const QByteArray& url);

    bool operator ==(const Document& rhs) const;

    /**
     * Setting the level to 0 indicates that you do not want the level
     * to be stored
     */
    void setIndexingLevel(int level);
    int indexingLevel() const;

    // FIXME: No one should really ever need all the terms.
    //        Currently we're only using them in tests, which is again strange
    QVector<QByteArray> terms() const { return m_terms; }

    void addValue(int slotNum, const QByteArray& arr);

private:
    uint m_id;
    QVector<QByteArray> m_terms;
    QByteArray m_url;
    int m_indexingLevel;

    QMap<uint, QByteArray> m_slots;

    friend class Database;
};

inline QDebug operator<<(QDebug dbg, const Document &doc) {
    dbg << doc.id() << doc.terms() << doc.url() << doc.indexingLevel();
    return dbg;
}

}

#endif // BALOO_DOCUMENT_H
