/*
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "filecontentindexerprovider.h"

#include "transaction.h"
#include "database.h"

using namespace Baloo;

FileContentIndexerProvider::FileContentIndexerProvider(Database* db)
    : m_db(db)
{
}

QVector<quint64> FileContentIndexerProvider::fetch(uint size)
{
    Transaction tr(m_db, Transaction::ReadOnly);
    return tr.fetchPhaseOneIds(size);
}

uint FileContentIndexerProvider::size()
{
    Transaction tr(m_db, Transaction::ReadOnly);
    return tr.phaseOneSize();
}

void FileContentIndexerProvider::markFailed(quint64 id)
{
    Transaction tr(m_db, Transaction::ReadWrite);
    if (!tr.hasFailed(id)) {
        tr.addFailed(id);
    }
    tr.removePhaseOne(id);
    tr.commit();
}
