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

#include "tagrelationremovejob.h"
#include "tagrelation.h"
#include "tag.h"

#include <QTimer>
#include <QVariant>

#include <QSqlQuery>
#include <QSqlError>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

using namespace Baloo;

class TagRelationRemoveJob::Private {
public:
    TagRelation relation;
};

TagRelationRemoveJob::TagRelationRemoveJob(const TagRelation& tagRelation, QObject* parent)
    : RelationRemoveJob(parent)
    , d(new Private)
{
    d->relation = tagRelation;
}

TagRelationRemoveJob::~TagRelationRemoveJob()
{
    delete d;
}

void TagRelationRemoveJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

namespace {
    int toInt(const QByteArray& arr) {
        return arr.mid(4).toInt(); // "tag:" takes 4 char
    }
}

void TagRelationRemoveJob::doStart()
{
    if (d->relation.item().id().isEmpty() || d->relation.tag().id().isEmpty()) {
        setError(Error_EmptyIds);
        setErrorText("The Item or Tag id is empty");
        emitResult();
        return;
    }

    int id = toInt(d->relation.tag().id());
    if (id <= 0) {
        setError(Error_InvalidTagId);
        setErrorText("Invalid Tag ID");
        emitResult();
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM tagRelations where tid = ? AND rid = ?");
    query.addBindValue(id);
    query.addBindValue(d->relation.item().id());

    if (!query.exec()) {
        setError(Error_ConnectionError);
        setErrorText(query.lastError().text());
        emitResult();
        return;
    }

    if (query.numRowsAffected() == 0) {
        setError(Error_RelationDoesNotExist);
        setErrorText("The tag relation does not exist");
        emitResult();
        return;
    }

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/tagrelations"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("removed"));

    QVariantList vl;
    vl.reserve(2);
    vl << d->relation.tag().id();
    vl << d->relation.item().id();
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    emit relationRemoved(d->relation);
    emit tagRelationRemoved(d->relation);
    emitResult();
}
