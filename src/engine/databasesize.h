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

#ifndef BALOO_DATABASE_SIZE_H
#define BALOO_DATABASE_SIZE_H

#include <lmdb.h>

namespace Baloo {

class DatabaseSize {
public:
    /**
     * This is the size which is computed with all the pages used from all the
     * individual database pages
     */
    uint expectedSize;

    /**
     * This is the size based on the MDB_env and the total number of pages used
     */
    uint actualSize;

    uint postingDb;
    uint positionDb;

    uint docTerms;
    uint docFilenameTerms;
    uint docXattrTerms;

    uint idTree;
    uint idFilename;

    uint docTime;
    uint docData;

    uint contentIndexingIds;
    uint failedIds;

    uint mtimeDb;
};

}
#endif
