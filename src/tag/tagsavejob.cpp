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

#include "tagsavejob.h"
#include "tag.h"
#include "util.h"

#include <QTimer>

#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>
#include <QDBusConnection>
#include <QDBusMessage>

using namespace Baloo;

class TagSaveJob::Private {
public:
    Tag tag;
};

TagSaveJob::TagSaveJob(const Tag& tag, QObject* parent)
    : ItemSaveJob(parent)
    , d(new Private)
{
    d->tag = tag;
}

TagSaveJob::~TagSaveJob()
{
    delete d;
}

void TagSaveJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void TagSaveJob::doStart()
{
    if (d->tag.id().isEmpty()) {
        setError(Error_TagNotCreated);
        setErrorText("Tags must have an id before saving");
        emitResult();
        return;
    }

    if (d->tag.name().isEmpty()) {
        setError(Error_TagEmptyName);
        setErrorText("A Tag must have a name");
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
    query.prepare("UPDATE tags SET name = ? WHERE id = ?");
    query.addBindValue(d->tag.name());
    query.addBindValue(id);

    if (!query.exec()) {
        d->tag.setId(QByteArray());
        setError(Error_TagExists);
        setErrorText("Tag with name " + d->tag.name() + " already exists");
        emitResult();
        return;
    }

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/tags"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("modified"));

    QVariantList vl;
    vl.reserve(2);
    vl << d->tag.id();
    vl << d->tag.name();
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    emit itemSaved(d->tag);
    emit tagSaved(d->tag);
    emitResult();
}

