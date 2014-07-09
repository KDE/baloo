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

#ifndef BALOO_WILDCARDPOSTINGSOURCE_H
#define BALOO_WILDCARDPOSTINGSOURCE_H

#include <xapian.h>
#include <QString>
#include <QRegularExpression>

namespace Baloo {

class WildcardPostingSource : public Xapian::PostingSource
{
public:
    WildcardPostingSource(const QString& word, const QString& prefix);

    virtual void init(const Xapian::Database& db);

    virtual Xapian::docid get_docid() const;

    virtual void next(Xapian::weight min_wt);
    virtual bool at_end() const;

    virtual Xapian::doccount get_termfreq_min() const;
    virtual Xapian::doccount get_termfreq_est() const;
    virtual Xapian::doccount get_termfreq_max() const;

    virtual PostingSource* clone() const {
        return new WildcardPostingSource(m_word, QString::fromUtf8(m_prefix));
    }
private:
    bool isMatch(uint docid);

    Xapian::Database m_db;
    Xapian::PostingIterator m_iter;
    Xapian::PostingIterator m_end;

    bool m_first;

    QRegularExpression m_regex;
    QByteArray m_prefix;
    QString m_word;
};
}

#endif // BALOO_WILDCARDPOSTINGSOURCE_H
