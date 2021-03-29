/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "queryparser.h"
#include "enginequery.h"

#include <QTextBoundaryFinder>
#include <QVector>

using namespace Baloo;

QueryParser::QueryParser()
    : m_autoExpandSize(3)
{
}

EngineQuery QueryParser::parseQuery(const QString& text_, const QByteArray& prefix)
{
    Q_ASSERT(!text_.isEmpty());

    QString text(text_);
    text.replace(QLatin1Char('_'), QLatin1Char(' '));

    QVector<EngineQuery> queries;
    QVector<EngineQuery> phraseQueries;

    int start = 0;
    int end = 0;

    bool inDoubleQuotes = false;
    bool inSingleQuotes = false;
    bool inPhrase = false;

    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        int pos = bf.position();
        if (!(bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem)) {
            //
            // Check the previous delimiter
            if (pos != end) {
                QString delim = text_.mid(end, pos-end);
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
                else if (std::none_of(delim.constBegin(), delim.constEnd(),
                    [](const QChar& ch) { return ch.isSpace();
                })) {
                    if (phraseQueries.isEmpty() && !queries.isEmpty()) {
                        EngineQuery q = queries.takeLast();
                        q.setOp(EngineQuery::Equal);
                        phraseQueries << q;
                        inPhrase = true;
                    }
                }
                else if (inPhrase && !phraseQueries.isEmpty()) {
                    queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
                    phraseQueries.clear();
                    inPhrase = false;
                }
                end = pos;
            }
        }

        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
            start = pos;
            continue;
        }
        else if (bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            end = bf.position();

            QString str = text.mid(start, end - start);

            // Remove all accents and lower it
            const QString denormalized = str.normalized(QString::NormalizationForm_KD).toLower();
            QString cleanString;
            for (const QChar& ch : denormalized) {
                auto cat = ch.category();
                if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining && cat != QChar::Mark_Enclosing) {
                    cleanString.append(ch);
                }
            }

            str = cleanString.normalized(QString::NormalizationForm_KC);

            const QByteArray arr = prefix + str.toUtf8();

            if (inDoubleQuotes || inSingleQuotes || inPhrase) {
                phraseQueries << EngineQuery(arr, EngineQuery::Equal);
            }
            else {
                if (m_autoExpandSize && arr.size() >= m_autoExpandSize) {
                    queries << EngineQuery(arr, EngineQuery::StartsWith);
                } else {
                    queries << EngineQuery(arr, EngineQuery::Equal);
                }
            }
        }
    }

    if (inPhrase) {
        queries << EngineQuery(phraseQueries, EngineQuery::Phrase);

    } else if (!phraseQueries.isEmpty()) {
        for (EngineQuery& q : phraseQueries) {
            if (m_autoExpandSize && q.term().size() >= m_autoExpandSize) {
                q.setOp(EngineQuery::StartsWith);
            } else {
                q.setOp(EngineQuery::Equal);
            }
        }
        queries << phraseQueries;
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
