/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include <QTextBoundaryFinder>
#include <QStringList>
#include <QDebug>

using namespace Baloo;

QueryParser::QueryParser()
    : m_db(0)
{
}

void QueryParser::setDatabase(Xapian::Database* db)
{
    m_db = db;
}

Xapian::Query QueryParser::parseQuery(const QString& text)
{
    /*
    Xapian::QueryParser parser;
    parser.set_default_op(Xapian::Query::OP_AND);

    if (m_db)
        parser.set_database(*m_db);

    int flags = Xapian::QueryParser::FLAG_PHRASE | Xapian::QueryParser::FLAG_PARTIAL;

    std::string stdString(text.toUtf8().constData());
    return parser.parse_query(stdString, flags);
    */

    if (text.isEmpty()) {
        return Xapian::Query();
    }

    QList<Xapian::Query> queries;

    int start = 0;
    int end = 0;
    int position = 0;

    // TODO: Mark in phrase when starting with a " and ending with a "
    //       or ' and '

    // TODO: Mark in phrase when certain words are separated by punctuations
    // TODO: Each word sepearted by x, auto expand it to each of these
    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
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
            Q_FOREACH (const QString& term, str.split(QLatin1Char('_'), QString::SkipEmptyParts)) {
                QByteArray arr = term.toUtf8();
                std::string stdString(arr.constData(), arr.size());

                position++;
                queries << Xapian::Query(stdString, 1, position);
            }
        }
    }

    return Xapian::Query(Xapian::Query::OP_AND, queries.begin(), queries.end());
}
