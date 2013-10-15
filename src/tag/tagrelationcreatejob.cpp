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

TagRelationCreateJob::TagRelationCreateJob(TagRelation* tagRelation, QObject* parent)
    : RelationCreateJob(parent)
    , m_tagRelation(tagRelation)
{
    Q_ASSERT(tagRelation);
}

void TagRelationCreateJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

namespace {
    int toInt(const QByteArray& arr) {
        return arr.mid(4).toInt(); // "tag:" takes 4 char
    }
}

void TagRelationCreateJob::doStart()
{
    if (m_tagRelation->item().id().isEmpty() || m_tagRelation->tag().id().isEmpty()) {
        setError(Error_InvalidRelation);
        setErrorText("Invalid or Empty Ids provided");
        emitResult();
        return;
    }

    int id = toInt(m_tagRelation->tag().id());
    if (id <= 0) {
        setError(Error_InvalidTagId);
        setErrorText("Invalid Tag ID");
        emitResult();
        return;
    }

    QSqlQuery query;
    query.prepare("insert into tagRelations (tid, rid) values (?, ?)");
    query.addBindValue(id);
    query.addBindValue(m_tagRelation->item().id());

    if (!query.exec()) {
        setError(Error_RelationExists);
        setErrorText("The relation already exists");
        emitResult();
        return;
    }

    emit relationCreated(m_tagRelation);
    emit tagRelationCreated(m_tagRelation);
    emitResult();
}
