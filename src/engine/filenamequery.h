/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILENAMEQUERY_H
#define BALOO_FILENAMEQUERY_H

#include "termgenerator.h"

#include <QByteArray>
#include <QDebug>
#include <QVector>

#include <algorithm>

namespace Baloo
{

class BALOO_ENGINE_EXPORT FilenameQuery
{
public:
    FilenameQuery(const QString &filename, bool expandLast = false)
        : m_name(filename)
        , m_terms(TermGenerator::termList(filename))
        , m_expandLast(expandLast)
    {
        if (m_terms.empty() || m_terms.last().size() <= 2) {
            m_expandLast = false;
        }
    }

    bool match(const QByteArray &utf8Name) const
    {
        const auto terms = TermGenerator::termList(QString::fromUtf8(utf8Name));
        // Check if the filename (terms) contains the query (m_terms)
        const auto found = std::ranges::search(terms, m_terms);
        return !found.empty();
    }

    QString m_name;
    QByteArrayList m_terms;
    bool m_expandLast = false;
};

inline QDebug operator<<(QDebug d, const Baloo::FilenameQuery &q)
{
    QDebugStateSaver state(d);
    d.setAutoInsertSpaces(false);

    d << "[FILENAME " << q.m_name << ":";
    for (auto &term : q.m_terms) {
        d << " " << term;
    }
    if (q.m_expandLast) {
        d << "..";
    }
    return d << "]";
}

} // namespace Baloo
#endif
