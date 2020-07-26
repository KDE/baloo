/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
    Q_OBJECT
public:
    explicit TimeEstimator(QObject* parent = nullptr);
    uint calculateTimeLeft(int filesLeft);

public Q_SLOTS:
    void handleNewBatchTime(uint time, uint batchSize);

private:
    float m_batchTimeBuffer[BUFFER_SIZE];

    int m_bufferIndex;
    bool m_estimateReady;
};

}

#endif //BALOO_TIMEESTIMATOR_H
