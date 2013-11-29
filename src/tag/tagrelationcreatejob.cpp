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

#include "tagrelationcreatejob.h"
#include "tagrelation.h"
#include "util.h"

#include <QTimer>
#include <QVariant>

#include <QSqlQuery>
#include <QSqlError>
#include <KDebug>

#include <QDBusConnection>
#include <QDBusMessage>

using namespace Baloo;

class TagRelationCreateJob::Private {
public:
    TagRelationCreateJob* q;
    TagRelation m_relation;

    QList<Item> m_itemList;
    QList<Tag> m_tagList;

    void createSingleRelation();
    void createMultipleRelations();
};

TagRelationCreateJob::TagRelationCreateJob(const TagRelation& relation, QObject* parent)
    : RelationCreateJob(parent)
    , d(new Private)
{
    d->q = this;
    d->m_relation = relation;
}

TagRelationCreateJob::TagRelationCreateJob(const QList<Item>& items, const QList<Tag>& tags,
                                           QObject* parent)
    : RelationCreateJob(parent)
    , d(new Private)
{
    d->q = this;
    d->m_itemList = items;
    d->m_tagList = tags;
}

TagRelationCreateJob::~TagRelationCreateJob()
{
    delete d;
}

void TagRelationCreateJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void TagRelationCreateJob::doStart()
{
    if (d->m_itemList.isEmpty() && d->m_tagList.isEmpty()) {
        const Tag& t = d->m_relation.tag();
        bool hasTagNameOrId = (t.name().size() || t.id().size());

        if (d->m_relation.item().id().isEmpty() || !hasTagNameOrId) {
            setError(Error_InvalidRelation);
            setErrorText("Invalid or Empty Ids provided");
            emitResult();
            return;
        }
        else {
            if (!t.id().size()) {
                TagFetchJob* job = new TagFetchJob(t, parent());
                connect(job, SIGNAL(tagReceived(Baloo::Tag)),
                        this, SLOT(slotTagFetched(Baloo::Tag)));
                connect(job, SIGNAL(finished(KJob*)),
                        this, SLOT(slotTagFetchJobFinished(KJob*)));
                job->start();
            }
            else {
                d->createSingleRelation();
            }
        }
    }
    else {
        // FIXME: Make sure all those tags have ids, if not fetch them!
        d->createMultipleRelations();
    }
}

void TagRelationCreateJob::Private::createSingleRelation()
{
    int id = deserialize("tag", m_relation.tag().id());
    if (id <= 0) {
        q->setError(Error_InvalidTagId);
        q->setErrorText("Invalid Tag ID");
        q->emitResult();
        return;
    }

    QSqlQuery query(db(q->parent()));
    query.prepare("insert into tagRelations (tid, rid) values (?, ?)");
    query.addBindValue(id);
    query.addBindValue(m_relation.item().id());

    if (!query.exec()) {
        q->setError(Error_RelationExists);
        q->setErrorText("The relation already exists");
        q->emitResult();
        return;
    }

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/tagrelations"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("added"));

    QVariantList vl;
    vl.reserve(2);
    vl << m_relation.tag().id();
    vl << m_relation.item().id();
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    Q_EMIT q->relationCreated(m_relation);
    Q_EMIT q->tagRelationCreated(m_relation);
    q->emitResult();
}

void TagRelationCreateJob::slotTagFetched(const Tag& tag)
{
    d->m_relation.setTag(tag);
    d->createSingleRelation();
}

void TagRelationCreateJob::slotTagFetchJobFinished(KJob* job)
{
    if (!job->error())
        return;

    switch (job->error()) {
        case TagFetchJob::Error_ConnectionError:
            setError(Error_ConnectionError);
            setErrorText(job->errorText());
            break;

        case TagFetchJob::Error_TagDoesNotExist:
        case TagFetchJob::Error_TagInvalidId:
            setError(Error_InvalidTagId);
            setErrorText(job->errorText());
            break;

        default:
            break;
    }

    emitResult();
}

void TagRelationCreateJob::Private::createMultipleRelations()
{
    QSqlDatabase sqlDb(db(q->parent()));
    sqlDb.transaction();

    QSqlQuery query(sqlDb);
    query.prepare("insert into tagRelations (tid, rid) values (:tid, :rid)");

    QList<TagRelation> newRelations;
    Q_FOREACH (const Item& item, m_itemList) {
        QByteArray itemId = item.id();

        Q_FOREACH (const Tag& tag, m_tagList) {
            int tagId = deserialize("tag", tag.id());
            Q_ASSERT(tagId > 0);

            query.bindValue(":tid", tagId);
            query.bindValue(":rid", itemId);

            // The only reason there would be an error is if the relation already exists!
            if (!query.exec()) {
                continue;
            }

            newRelations << TagRelation(tag, item);
        }
    }
    sqlDb.commit();

    Q_FOREACH (const TagRelation& rel, newRelations) {
        QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/tagrelations"),
                                                          QLatin1String("org.kde"),
                                                          QLatin1String("added"));

        QVariantList vl;
        vl.reserve(2);
        vl << rel.tag().id();
        vl << rel.item().id();
        message.setArguments(vl);

        QDBusConnection::sessionBus().send(message);

        Q_EMIT q->relationCreated(rel);
        Q_EMIT q->tagRelationCreated(rel);
    }

    q->emitResult();
}


