/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "app.h"
#include "basicindexingjob.h"
#include "result.h"
#include "idutils.h"
#include "transaction.h"
#include "baloodebug.h"
#include "global.h"

#include <QCoreApplication>

#include <QTimer>
#include <QFileInfo>

#include <KFileMetaData/Extractor>
#include <KFileMetaData/MimeUtils>
#include <KIdleTime>

#include <unistd.h> //for STDIN_FILENO
#include <iostream>

using namespace Baloo;

App::App(QObject *parent)
    : QObject(parent)
    , m_notifyNewData(STDIN_FILENO, QSocketNotifier::Read)
    , m_input()
    , m_output()
    , m_workerPipe(&m_input, &m_output)
{
    m_input.open(STDIN_FILENO, QIODevice::ReadOnly | QIODevice::Unbuffered );
    m_output.open(STDOUT_FILENO, QIODevice::WriteOnly | QIODevice::Unbuffered );

    static int s_idleTimeout = 1000 * 60 * 1; // 1 min
    m_idleTime = KIdleTime::instance();
    m_idleTime->addIdleTimeout(s_idleTimeout);
    connect(m_idleTime, &KIdleTime::resumingFromIdle, this, [this]() {
        qCInfo(BALOO) << "Busy, paced indexing";
        m_isBusy = true;
    });
    connect(m_idleTime, &KIdleTime::timeoutReached, this, [this]() {
        qCInfo(BALOO) << "Not busy, fast indexing";
        m_isBusy = false;
    });

    using WorkerPipe = Baloo::Private::WorkerPipe;
    connect(&m_notifyNewData, &QSocketNotifier::activated, &m_workerPipe, &WorkerPipe::processIdData);
    connect(&m_workerPipe, &WorkerPipe::newDocumentIds, this, &App::slotNewBatch);
    connect(&m_workerPipe, &WorkerPipe::inputEnd, this, &QCoreApplication::quit);
}

App::~App() = default;

void App::slotNewBatch(const QVector<quint64>& ids)
{
    // The DB is a per process static instance. Any open call but the first is a no-op
    Database *db = globalDatabaseInstance();
    if (db->open(Database::ReadWriteDatabase) != Database::OpenResult::Success) {
        qCCritical(BALOO) << "Failed to open the database";
        exit(1);
    }

    if (!m_isBusy) {
        m_idleTime->catchNextResumeEvent();
    }

    {
        Transaction tr(db, Transaction::ReadOnly);
        for (const auto id : ids) {
            auto url = tr.documentUrl(id);
            m_batch.push_back({id, url, IndexState::Pending, {}});
        }
    }

    m_batchTime.start();

    QTimer::singleShot((m_isBusy ? 500 : 0), this, [this, db]() {
        processNextFile();
    });
}

void App::processNextFile()
{
    auto next = std::find_if(m_batch.begin(), m_batch.end(), [](const auto &info) {
        return info.m_state == IndexState::Pending;
    });

    if (next != m_batch.end()) {
        bool indexed = index(*next);
        Q_ASSERT(next->m_state != IndexState::Pending);

        // Try to commit at least every 2 seconds
        if (m_batchTime.durationElapsed() < std::chrono::seconds{2}) {
            int delay = (m_isBusy && indexed) ? 10 : 0;
            QTimer::singleShot(delay, this, &App::processNextFile);
            return;
        }
    }

    // End of batch, or batch time exceeded, commit
    {
        Database *db = globalDatabaseInstance();
        Transaction tr(db, Transaction::ReadWrite);

        size_t pendingCount = 0;

        for (auto &info : m_batch) {
            auto result = info.m_result.release();
            switch (info.m_state) {
            case IndexState::DoesNotExist:
            case IndexState::RemoveIndex:
                info.m_state = IndexState::Committed;
                tr.removeDocument(info.m_id);
                break;
            case IndexState::SkipIndex:
                info.m_state = IndexState::Committed;
                tr.removePhaseOne(info.m_id);
                break;
            case IndexState::Succeeded:
                info.m_state = IndexState::Committed;
                if (!tr.inPhaseOne(info.m_id)) {
                    // Document was replaced by a document which should
                    // not be indexed, typically by overwriting it.
                    continue;
                }
                if (!tr.hasDocument(info.m_id)) {
                    // Document was deleted after indexing had finished
                    continue;
                }
                if (tr.documentUrl(info.m_id) != info.m_path) {
                    // Document was either renamed, or documentId was
                    // reused
                    continue;
                }
                tr.replaceDocument(result->document(), DocumentTerms | DocumentData);
                tr.removePhaseOne(info.m_id);
                tr.removeFailed(info.m_id);
                break;
            case IndexState::Pending:
                pendingCount++;
                break;
            case IndexState::Committed:
                break;
            }
        }

        if (bool ok = tr.commit(); !ok) {
            exit(2);
        }

        if (pendingCount > 0) {
            m_batchTime.restart();

            int delay = m_isBusy ? 500 : 0;
            QTimer::singleShot(delay, this, &App::processNextFile);

        } else {
            m_batch.clear();
            m_workerPipe.batchFinished();
        }
    }
}

bool App::index(BatchInfo &info)
{
    QString url = QFile::decodeName(info.m_path);

    if (url.isEmpty()) {
        info.m_state = IndexState::DoesNotExist;
        return false;
    }

    if (!QFile::exists(url)) {
        info.m_state = IndexState::DoesNotExist;
        m_workerPipe.urlFailed(url);
        return false;
    }

    if (!m_config.shouldBeIndexed(url)) {
        // This apparently happens when the config has changed after the document
        // was added to the content indexing db
        qCDebug(BALOO) << "Found" << url << "in the ContentIndexingDB, although it should be skipped";
        info.m_state = IndexState::RemoveIndex;
        m_workerPipe.urlFailed(url);
        return false;
    }

    // The initial BasicIndexingJob run has been supplied with the file extension
    // mimetype only, skip based on the "real" mimetype
    QString mimetype = KFileMetaData::MimeUtils::strictMimeType(url, m_mimeDb).name();
    if (!m_config.shouldMimeTypeBeIndexed(mimetype)) {
        qCDebug(BALOO) << "Skipping" << url << "- mimetype:" << mimetype;
        // FIXME: in case the extension based and content based mimetype differ
        // we should update it.
        info.m_state = IndexState::SkipIndex;
        m_workerPipe.urlFailed(url);
        return false;
    }

    // HACK: Also, we're ignoring text files which are greater than 10 MB as we
    // have trouble processing them
    // Also include .mbox files as these can be large and application/mbox is
    // a subclass of text/plain
    if (mimetype.startsWith(QLatin1String("text/")) || mimetype == QLatin1String("application/mbox")) {
        QFileInfo fileInfo(url);
        if (fileInfo.size() >= 10 * 1024 * 1024) {
            qCDebug(BALOO) << "Skipping large" << url << "- mimetype:" << mimetype << fileInfo.size() << "bytes";
            info.m_state = IndexState::SkipIndex;
            m_workerPipe.urlFailed(url);
            return false;
        }
    }
    qCDebug(BALOO) << "Indexing" << info.m_id << url << mimetype;
    m_workerPipe.urlStarted(url);

    // We always run the basic indexing again. This is mostly so that the proper
    // mimetype is set and we get proper type information.
    // The mimetype fetched in the BasicIndexingJob is fast but not accurate
    BasicIndexingJob basicIndexer(url, mimetype, BasicIndexingJob::NoLevel);
    if (!basicIndexer.index()) {
        qCDebug(BALOO) << "Skipping non-existing file " << url;
        info.m_state = IndexState::DoesNotExist;
        m_workerPipe.urlFailed(url);
        return false;
    }

    Baloo::Document doc = basicIndexer.document();
    if (doc.id() != info.m_id) {
        qCWarning(BALOO) << url << "id seems to have changed. Perhaps baloo was not running, and this file was deleted + re-created";
        info.m_state = IndexState::RemoveIndex;
        m_workerPipe.urlFailed(url);
        return false;
    }

    info.m_result =
        std::make_unique<Result>(url, mimetype, KFileMetaData::ExtractionResult::ExtractMetaData | KFileMetaData::ExtractionResult::ExtractPlainText);
    info.m_result->setDocument(doc);

    const QList<KFileMetaData::Extractor*> exList = m_extractorCollection.fetchExtractors(mimetype);

    for (KFileMetaData::Extractor* ex : exList) {
        ex->extract(info.m_result.get());
    }

    info.m_result->finish();
    info.m_state = IndexState::Succeeded;
    m_workerPipe.urlFinished(url);
    return true;
}

#include "moc_app.cpp"
