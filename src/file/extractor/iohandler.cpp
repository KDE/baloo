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

#include "iohandler.h"
#include <unistd.h>
#include <iostream>
#include <QDebug>

using namespace Baloo;

IOHandler::IOHandler(int stdIn, int stdOut)
    : m_stdinHandle(stdIn)
    , m_stdoutHandle(stdOut)
    , m_batchSize(0)
    , m_stdout(stdout)
{
}

void IOHandler::newBatch()
{
    read(m_stdinHandle, &m_batchSize, sizeof(quint32));
    Q_ASSERT(m_batchSize != 0);
}

quint64 IOHandler::nextId()
{
    Q_ASSERT(!atEnd());
    quint64 id = 0;
    read(m_stdinHandle, &id, sizeof(quint64));
    --m_batchSize;
    Q_ASSERT(id != 0);
    return id;
}

bool IOHandler::atEnd()
{
    if (m_batchSize == 0) {
        return true;
    }
    return false;
}

void IOHandler::indexedId(quint64 id)
{
    write(m_stdoutHandle, reinterpret_cast<void*>(&id), sizeof(quint64));
}

void IOHandler::batchIndexed()
{
    m_batchSize = 0;
    m_stdout << "batch commited" << endl;
}

void IOHandler::indexingUrl(QString url)
{
    m_stdout << url << endl;
}
