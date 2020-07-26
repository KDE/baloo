/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "enginequery.h"

namespace Baloo {

EngineQuery::EngineQuery()
    : m_op(Equal)
{
}

EngineQuery::EngineQuery(const QByteArray& term, EngineQuery::Operation op)
    : m_term(term)
    , m_op(op)
{
}

EngineQuery::EngineQuery(const QVector<EngineQuery> &subQueries, Operation op)
    : m_op(op)
    , m_subQueries(subQueries)
{
}

} // namespace Baloo
