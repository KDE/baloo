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

#include <cmath>

#include "timeestimator.h"
#include "filecontentindexerprovider.h"

using namespace Baloo;

TimeEstimator::TimeEstimator(FileIndexerConfig *config, QObject* parent)
    : QObject(parent)
    , m_bufferIndex(0)
    , m_estimateReady(false)
    , m_config(config)
    , m_batchSize(m_config->maxUncomittedFiles())
{

}

uint TimeEstimator::calculateTimeLeft(int filesLeft)
{
    if (!m_estimateReady) {
        return 0;
    }

    //TODO: We should probably make the batch size a global macro
    float totalTime = 0;
    float totalWeight = 0;

    int bufferIndex = m_bufferIndex;
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        float weight = sqrt(i + 1);
        totalWeight += weight;

        totalTime += m_batchTimeBuffer[bufferIndex] * weight;
        bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
    }

    float weightedAverage = totalTime / totalWeight;
    float batchesLeft = (float)filesLeft / (float)m_batchSize;

    return weightedAverage * batchesLeft;
}

void TimeEstimator::handleNewBatchTime(uint time)
{
    // add the current batch time in place of the oldest batch time
    m_batchTimeBuffer[m_bufferIndex] = time;

    if (!m_estimateReady && m_bufferIndex == BUFFER_SIZE - 1) {
        // Buffer has been filled once. We are ready to estimate
        m_estimateReady = true;
    }

    m_bufferIndex = (m_bufferIndex + 1) % BUFFER_SIZE;
}
