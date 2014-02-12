/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "agepostingsource.h"

#include <QString>
#include <QDateTime>
#include <KDebug>

#include <cmath>

using namespace Baloo;

AgePostingSource::AgePostingSource(Xapian::valueno slot_)
    : Xapian::ValuePostingSource(slot_)
{
    m_currentTime_t = QDateTime::currentDateTime().toTime_t();
}

Xapian::weight AgePostingSource::get_weight() const
{
    QString str = QString::fromStdString(*value_it);

    bool ok = false;
    uint time = str.toUInt(&ok);

    if (!ok)
        return 0.0;

    QDateTime dt = QDateTime::fromTime_t(time);
    uint diff = m_currentTime_t - time;

    // Each day is given a penalty of penalty of 1.0
    double penalty = 1.0 / (24*60*60);
    double result = 1000.0 - (diff*penalty);

    if (result < 0.0)
        return 0.0;

    return result;
}

Xapian::PostingSource* AgePostingSource::clone() const
{
    return new AgePostingSource(slot);
}

void AgePostingSource::init(const Xapian::Database& db_)
{
    Xapian::ValuePostingSource::init(db_);
    set_maxweight(1000.0);
}
