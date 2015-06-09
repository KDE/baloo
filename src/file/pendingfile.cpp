/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "pendingfile.h"
#include "baloodebug.h"

using namespace Baloo;

PendingFile::PendingFile(const QString& path)
    : m_path(path)
    , m_created(false)
    , m_closedOnWrite(false)
    , m_attributesChanged(false)
    , m_deleted(false)
    , m_modified(false)
{
}

QString PendingFile::path() const
{
    return m_path;
}

void PendingFile::setPath(const QString& path)
{
    m_path = path;
    if (m_path.endsWith('/')) {
        m_path = m_path.mid(0, m_path.length() - 1);
    }
}

bool PendingFile::isNewFile() const
{
    return m_created;
}

bool PendingFile::shouldIndexContents() const
{
    if (m_created || m_closedOnWrite || m_modified) {
        return true;
    }
    return false;
}

bool PendingFile::shouldIndexXAttrOnly() const
{
    if (m_attributesChanged && !shouldIndexContents()) {
        return true;
    }
    return false;
}

bool PendingFile::shouldRemoveIndex() const
{
    return m_deleted;
}

void PendingFile::merge(const PendingFile& file)
{
    m_attributesChanged |= file.m_attributesChanged;
    m_closedOnWrite |= file.m_closedOnWrite;
    m_created |= file.m_created;
    m_deleted |= file.m_deleted;
    m_modified |= file.m_modified;
}

void PendingFile::printFlags() const
{
    qCDebug(BALOO) << "AttributesChanged:" << m_attributesChanged;
    qCDebug(BALOO) << "ClosedOnWrite:" << m_closedOnWrite;
    qCDebug(BALOO) << "Created:" << m_created;
    qCDebug(BALOO) << "Deleted:" << m_deleted;
    qCDebug(BALOO) << "Modified:" << m_modified;
}
