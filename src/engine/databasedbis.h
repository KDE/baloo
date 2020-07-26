/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
    MDB_dbi failedIdDbi;

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
        , failedIdDbi(0)
    {}

    bool isValid() {
        return postingDbi && positionDBi && docTermsDbi && docFilenameTermsDbi && docXattrTermsDbi &&
               idTreeDbi && idFilenameDbi && docTimeDbi && docDataDbi && contentIndexingDbi && mtimeDbi
               && failedIdDbi;
    }
};

}
#endif
