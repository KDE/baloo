/*
   This file is part of the KDE Baloo project.
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

#include "idfilenamedb.h"
#include "singledbtest.h"

using namespace Baloo;

class BALOO_ENGINE_EXPORT IdFilenameDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        IdFilenameDB db(IdFilenameDB::create(m_txn), m_txn);

        IdFilenameDB::FilePath path;
        path.parentId = 5;
        path.name = "fire";

        db.put(1, path);

        QCOMPARE(db.get(1), path);

        db.del(1);
        QCOMPARE(db.get(1), IdFilenameDB::FilePath());
    }
};

QTEST_MAIN(IdFilenameDBTest)

#include "idfilenamedbtest.moc"
