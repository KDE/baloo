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

#include "tagremovejob.h"
#include "tag.h"
#include "util.h"

#include <QTimer>
#include <QVariant>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>

#include <QDBusConnection>
#include <QDBusMessage>

using namespace Baloo;

class TagRemoveJob::Private {
public:
    Tag tag;
};

TagRemoveJob::TagRemoveJob(const Tag& tag, QObject* parent)
    : ItemRemoveJob(parent)
    , d(new Private)
{
    d->tag = tag;
}

TagRemoveJob::~TagRemoveJob()
{
    delete d;
}

void TagRemoveJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void TagRemoveJob::doStart()
{
    if (d->tag.id().isEmpty()) {
        setError(Error_TagEmptyId);
        setErrorText("A tagid must not be provided when creating a tag");
        emitResult();
        return;
    }

    int id = deserialize("tag", d->tag.id());
    if (id <= 0) {
        setError(Error_TagInvalidId);
        setErrorText("Invalid id " + d->tag.id());
        emitResult();
        return;
    }

    QSqlQuery query(db(parent()));
    query.prepare("DELETE FROM tags where id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        setError(Error_ConnectionError);
        setErrorText(query.lastError().text());
        emitResult();
        return;
    }

    if (query.numRowsAffected() == 0) {
        setError(Error_TagDoesNotExist);
        setErrorText("Tag with ID " + d->tag.id() + " does not exist");
        emitResult();
        return;
    }

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/tags"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("removed"));

    QVariantList vl;
    vl.reserve(1);
    vl << d->tag.id();
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    Q_EMIT itemRemoved(d->tag);
    Q_EMIT tagRemoved(d->tag);
    emitResult();
}

