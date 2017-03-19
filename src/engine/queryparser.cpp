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
    : m_autoExpandSize(3)
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

EngineQuery QueryParser::parseQuery(const QString& text_, const QString& prefix)
{
    Q_ASSERT(!text_.isEmpty());

    QString text(text_);
    text.replace('_', ' ');

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

            // Remove all accents and lower it
            const QString denormalized = str.normalized(QString::NormalizationForm_KD).toLower();
            QString cleanString;
            Q_FOREACH (const QChar& ch, denormalized) {
                auto cat = ch.category();
                if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining && cat != QChar::Mark_Enclosing) {
                    cleanString.append(ch);
                }
            }

            str = cleanString.normalized(QString::NormalizationForm_KC);

            const QString term = prefix + str;
            const QByteArray arr = term.toUtf8();

            position++;
            if (inDoubleQuotes || inSingleQuotes || inPhrase) {
                phraseQueries << EngineQuery(arr, position);
            }
            else {
                if (m_autoExpandSize && arr.size() >= m_autoExpandSize) {
                    queries << EngineQuery(arr, EngineQuery::StartsWith, position);
                } else {
                    queries << EngineQuery(arr, position);
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
            if (m_autoExpandSize && q.term().size() >= m_autoExpandSize) {
                q.setOp(EngineQuery::StartsWith);
            } else {
                q.setOp(EngineQuery::Equal);
            }
        }
        queries << phraseQueries;
        phraseQueries.clear();
    }

    if (queries.size() == 1) {
        return queries.first();
    }
    return EngineQuery(queries, EngineQuery::And);
}

void QueryParser::setAutoExapandSize(int size)
{
    m_autoExpandSize = size;
}
