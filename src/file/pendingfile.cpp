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
#include <QDebug>

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
    if (file.m_attributesChanged)
        m_attributesChanged = true;
    if (file.m_closedOnWrite)
        m_closedOnWrite = true;
    if (file.m_created)
        m_created = true;
    if (file.m_deleted)
        m_deleted = true;
    if (file.m_modified)
        m_modified = true;
}

void PendingFile::printFlags() const
{
    qDebug() << "AttributesChanged:" << m_attributesChanged;
    qDebug() << "ClosedOnWrite:" << m_closedOnWrite;
    qDebug() << "Created:" << m_created;
    qDebug() << "Deleted:" << m_deleted;
    qDebug() << "Modified:" << m_modified;
}
