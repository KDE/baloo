/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
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

#include "app.h"
#include "basicindexingjob.h"
#include "tests/file/util.h"
#include "result.h"
#include "idutils.h"
#include "transaction.h"
#include "baloodebug.h"

#include <QCoreApplication>

#include <QTimer>
#include <QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>

#include <KFileMetaData/Extractor>
#include <KFileMetaData/PropertyInfo>

#include <unistd.h> //for STDIN_FILENO

using namespace Baloo;

App::App(const QString& path, QObject* parent)
    : QObject(parent)
    , m_debugEnabled(false)
    , m_ignoreConfig(false)
    , m_db(path)
    , m_termCount(0)
    , m_inputStream(stdin)
    , m_outStream(stdout)
    , m_notifyNewData(STDIN_FILENO, QSocketNotifier::Read)

{
    if (!m_db.open(Database::OpenDatabase)) {
        qCritical() << "Failed to open the database";
        exit(1);
    }
    connect(&m_notifyNewData, &QSocketNotifier::activated, this, &App::slotNewInput);
}

void App::slotNewInput()
{
    // FIXME: The transaction is open for way too long. We should just open it for when we're
    //        committing the data not during the extraction.
    QString arg;
    m_inputStream.skipWhiteSpace();
    arg = m_inputStream.readLine();
    Q_ASSERT(!arg.isEmpty());
    Transaction tr(m_db, Transaction::ReadWrite);

    bool ok = false;
    quint64 id = arg.toULongLong(&ok);

    QString filePath;
    if (ok && !QFile::exists(arg)) {
        filePath = QFile::decodeName(tr.documentUrl(id));
    } else {
        filePath = arg;
        id = filePathToId(QFile::encodeName(filePath));
    }

    if (!QFile::exists(filePath)) {
        tr.removeDocument(id);
        m_outStream << "error:File doesn't exist" << endl;
        return;
    }
    index(&tr, filePath, id);
    tr.commit();
    m_updatedFiles << filePath;
    if (m_updatedFiles.length() > 40) {
        /**
         * FIXME: after the last file has been indexed we can have a few (<40) files
         * remaining in m_updatedFiles which will not be emitted, this will be handled
         * in the next version where we make batch commits as in that version extractor
         * will handle special commands (commit, emit) through stdin.
         */

        QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

        QVariantList vl;
        vl.reserve(1);
        vl << QVariant(m_updatedFiles);
        message.setArguments(vl);

        QDBusConnection::sessionBus().send(message);
        m_updatedFiles.clear();
    }


    m_termCount = 0;

    if (m_debugEnabled) {
        printIOUsage();
    }
    m_outStream << "i:" << id;
    m_outStream.flush();
}

void App::index(Transaction* tr, const QString& url, quint64 id)
{
    QString mimetype = m_mimeDb.mimeTypeForFile(url, QMimeDatabase::MatchContent).name();

    if (!ignoreConfig()) {
        bool shouldIndex = m_config.shouldBeIndexed(url) && m_config.shouldMimeTypeBeIndexed(mimetype);
        if (!shouldIndex) {
            // FIXME: This should never be happening!
            tr->removeDocument(id);
            return;
        }
    }

    //
    // HACK: We only want to index plain text files which end with a .txt
    //
    if (mimetype == QLatin1String("text/plain")) {
        if (!url.endsWith(QLatin1String(".txt"))) {
            qCDebug(BALOO) << "text/plain does not end with .txt. Ignoring";
            tr->removePhaseOne(id);
            return;
        }
    }

    //
    // HACK: Also, we're ignoring ttext files which are greater tha 10 Mb as we
    // have trouble processing them
    //
    if (mimetype.startsWith(QStringLiteral("text/"))) {
        QFileInfo fileInfo(url);
        if (fileInfo.size() >= 10 * 1024 * 1024) {
            tr->removePhaseOne(id);
            return;
        }
    }

    // We always run the basic indexing again. This is mostly so that the proper
    // mimetype is set and we get proper type information.
    // The mimetype fetched in the BasicIQ is fast but not accurate
    BasicIndexingJob basicIndexer(url, mimetype, true /*Indexing Level 2*/);
    basicIndexer.index();

    Baloo::Document doc = basicIndexer.document();

    Result result(url, mimetype, KFileMetaData::ExtractionResult::ExtractEverything);
    result.setDocument(doc);

    QList<KFileMetaData::Extractor*> exList = m_extractorCollection.fetchExtractors(mimetype);

    Q_FOREACH (KFileMetaData::Extractor* ex, exList) {
        ex->extract(&result);
    }

    result.finish();
    if (doc.id() != id) {
        qWarning() << url << "id seems to have changed. Perhaps baloo was not running, and this file was deleted + re-created";
        tr->removeDocument(id);
        if (!tr->hasDocument(doc.id())) {
            tr->addDocument(result.document());
        } else {
            tr->replaceDocument(result.document(), DocumentTerms | DocumentData);
        }
    } else {
        tr->replaceDocument(result.document(), DocumentTerms | DocumentData);
    }
    tr->removePhaseOne(doc.id());
}

bool App::ignoreConfig() const
{
    return m_ignoreConfig;
}
