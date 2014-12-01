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

#include "wildcardpostingsource.h"

#include <QDebug>

using namespace Baloo;

WildcardPostingSource::WildcardPostingSource(const QString& word, const QString& prefix)
    : m_first(true)
{
    m_word = word;
    m_prefix = prefix.toUtf8();

    QString f = word;
    f.replace(QLatin1Char('?'), QLatin1Char('.'));
    f.replace(QStringLiteral("*"), QStringLiteral(".*"));
    f = QLatin1String("^") + f + QLatin1String("$");
    m_regex = QRegularExpression(f);
}

void WildcardPostingSource::init(const Xapian::Database& db)
{
    m_db = db;
    m_iter = db.postlist_begin("");
    m_end = db.postlist_end("");
    m_first = true;

    // FIXME: Maybe we want to do a query for all documents with the prefix
    // and then just use that as a matching set?
}

bool WildcardPostingSource::isMatch(uint docid)
{
    Xapian::Document doc = m_db.get_document(docid);
    auto tit = doc.termlist_begin();
    tit.skip_to(m_prefix.constData());

    while (1) {
        if (tit == doc.termlist_end())
            break;

        std::string str = *tit;
        QByteArray data = QByteArray::fromRawData(str.c_str(), str.length());

        if (!data.startsWith(m_prefix)) {
            break;
        }

        QString s = QString::fromUtf8(data.mid(m_prefix.length()));
        if (m_regex.match(s).hasMatch()) {
            return true;
        }

        tit++;
    }

    return false;
}

void WildcardPostingSource::next(Xapian::weight)
{
    do {
        // This has been done so that we do not skip the first element
        // as the PostingSource is supposed to start one before the first element
        // whereas Xapian::Database::postlist_begin gives us the first element
        //
        if (m_first) {
            m_first = false;
        }
        else {
            m_iter++;
        }

        if (m_iter == m_end) {
            return;
        }
    } while (!isMatch(*m_iter));
}

bool WildcardPostingSource::at_end() const
{
    return m_iter == m_end;
}

Xapian::docid WildcardPostingSource::get_docid() const
{
    return *m_iter;
}

//
// Term Frequiencies
//
Xapian::doccount WildcardPostingSource::get_termfreq_min() const
{
    return 0;
}

Xapian::doccount WildcardPostingSource::get_termfreq_est() const
{
    return m_db.get_doccount();
}

Xapian::doccount WildcardPostingSource::get_termfreq_max() const
{
    return m_db.get_doccount();
}
