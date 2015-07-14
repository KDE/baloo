/*
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

#include "filecontentindexer.h"
#include "filecontentindexerprovider.h"
#include "extractorprocess.h"

#include <QEventLoop>
#include <QElapsedTimer>
#include <QTimer>

using namespace Baloo;

FileContentIndexer::FileContentIndexer(FileContentIndexerProvider* provider)
    : m_provider(provider)
    , m_stop(0)
    , m_delay(0)
    , m_batchTimeBuffer(6, 0)
    , m_bufferIndex(0)
{
    Q_ASSERT(provider);
}

void FileContentIndexer::run()
{
    ExtractorProcess process;
    while (m_provider->size() && !m_stop.load()) {
        //
        // WARNING: This will go mad, if the Extractor does not commit after 40 files
        // cause then we will keep fetching the same 40 again and again.
        //
        QElapsedTimer timer;
        timer.start();

        if (m_delay.load()) {
            QTimer delayTimer;
            QEventLoop loop;

            connect(&delayTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
            delayTimer.start(m_delay.load());
            loop.exec();
        }

        QVector<quint64> idList = m_provider->fetch(40);
        QEventLoop loop;
        connect(&process, &ExtractorProcess::done, &loop, &QEventLoop::quit);

        process.index(idList);
        loop.exec();
        // add the current batch time in place of the oldest batch time
        m_batchTimeBuffer[m_bufferIndex % (m_batchTimeBuffer.size() - 1)] = timer.elapsed();
        ++m_bufferIndex;
    }
    Q_EMIT done();
}

QVector<uint> FileContentIndexer::batchTimings()
{
    if (m_bufferIndex < m_batchTimeBuffer.size() - 1) {
        return QVector<uint>(6, 0);
    }
    // Insert the index of the oldest batch timming as the last entry to let the estimator
    // know which the recentness of each batch.
    m_batchTimeBuffer[m_batchTimeBuffer.size() - 1] = m_bufferIndex % (m_batchTimeBuffer.size() - 1);
    return m_batchTimeBuffer;
}
