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

#include <QByteArray>
#include <QVector>

namespace Baloo {

class EngineQuery
{
public:
    enum Operation {
        Equal,
        And,
        Or
    };

    EngineQuery();
    EngineQuery(const QByteArray& term, int pos = 0);
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

    bool leaf() const {
        return !m_term.isEmpty();
    }

    QVector<EngineQuery> subQueries() const {
        return m_subQueries;
    }

private:
    QByteArray m_term;
    int m_pos;

    Operation m_op;
    QVector<EngineQuery> m_subQueries;
};
}

#endif
