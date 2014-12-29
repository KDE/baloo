/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2014  Vishesh Handa <me@vhanda.in>
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
#include "../database.h"
#include "xapiandatabase.h"
#include "../tests/util.h"

#include <QDebug>
#include <QCoreApplication>

#include <QTimer>
#include <QFileInfo>
#include <QDBusMessage>
#include <QSqlQuery>
#include <QSqlError>
#include <QDBusConnection>

#include <KFileMetaData/Extractor>
#include <KFileMetaData/PropertyInfo>

#include <iostream>

using namespace Baloo;

App::App(const QString& path, QObject* parent)
    : QObject(parent)
    , m_debugEnabled(false)
    , m_ignoreConfig(false)
    , m_path(path)
    , m_termCount(0)
{
    m_db.setPath(m_path);
    if (!m_db.init(true /*sql db only*/)) {
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }
    m_db.sqlDatabase().transaction();

    connect(this, &App::saved, this, &App::processNextUrl, Qt::QueuedConnection);
}

void App::startProcessing(const QStringList& args)
{
    m_results.reserve(args.size());
    Q_FOREACH (const QString& arg, args) {
        FileMapping mapping = FileMapping(arg.toUInt());
        QString url;

        // arg is an id
        if (mapping.fetch(m_db.sqlDatabase())) {
            url = mapping.url();
            if (!QFile::exists(url)) {
                mapping.remove(m_db.sqlDatabase());
                continue;
            }
        } else {
            // arg is a url
            url = QFileInfo(arg).absoluteFilePath();
        }

        if (QFile::exists(url)) {
            m_urls << url;
        } else {
            // id or url was looked up, but file deleted
            qDebug() << url << "does not exist";

            // Try to delete it as an id:
            // it may have been deleted from the FileMapping db as well.
            // The worst that can happen is deleting nothing.
            mapping.remove(m_db.sqlDatabase());
            m_docsToDelete << mapping.id();
        }
    }

    QTimer::singleShot(0, this, SLOT(processNextUrl()));
}

void App::processNextUrl()
{
    if (m_urls.isEmpty()) {
        if (m_results.isEmpty() && m_docsToDelete.isEmpty()) {
            QCoreApplication::instance()->exit(0);
        }
        else {
            saveChanges();
        }
        return;
    }

    const QString url = m_urls.takeFirst();
    QString mimetype = m_mimeDb.mimeTypeForFile(url).name();

    if (!ignoreConfig()) {
        bool shouldIndex = m_config.shouldBeIndexed(url) && m_config.shouldMimeTypeBeIndexed(mimetype);
        if (!shouldIndex) {
            qDebug() << url << "should not be indexed. Ignoring";

            FileMapping mapping(url);
            mapping.remove(m_db.sqlDatabase());
            m_docsToDelete << mapping.id();

            QTimer::singleShot(0, this, SLOT(processNextUrl()));
            return;
        }
    }

    //
    // HACK: We only want to index plain text files which end with a .txt
    // Also, we're ignoring txt files which are greater tha 50 Mb as we
    // have trouble processing them
    //
    if (mimetype == QLatin1String("text/plain")) {
        if (!url.endsWith(QLatin1String(".txt"))) {
            qDebug() << "text/plain does not end with .txt. Ignoring";
            mimetype.clear();
        }

        QFileInfo fileInfo(url);
        if (fileInfo.size() >= 50 * 1024 * 1024 ) {
            mimetype.clear();
        }
    }

    FileMapping file(url);
    if (!file.fetch(m_db.sqlDatabase())) {
        file.create(m_db.sqlDatabase());
    }

    // We always run the basic indexing again. This is mostly so that the proper
    // mimetype is set and we get proper type information.
    // The mimetype fetched in the BasicIQ is fast but not accurate
    BasicIndexingJob basicIndexer(file, mimetype, true /*Indexing Level 2*/);
    basicIndexer.index();

    file.setId(basicIndexer.id());
    Xapian::Document doc = basicIndexer.document();

    Result result(url, mimetype, KFileMetaData::ExtractionResult::ExtractEverything);
    result.setId(file.id());
    result.setDocument(doc);

    QList<KFileMetaData::Extractor*> exList = m_extractorCollection.fetchExtractors(mimetype);

    Q_FOREACH (KFileMetaData::Extractor* ex, exList) {
        ex->extract(&result);
    }
    m_results << result;
    m_termCount += result.document().termlist_count();

    // Documents with these many terms occupy about 10 mb
    if (m_termCount >= 10000) {
        saveChanges();
        return;
    }

    if (m_urls.isEmpty()) {
        saveChanges();
    }

    QTimer::singleShot(0, this, SLOT(processNextUrl()));
}

void App::saveChanges()
{
    if (m_results.isEmpty() && m_docsToDelete.isEmpty())
        return;

    m_updatedFiles.clear();

    XapianDatabase xapDb(m_path);
    for (int i = 0; i<m_results.size(); ++i) {
        Result& res = m_results[i];
        res.finish();

        xapDb.replaceDocument(res.id(), res.document());
        m_updatedFiles << res.inputUrl();
    }

    Q_FOREACH (int docid, m_docsToDelete) {
        xapDb.deleteDocument(docid);
    }
    m_docsToDelete.clear();

    xapDb.commit();
    m_db.sqlDatabase().commit();

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(m_updatedFiles);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    m_results.clear();
    m_termCount = 0;
    m_updatedFiles.clear();

    if (m_debugEnabled) {
        printDebug();
    }

    Q_EMIT saved();
}

void App::printDebug()
{
    Q_FOREACH (const Result& res, m_results) {
        qDebug() << res.inputUrl();
        QMapIterator<QString, QVariant> it(res.map());
        while (it.hasNext()) {
            it.next();
            int propNum = it.key().toInt();

            using namespace KFileMetaData::Property;
            Property prop = static_cast<Property>(propNum);
            KFileMetaData::PropertyInfo pi(prop);
            qDebug() << pi.name() << it.value();
        }
    }

    printIOUsage();
}

bool App::ignoreConfig() const
{
    return m_ignoreConfig;
}
