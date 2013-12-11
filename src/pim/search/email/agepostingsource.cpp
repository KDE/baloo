/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    kDebug() << "Created with current time" << m_currentTime_t;
}

Xapian::weight AgePostingSource::get_weight() const
{
    QString str = QString::fromStdString(*value_it);

    bool ok = false;
    uint time = str.toUInt(&ok);
    uint diff = m_currentTime_t - time;

    if (!ok)
        return 0.0;

    // Don't ask - Just tried random things. We need to improve the weights.
    return log10(diff);
}

Xapian::PostingSource* AgePostingSource::clone() const
{
    return new AgePostingSource(slot);
}

void AgePostingSource::init(const Xapian::Database& db_)
{
    Xapian::ValuePostingSource::init(db_);
    //set_maxweight(1.0);
}
