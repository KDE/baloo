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

#include <QTimer>

#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>

TagSaveJob::TagSaveJob(Tag* tag, QObject* parent)
    : ItemSaveJob(parent)
    , m_tag(tag)
{
    Q_ASSERT(tag);
    qRegisterMetaType<KJob*>();
}

void TagSaveJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

namespace {
    int toInt(const QByteArray& arr) {
        return arr.mid(4).toInt(); // "tag:" takes 4 char
    }
}

void TagSaveJob::doStart()
{
    if (m_tag->id().isEmpty()) {
        setError(Error_TagNotCreated);
        setErrorText("Tags must have an id before saving");
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
    query.prepare("UPDATE tags SET name = ? WHERE id = ?");
    query.addBindValue(m_tag->name());
    query.addBindValue(toInt(m_tag->id()));

    if (!query.exec()) {
        m_tag->setId(QByteArray());
        setError(Error_TagExists);
        setErrorText("Tag with name " + m_tag->name() + " already exists");
        emitResult();
        return;
    }

    emit itemSaved(m_tag);
    emit tagSaved(m_tag);
    emitResult();
}

