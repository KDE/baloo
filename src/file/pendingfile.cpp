/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "pendingfile.h"

namespace Baloo
{

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

void PendingFile::merge(const PendingFile& file)
{
    m_attributesChanged |= file.m_attributesChanged;
    m_closedOnWrite |= file.m_closedOnWrite;
    m_created |= file.m_created;
    m_modified |= file.m_modified;
}

QDebug operator<<(QDebug debug, const PendingFile &f)
{
    auto fmt = QStringLiteral(u"%1 [%2 %3 %4 %5 %6]");
    debug << fmt.arg( //
        (f.path()),
        (f.m_attributesChanged ? "ATTR" : "    "),
        (f.m_closedOnWrite ? "CLO" : "   "),
        (f.m_created ? "CRE" : "   "),
        (f.m_deleted ? "DEL" : "   "),
        (f.m_modified ? "MOD" : "   "));

    return debug;
}

} // namespace
