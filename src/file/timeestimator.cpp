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

TimeEstimator::TimeEstimator(QObject* parent)
    : QObject(parent)
    , m_bufferIndex(0)
    , m_estimateReady(false)
{

}

uint TimeEstimator::calculateTimeLeft(int filesLeft)
{
    if (!m_estimateReady) {
        return 0;
    }

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

    return weightedAverage * filesLeft;
}

void TimeEstimator::handleNewBatchTime(uint time, uint batchSize)
{
    // add the current batch time in place of the oldest batch time
    m_batchTimeBuffer[m_bufferIndex] = (float)time / batchSize;

    m_bufferIndex = (m_bufferIndex + 1) % BUFFER_SIZE;

    if (!m_estimateReady && m_bufferIndex == 0) {
        // Buffer has been filled once. We are ready to estimate
        m_estimateReady = true;
    }
}
