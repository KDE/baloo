/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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
#include <KDebug>

#include <xapian.h>
#include <QTimer>
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
    Xapian::Database xapianDb(fileIndexDbPath().toStdString());
    Xapian::TermIterator it = xapianDb.allterms_begin("TAG");
    Xapian::TermIterator end = xapianDb.allterms_end("TAG");

    for (; it != end; it++ ) {
        QString tag = QString::fromStdString(*it);
        if (tag.startsWith("TAG")) {
            d->tags << tag.mid(3);
        }
    }

    emitResult();
}

QStringList TagListJob::tags()
{
    return d->tags;
}
