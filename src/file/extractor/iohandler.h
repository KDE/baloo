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

#ifndef EXTRACTOR_IOHANDLER_H
#define EXTRACTOR_IOHANDLER_H

#include <QObject>
#include <QTextStream>

namespace Baloo {

class IOHandler
{
public:
    IOHandler(int stdin, int stdout);
    quint64 nextId();
    bool atEnd() const;

    void newBatch();

    void writeStartedIndexingUrl(const QString& url);
    void writeFinishedIndexingUrl(const QString& url);

    // always call this after a batch has been indexed
    void writeBatchIndexed();

private:
    int m_stdinHandle;
    int m_stdoutHandle;

    quint32 m_batchSize;

    QTextStream m_stdout;
};
}

 #endif //EXTRACTOR_IOHANDLER_H
