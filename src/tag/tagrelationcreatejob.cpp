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
    TagRelation relation;
};

TagRelationCreateJob::TagRelationCreateJob(const TagRelation& relation, QObject* parent)
    : RelationCreateJob(parent)
    , d(new Private)
{
    d->relation = relation;
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
    if (d->relation.item().id().isEmpty() || d->relation.tag().id().isEmpty()) {
        setError(Error_InvalidRelation);
        setErrorText("Invalid or Empty Ids provided");
        emitResult();
        return;
    }

    int id = deserialize("tag", d->relation.tag().id());
    if (id <= 0) {
        setError(Error_InvalidTagId);
        setErrorText("Invalid Tag ID");
        emitResult();
        return;
    }

    QSqlQuery query;
    query.prepare("insert into tagRelations (tid, rid) values (?, ?)");
    query.addBindValue(id);
    query.addBindValue(d->relation.item().id());

    if (!query.exec()) {
        setError(Error_RelationExists);
        setErrorText("The relation already exists");
        emitResult();
        return;
    }

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/tagrelations"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("added"));

    QVariantList vl;
    vl.reserve(2);
    vl << d->relation.tag().id();
    vl << d->relation.item().id();
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    emit relationCreated(d->relation);
    emit tagRelationCreated(d->relation);
    emitResult();
}
