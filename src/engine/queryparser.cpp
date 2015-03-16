/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014-2015  Vishesh Handa <vhanda@kde.org>
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

#include "queryparser.h"
#include "enginequery.h"

#include <QTextBoundaryFinder>
#include <QStringList>
#include <QVector>

using namespace Baloo;

QueryParser::QueryParser()
    : m_autoExpand(true)
{
}

namespace {
    bool containsSpace(const QString& string) {
        Q_FOREACH (const QChar& ch, string) {
            if (ch.isSpace())
                return true;
        }

        return false;
    }
}

EngineQuery QueryParser::parseQuery(const QString& text, const QString& prefix)
{
    Q_ASSERT(!text.isEmpty());

    QVector<EngineQuery> queries;
    QVector<EngineQuery> phraseQueries;

    int start = 0;
    int end = 0;
    int position = 0;

    bool inDoubleQuotes = false;
    bool inSingleQuotes = false;
    bool inPhrase = false;

    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
            //
            // Check the previous delimiter
            int pos = bf.position();
            if (pos != end) {
                QString delim = text.mid(end, pos-end);
                if (delim.contains(QLatin1Char('"'))) {
                    if (inDoubleQuotes) {
                        queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
                        phraseQueries.clear();
                        inDoubleQuotes = false;
                    }
                    else {
                        inDoubleQuotes = true;
                    }
                }
                else if (delim.contains(QLatin1Char('\''))) {
                    if (inSingleQuotes) {
                        queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
                        phraseQueries.clear();
                        inSingleQuotes = false;
                    }
                    else {
                        inSingleQuotes = true;
                    }
                }
                else if (!containsSpace(delim)) {
                    if (!inPhrase && !queries.isEmpty()) {
                        EngineQuery q = queries.takeLast();
                        q.setOp(EngineQuery::Equal);
                        phraseQueries << q;
                    }
                    inPhrase = true;
                }
                else if (inPhrase && !phraseQueries.isEmpty()) {
                    queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
                    phraseQueries.clear();
                    inPhrase = false;
                }
            }

            start = bf.position();
            continue;
        }
        else if (bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            end = bf.position();

            QString str = text.mid(start, end - start);

            // Get the string ready for saving
            str = str.toLower();

            // Remove all accents
            const QString denormalized = str.normalized(QString::NormalizationForm_KD);
            QString cleanString;
            Q_FOREACH (const QChar& ch, denormalized) {
                auto cat = ch.category();
                if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining && cat != QChar::Mark_Enclosing) {
                    cleanString.append(ch);
                }
            }

            str = cleanString.normalized(QString::NormalizationForm_KC);
            Q_FOREACH (const QString& t, str.split(QLatin1Char('_'), QString::SkipEmptyParts)) {
                const QString term = prefix + t;
                const QByteArray arr = term.toUtf8();

                position++;
                if (inDoubleQuotes || inSingleQuotes || inPhrase) {
                    phraseQueries << EngineQuery(arr, position);
                }
                else {
                    if (m_autoExpand) {
                        queries << EngineQuery(arr, EngineQuery::StartsWith, position);
                    } else {
                        queries << EngineQuery(arr, position);
                    }
                }
            }
        }
    }

    if (inPhrase) {
        queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
        phraseQueries.clear();
        inPhrase = false;
    }

    if (!phraseQueries.isEmpty()) {
        for (EngineQuery& q : phraseQueries) {
            q.setOp(EngineQuery::StartsWith);
        }
        queries << phraseQueries;
        phraseQueries.clear();
    }

    if (queries.size() == 1) {
        return queries.first();
    }
    return EngineQuery(queries, EngineQuery::And);
}

void QueryParser::setAutoExapand(bool autoexpand)
{
    m_autoExpand = autoexpand;
}
