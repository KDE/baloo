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

#include "enginequery.h"

using namespace Baloo;

EngineQuery::EngineQuery()
    : m_pos(0)
    , m_op(Equal)
{
}

EngineQuery::EngineQuery(const QByteArray& term, int pos)
    : m_term(term)
    , m_pos(pos)
    , m_op(Equal)
{
}

EngineQuery::EngineQuery(const QByteArray& term, EngineQuery::Operation op, int pos)
    : m_term(term)
    , m_op(op)
    , m_pos(pos)
{
}

EngineQuery::EngineQuery(const QVector<EngineQuery> subQueries, Operation op)
    : m_pos(0)
    , m_op(op)
    , m_subQueries(subQueries)
{
}
