/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BALOO_LUCENESEARCHSTORE_H
#define BALOO_LUCENESEARCHSTORE_H

#include <lucene++/LuceneHeaders.h>
#include "searchstore.h"
#include "term.h"
#include <QMutex>

namespace Baloo {
    class LuceneSearchStore : public SearchStore {
    public:
        explicit LuceneSearchStore(QObject* parent = 0);
        virtual ~LuceneSearchStore();

        virtual int exec(const Query& query);
        virtual void close(int queryId);
        virtual bool next(int queryId);

        virtual QByteArray id(int queryId);
        virtual QString filePath(int queryId);

        virtual void setIndexPath(const QString& path);
        virtual QString dbPath();

    protected:
        virtual Lucene::QueryPtr constructQuery(const QString& property,
                                                const QVariant& value,
                                                Term::Comparator com) = 0;
        virtual Lucene::QueryPtr constructFilterQuery(int year, int month, int day);
        virtual Lucene::QueryPtr finalizeQuery(const Lucene::QueryPtr& query);
        virtual Lucene::QueryPtr applyIncludeFolder(const Lucene::QueryPtr& q, QString includeFolder);
        virtual QString constructFilePath(int docid) = 0;
        virtual Lucene::QueryPtr convertTypes(const QStringList& types) = 0;
        virtual QByteArray idPrefix() = 0;

        Lucene::DocumentPtr docForQuery(int queryId);

        Lucene::QueryPtr andQuery(const Lucene::QueryPtr& a, const Lucene::QueryPtr& b);

        Lucene::IndexReaderPtr luceneIndex();

        QMutex m_mutex;

        Lucene::QueryPtr toLuceneQuery(const Term& term);
        Lucene::QueryPtr toLuceneQuery(Lucene::BooleanClause::Occur op, const QList<Term>& terms);
        bool isEmpty(const Lucene::QueryPtr& query);

        typedef Lucene::Collection<Lucene::ScoreDocPtr> LuceneResults;
        struct Result {
            LuceneResults hits;
            LuceneResults::iterator it;

            uint lastId;
            QString lastUrl;
        };

        QHash<int, Result> m_queryMap;
        int m_nextId;

        QString m_indexPath;

        Lucene::IndexReaderPtr m_reader;
};
}

#endif //BALOO_LUCENESEARCHSTORE_H