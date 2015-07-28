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

#include "timeestimator.h"

Baloo::TimeEstimator::TimeEstimator()
    : m_filesLeft(0)
{
}

uint Baloo::TimeEstimator::calculateTimeLeft()
{
    //TODO: We should probably make the batch size a global macro
    float totalTime = 0;
    float totalWeight = 0;

    uint currentIndex = m_batchTimings.takeLast();

    uint weightI = 1;
    for (int i = currentIndex; i < m_batchTimings.size(); ++i) {
        float weight = sqrt(weightI);
        totalTime += m_batchTimings.at(i) * weight;
        totalWeight += weight;
        ++weightI;
    }

    for (uint i = 0; i < currentIndex; ++i) {
        float weight = sqrt(weightI);
        totalTime += m_batchTimings.at(i) * weight;
        totalWeight += weight;
        ++weightI;
    }

    float weightedAverage = totalTime / totalWeight;
    float batchesLeft = (float)m_filesLeft / (float)40;

    return weightedAverage * batchesLeft;
}
