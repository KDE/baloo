/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
    };

    struct PhraseTerm {
        QByteArray m_term;
        Operation m_op;

        bool operator==(const PhraseTerm &pt) const
        {
            return m_op == pt.m_op && m_term == pt.m_term;
        }
    };

    EngineQuery(const QVector<PhraseTerm> &subQueries)
        : m_subQueries(subQueries)
    {
    }

    bool empty() {
        return m_subQueries.isEmpty();
    }

    QVector<PhraseTerm> subQueries() const
    {
        return m_subQueries;
    }

    bool operator==(const EngineQuery &q) const
    {
        return m_subQueries == q.m_subQueries;
    }

private:
    QVector<PhraseTerm> m_subQueries;
};

inline QDebug operator<<(QDebug d, const Baloo::EngineQuery& q)
{
    QDebugStateSaver state(d);
    d.setAutoInsertSpaces(false);

    d << "[PHRASE";
    for (auto &sq : q.subQueries()) {
        if (sq.m_op == Baloo::EngineQuery::Operation::StartsWith) {
            d << " " << sq.m_term << "..";
        } else {
            d << " " << sq.m_term;
        }
    }
    return d << "]";
}

/**
 * Helper for QTest
 * \sa QTest::toString
 *
 * @since: 5.70
 */
inline char *toString(const EngineQuery& query)
{
    QString buffer;
    QDebug stream(&buffer);
    stream << query;
    return qstrdup(buffer.toUtf8().constData());
}

} // namespace Baloo
#endif
