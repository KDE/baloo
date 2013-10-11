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

#include <QTimer>
#include <QVariant>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>

TagRemoveJob::TagRemoveJob(Tag* tag, QObject* parent)
    : ItemRemoveJob(parent)
    , m_tag(tag)
{
    Q_ASSERT(tag);
    qRegisterMetaType<KJob*>();
}

void TagRemoveJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

namespace {
    int toInt(const QByteArray& arr) {
        return arr.mid(4).toInt(); // "tag:" takes 4 char
    }
}

void TagRemoveJob::doStart()
{
    if (m_tag->id().isEmpty()) {
        setError(Error_TagEmptyId);
        setErrorText("A tagid must not be provided when creating a tag");
        emitResult();
        return;
    }

    int id = toInt(m_tag->id());
    if (id <= 0) {
        setError(Error_TagInvalidId);
        setErrorText("Invalid id " + m_tag->id());
        emitResult();
        return;
    }

    QSqlQuery query;
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
        setErrorText("Tag with ID " + m_tag->id() + " does not exist");
        emitResult();
        return;
    }

    emit itemRemoved(m_tag);
    emit tagRemoved(m_tag);
    emitResult();
}

