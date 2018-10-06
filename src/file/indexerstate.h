/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef BALOO_INDEXER_STATE_H
#define BALOO_INDEXER_STATE_H

#include <QString>
#include <KLocalizedString>

namespace Baloo {
enum IndexerState {
        Idle,
        Suspended,
        FirstRun,
        NewFiles,
        ModifiedFiles,
        XAttrFiles,
        ContentIndexing,
        UnindexedFileCheck,
        StaleIndexEntriesClean
};

inline QString stateString(IndexerState state)
{
    QString status = i18n("Unknown");
    switch (state) {
    case Idle:
        status = i18n("Idle");
        break;
    case Suspended:
        status =  i18n("Suspended");
        break;
    case FirstRun:
        status =  i18n("Initial Indexing");
        break;
    case NewFiles:
        status = i18n("Indexing new files");
        break;
    case ModifiedFiles:
        status = i18n("Indexing modified files");
        break;
    case XAttrFiles:
        status = i18n("Indexing Extended Attributes");
        break;
    case ContentIndexing:
        status = i18n("Indexing file content");
        break;
    case UnindexedFileCheck:
        status = i18n("Checking for unindexed files");
        break;
    case StaleIndexEntriesClean:
        status = i18n("Checking for stale index entries");
    }
    return status;
}

//TODO: check for implicit conversion
inline QString stateString(int state)
{
    return stateString(static_cast<IndexerState>(state));
}

}
#endif //BALOO_INDEXER_STATE_H
