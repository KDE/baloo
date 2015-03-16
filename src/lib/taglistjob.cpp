/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014-2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "taglistjob.h"
#include "db.h"
#include "database.h"

#include <QStringList>

using namespace Baloo;

class TagListJob::Private {
public:
    QStringList tags;
};

TagListJob::TagListJob(QObject* parent)
    : KJob(parent)
    , d(new Private)
{
}

TagListJob::~TagListJob()
{
    delete d;
}

void TagListJob::start()
{
    Database db(fileIndexDbPath());
    db.open();
    db.transaction(Database::ReadOnly);

    QList<QByteArray> tagList = db.fetchTermsStartingWith("TAG-");
    d->tags.reserve(tagList.size());
    for (const QByteArray& ba : tagList) {
        d->tags << QString::fromUtf8(ba.mid(4));
    }

    emitResult();
}

QStringList TagListJob::tags()
{
    return d->tags;
}
