/*
 * This file is part of the KDE Baloo Project
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

#ifndef BALOO_ENGINEQUERY_H
#define BALOO_ENGINEQUERY_H

#include "engine_export.h"

#include <QByteArray>
#include <QVector>
#include <QDebug>

namespace Baloo {

class BALOO_ENGINE_EXPORT EngineQuery
{
public:
    enum Operation {
        Equal,
        StartsWith,
        And,
        Or,
        Phrase
    };

    EngineQuery();
    EngineQuery(const QByteArray& term, int pos = 0);
    EngineQuery(const QByteArray& term, Operation op, int pos = 0);
    EngineQuery(const QVector<EngineQuery> subQueries, Operation op);

    QByteArray term() const {
        return m_term;
    }

    int pos() const {
        return m_pos;
    }

    Operation op() const {
        return m_op;
    }

    void setOp(const Operation& op) {
        m_op = op;
    }

    bool leaf() const {
        return !m_term.isEmpty();
    }

    QVector<EngineQuery> subQueries() const {
        return m_subQueries;
    }

    bool operator ==(const EngineQuery& q) const {
        return m_term == q.m_term && m_pos == q.m_pos && m_op == q.m_op && m_subQueries == q.m_subQueries;
    }
private:
    QByteArray m_term;
    int m_pos;
    Operation m_op;

    QVector<EngineQuery> m_subQueries;
};

} // namespace Baloo

inline QDebug operator << (QDebug d, const Baloo::EngineQuery& q) {
    if (q.op() == Baloo::EngineQuery::And) {
        d << "[AND " << q.subQueries() << "]";
    } else if (q.op() == Baloo::EngineQuery::Or) {
        d << "[OR " << q.subQueries() << "]";
    } else if (q.op() == Baloo::EngineQuery::Phrase) {
        d << "[PHRASE " << q.subQueries() << "]";
    } else {
        Q_ASSERT(q.subQueries().isEmpty());
        d << "(" << q.term() << q.pos() << q.op() << ")";
    }
    return d;
}

#endif
