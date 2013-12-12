/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tagrelationfetchjob.h"
#include "tagrelation.h"
#include "tag.h"
#include "tagfetchjob.h"
#include "util.h"

#include <QTimer>

#include <QSqlError>
#include <QSqlQuery>
#include <KDebug>

using namespace Baloo;

class TagRelationFetchJob::Private {
public:
    TagRelation relation;
};

TagRelationFetchJob::TagRelationFetchJob(const TagRelation& relation, QObject* parent)
    : RelationFetchJob(parent)
    , d(new Private)
{
    d->relation = relation;
}

TagRelationFetchJob::TagRelationFetchJob(const Tag& tag, QObject* parent)
    : RelationFetchJob(parent)
    , d(new Private)
{
    d->relation = TagRelation(tag);
}

TagRelationFetchJob::TagRelationFetchJob(const Item& item, QObject* parent)
    : RelationFetchJob(parent)
    , d(new Private)
{
    d->relation = TagRelation(item);
}

TagRelationFetchJob::~TagRelationFetchJob()
{
    delete d;
}

void TagRelationFetchJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void TagRelationFetchJob::doStart()
{
    Item& item = d->relation.item();
    Tag& tag = d->relation.tag();

    if (item.id().isEmpty()) {
        // We must first fetch the tag
        if (tag.id().isEmpty() || tag.name().isEmpty()) {
            TagFetchJob* tagFetchJob = new TagFetchJob(tag, parent());
            connect(tagFetchJob, SIGNAL(tagReceived(Baloo::Tag)), SLOT(slotTagReceived(Baloo::Tag)));
            tagFetchJob->start();
        }
        else {
            slotTagReceived(d->relation.tag());
        }
    }
    else {
        QSqlQuery query(db(parent()));
        query.prepare(QLatin1String("select tid, name from tagRelations, tags where "
                                    "rid = ? and tags.id = tagRelations.tid"));
        query.addBindValue(item.id());

        if (!query.exec()) {
            setError(Error_ConnectionError);
            setErrorText(query.lastError().text());
            emitResult();
            return;
        }

        bool found = false;
        while (query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();

            TagRelation rel(d->relation);
            rel.tag().setId(serialize("tag", id));
            rel.tag().setName(name);

            Q_EMIT relationReceived(rel);
            Q_EMIT tagRelationReceived(rel);
            found = true;
        }

        if (!found) {
            setError(Error_NoRelation);
            setErrorText("No relation could be found");
        }

        emitResult();
    }
}

void TagRelationFetchJob::slotTagReceived(const Tag& tag)
{
    d->relation.setTag(tag);

    int id = deserialize("tag", tag.id());
    if (id <= 0) {
        setError(Error_InvalidTagId);
        setErrorText("Invalid Tag ID");
        emitResult();
        return;
    }

    QSqlQuery query(db(parent()));
    query.prepare(QLatin1String("select rid from tagRelations where tid = ?"));
    query.addBindValue(id);

    if (!query.exec()) {
        setError(Error_ConnectionError);
        setErrorText(query.lastError().text());
        emitResult();
        return;
    }

    if (query.next()) {
        d->relation.item().setId(query.value(0).toByteArray());

        Q_EMIT relationReceived(d->relation);
        Q_EMIT tagRelationReceived(d->relation);
        emitResult();
    }
    else {
        setError(Error_NoRelation);
        setErrorText("No relation could be found");
        emitResult();
        return;
    }
}

