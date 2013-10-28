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
#include "item.h"
#include "tag.h"
#include "tagfetchjob.h"

#include <QTimer>
#include <QVariant>

#include <QSqlQuery>
#include <QSqlError>
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
            TagFetchJob* tagFetchJob = tag.fetch();
            connect(tagFetchJob, SIGNAL(tagReceived(Tag)), SLOT(slotTagReceived(Tag)));
            tagFetchJob->start();
        }
        else {
            slotTagReceived(d->relation.tag());
        }
    }
    else {
        QSqlQuery query;
        query.prepare(QLatin1String("select tid from tagRelations where rid = ?"));
        query.addBindValue(item.id());

        if (!query.exec()) {
            setError(Error_ConnectionError);
            setErrorText(query.lastError().text());
            emitResult();
            return;
        }

        if (query.next()) {
            int id = query.value(0).toInt();
            d->relation.tag().setId(QByteArray("tag:") + QByteArray::number(id));

            emit relationReceived(d->relation);
            emit tagRelationReceived(d->relation);
            emitResult();
        }
        else {
            setError(Error_NoRelation);
            setErrorText("No relation could be found");
            emitResult();
            return;
        }
    }
}

namespace {
    int toInt(const QByteArray& arr) {
        return arr.mid(4).toInt(); // "tag:" takes 4 char
    }
}

void TagRelationFetchJob::slotTagReceived(const Tag& tag)
{
    d->relation.setTag(tag);

    int id = toInt(tag.id());
    if (id <= 0) {
        setError(Error_InvalidTagId);
        setErrorText("Invalid Tag ID");
        emitResult();
        return;
    }

    QSqlQuery query;
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

        emit relationReceived(d->relation);
        emit tagRelationReceived(d->relation);
        emitResult();
    }
    else {
        setError(Error_NoRelation);
        setErrorText("No relation could be found");
        emitResult();
        return;
    }
}

