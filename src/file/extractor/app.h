/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXTRACTOR_APP_H
#define EXTRACTOR_APP_H

#include <QMimeDatabase>
#include <QSocketNotifier>
#include <QFile>

#include <KFileMetaData/ExtractorCollection>

#include <memory>
#include <vector>

#include "database.h"
#include "extractor/commandpipe.h"
#include "../fileindexerconfig.h"

class KIdleTime;
class QString;

namespace Baloo {

class Result;

class App : public QObject
{
    Q_OBJECT

public:
    explicit App(QObject* parent = nullptr);
    ~App();

private Q_SLOTS:
    void slotNewBatch(const QVector<quint64>& ids);
    void processNextFile();

private:
    struct BatchInfo;
    bool index(BatchInfo &info);

    QMimeDatabase m_mimeDb;

    KFileMetaData::ExtractorCollection m_extractorCollection;

    FileIndexerConfig m_config;

    QSocketNotifier m_notifyNewData;
    QFile m_input;
    QFile m_output;
    Private::WorkerPipe m_workerPipe;

    KIdleTime* m_idleTime = nullptr;
    bool m_isBusy = true;

    enum class IndexState {
        Pending = 0,
        DoesNotExist,
        RemoveIndex,
        SkipIndex,
        Succeeded,
    };
    struct BatchInfo {
        quint64 m_id = 0;
        QByteArray m_path;
        IndexState m_state = IndexState::Pending;
        std::unique_ptr<Baloo::Result> m_result;
    };
    std::vector<BatchInfo> m_batch;
};

}
#endif
