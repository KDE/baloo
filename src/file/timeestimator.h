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

#include <QtGlobal>
#include <QVector>

namespace Baloo {

/*
* This class handles the time estimation logic
* for filecontentindexer.
*/
class TimeEstimator
{
public:
    TimeEstimator();
    void setFilesLeft(uint left) { m_filesLeft = left; }
    void setBatchTimings(const QVector<uint>& batchTimings) { m_batchTimings = batchTimings; }

    // gives the estimated time left
    uint calculateTimeLeft();

private:
    uint m_filesLeft;
    QVector<uint> m_batchTimings;
};

}

#endif //BALOO_TIMEESTIMATOR_H
