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
#include "../basicindexingjob.h"
#include "../tests/util.h"
#include "result.h"
#include "idutils.h"
#include "transaction.h"

#include <QDebug>
#include <QCoreApplication>

#include <QTimer>
#include <QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>

#include <KFileMetaData/Extractor>
#include <KFileMetaData/PropertyInfo>

#include <iostream>

using namespace Baloo;

App::App(const QString& path, QObject* parent)
    : QObject(parent)
    , m_debugEnabled(false)
    , m_ignoreConfig(false)
    , m_db(path)
    , m_termCount(0)
{
    if (!m_db.open()) {
        qCritical() << "Failed to open the database";
        exit(1);
    }
}

void App::startProcessing(const QStringList& args)
{
    m_args = args;
    QTimer::singleShot(0, this, SLOT(process()));
}

void App::process()
{
    // FIXME: The transaction is open for way too long. We should just open it for when we're
    //        committing the data not during the extraction.
    Transaction tr(m_db, Transaction::ReadWrite);

    Q_FOREACH (const QString& arg, m_args) {
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
            continue;
        }

        index(&tr, filePath, id);
    }

    tr.commit();

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(m_updatedFiles);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    m_termCount = 0;
    m_updatedFiles.clear();

    if (m_debugEnabled) {
        printIOUsage();
    }

    QCoreApplication::instance()->exit(0);
}

void App::index(Transaction* tr, const QString& url, quint64 id)
{
    QString mimetype = m_mimeDb.mimeTypeForFile(url).name();

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
            qDebug() << "text/plain does not end with .txt. Ignoring";
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
    tr->replaceDocument(result.document(), Transaction::DocumentTerms | Transaction::DocumentData);

    m_updatedFiles << url;
}

bool App::ignoreConfig() const
{
    return m_ignoreConfig;
}
