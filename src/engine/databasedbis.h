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

#ifndef BALOO_DATABASE_DBIS_H
#define BALOO_DATABASE_DBIS_H

#include <lmdb.h>

namespace Baloo {

class DatabaseDbis {
public:
    MDB_dbi postingDbi;
    MDB_dbi positionDBi;

    MDB_dbi docTermsDbi;
    MDB_dbi docFilenameTermsDbi;
    MDB_dbi docXattrTermsDbi;

    MDB_dbi idTreeDbi;
    MDB_dbi idFilenameDbi;

    MDB_dbi docTimeDbi;
    MDB_dbi docDataDbi;
    MDB_dbi contentIndexingDbi;

    MDB_dbi mtimeDbi;

    DatabaseDbis()
        : postingDbi(0)
        , positionDBi(0)
        , docTermsDbi(0)
        , docFilenameTermsDbi(0)
        , docXattrTermsDbi(0)
        , idTreeDbi(0)
        , idFilenameDbi(0)
        , docTimeDbi(0)
        , docDataDbi(0)
        , contentIndexingDbi(0)
        , mtimeDbi(0)
    {}
};

}
#endif
