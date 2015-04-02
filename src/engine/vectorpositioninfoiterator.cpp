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

#include "vectorpositioninfoiterator.h"
#include "positioninfo.h"

using namespace Baloo;

VectorPositionInfoIterator::VectorPositionInfoIterator(const QVector<PositionInfo>& vector)
    : m_vector(vector)
    , m_pos(-1)
{
}

quint64 VectorPositionInfoIterator::next()
{
    m_pos++;
    if (m_pos >= m_vector.size()) {
        m_vector.clear();
        return 0;
    }

    return m_vector[m_pos].docId;
}

quint64 VectorPositionInfoIterator::docId()
{
    if (m_pos < 0 || m_pos >= m_vector.size()) {
        return 0;
    }

    return m_vector[m_pos].docId;
}

QVector<uint> VectorPositionInfoIterator::positions()
{
    if (m_pos < 0 || m_pos >= m_vector.size()) {
        return QVector<uint>();
    }

    return m_vector[m_pos].positions;
}



