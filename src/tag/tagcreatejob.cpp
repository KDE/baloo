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

#include "tagcreatejob.h"
#include "tag.h"

#include <QTimer>
#include <QVariant>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>

#include <QDBusConnection>
#include <QDBusMessage>

TagCreateJob::TagCreateJob(Tag* tag, QObject* parent)
    : ItemCreateJob(parent)
    , m_tag(tag)
{
    Q_ASSERT(tag);
    qRegisterMetaType<KJob*>();
}

void TagCreateJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void TagCreateJob::doStart()
{
    if (m_tag->id().size()) {
        setError(Error_TagIdProvided);
        setErrorText("A tagid must not be provided when creating a tag");
        emitResult();
        return;
    }

    if (m_tag->name().isEmpty()) {
        setError(Error_TagEmptyName);
        setErrorText("A Tag must have a name");
        emitResult();
        return;
    }

    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(QLatin1String("insert into tags (name) VALUES (?)"));
    query.addBindValue(m_tag->name());

    if (!query.exec()) {
        // FIXME: What about connection errors?
        setError(Error_TagExists);
        setErrorText(query.lastError().text());
        emitResult();
        return;
    }

    m_tag->setId(QByteArray("tag:") + QByteArray::number(query.lastInsertId().toInt()));


    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/tags"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("created"));

    QVariantList vl;
    vl.reserve(2);
    vl << m_tag->id();
    vl << m_tag->name();
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    emit itemCreated(m_tag);
    emit tagCreated(m_tag);
    emitResult();
}
