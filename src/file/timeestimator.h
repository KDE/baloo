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

#ifndef BALOO_TIMEESTIMATOR_H
#define BALOO_TIMEESTIMATOR_H

#define BUFFER_SIZE 5

#include "fileindexerconfig.h"
#include <QObject>

namespace Baloo {
/*
* This class handles the time estimation logic for filecontentindexer.
* Time estimations use a weighted moving average of the time taken by
* 5 most recent batches. The more recent the batch is, higher the weight
* it will be assigned.
*/

class TimeEstimator : public QObject
{
public:
    explicit TimeEstimator(FileIndexerConfig* config, QObject* parent = 0);
    uint calculateTimeLeft(int filesLeft);

public Q_SLOTS:
    void handleNewBatchTime(uint time);

private:
    uint m_batchTimeBuffer[BUFFER_SIZE];

    int m_bufferIndex;
    bool m_estimateReady;

    FileIndexerConfig* m_config;
    uint m_batchSize;
};

}

#endif //BALOO_TIMEESTIMATOR_H
