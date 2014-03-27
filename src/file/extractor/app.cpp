/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include <KCmdLineArgs>
#include <KMimeType>
#include <KStandardDirs>
#include <KDebug>

#include <QTimer>
#include <QtCore/QFileInfo>
#include <QDBusMessage>
#include <QSqlQuery>
#include <QSqlError>
#include <QDBusConnection>
#include <KApplication>
#include <kfilemetadata/propertyinfo.h>

#include <iostream>

using namespace Baloo;

App::App(QObject* parent)
    : QObject(parent)
    , m_termCount(0)
{
    const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    if (!args->getOption("db").isEmpty()) {
        m_path = args->getOption("db");
    }
    else {
        m_path = KGlobal::dirs()->localxdgdatadir() + "baloo/file";
    }

    m_db.setPath(m_path);
    if (!m_db.init()) {
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    connect(m_db.xapianDatabase(), SIGNAL(committed()),
            this, SLOT(slotCommitted()));

    m_bData = args->isSet("bdata");

    m_results.reserve(args->count());
    for (int i=0; i<args->count(); i++) {
        FileMapping mapping = FileMapping(args->arg(i).toUInt());
        QString url;
        if (mapping.fetch(m_db.sqlDatabase())) {
            // arg is an id
            url = mapping.url();
            // If this url no longer exists, remove it from the mapping db.
            if (!QFile::exists(url)) {
                QSqlQuery query(m_db.sqlDatabase());
                query.prepare("delete from files where url = ?");
                query.addBindValue(url);
                if (!query.exec()) {
                    kError() << query.lastError().text();
                }
            }
        } else {
            // arg is a url
            url = args->url(i).toLocalFile();
        }
        if (QFile::exists(url)) {
            m_urls << url;
        } else {
            // id or url was looked up, but file deleted
            kDebug() << url << "does not exist";
            // Try to delete it as an id:
            // it may have been deleted from the FileMapping db as well.
            // The worst that can happen is deleting nothing.
            deleteDocument(mapping.id());
        }
    }

    connect(this, SIGNAL(saved()), this, SLOT(processNextUrl()), Qt::QueuedConnection);

    QTimer::singleShot(0, this, SLOT(processNextUrl()));
}

App::~App()
{
}

void App::processNextUrl()
{
    if (m_urls.isEmpty()) {
        if (m_results.isEmpty()) {
            QCoreApplication::instance()->exit(0);
        }
        else {
            saveChanges();
        }
        return;
    }

    const QString url = m_urls.takeFirst();
    const QString mimetype = KMimeType::findByUrl(KUrl::fromLocalFile(url))->name();

    FileMapping file(url);
    if (!m_bData) {
        if (!file.fetch(m_db.sqlDatabase())) {
            file.create(m_db.sqlDatabase());
        }
    }

    // We always run the basic indexing again. This is mostly so that the proper
    // mimetype is set and we get proper type information.
    // The mimetype fetched in the BasicIQ is fast but not accurate
    BasicIndexingJob basicIndexer(&m_db.sqlDatabase(), file, mimetype);
    basicIndexer.index();

    file.setId(basicIndexer.id());
    Xapian::Document doc = basicIndexer.document();

    Result result(url, mimetype);
    result.setId(file.id());
    result.setDocument(doc);
    result.setReadOnly(m_bData);

    QList<KFileMetaData::ExtractorPlugin*> exList = m_manager.fetchExtractors(mimetype);

    Q_FOREACH (KFileMetaData::ExtractorPlugin* plugin, exList) {
        plugin->extract(&result);
    }
    m_results << result;
    m_termCount += result.document().termlist_count();

    // Documents with these many terms occupy about 10 mb
    if (m_termCount >= 10000 && !m_bData) {
        saveChanges();
        return;
    }

    if (m_urls.isEmpty()) {
        if (m_bData) {
            QByteArray arr;
            QDataStream s(&arr, QIODevice::WriteOnly);

            Q_FOREACH (const Result& res, m_results) {
                QVariantMap map;

                QMapIterator<QString, QVariant> it(res.map());
                while (it.hasNext()) {
                    it.next();
                    int propNum = it.key().toInt();

                    using namespace KFileMetaData::Property;
                    Property prop = static_cast<Property>(propNum);
                    KFileMetaData::PropertyInfo pi(prop);
                    map.insert(pi.name(), it.value());
                }
                s << map;
            }

            std::cout << arr.toBase64().constData();
            m_results.clear();
        }
        else {
            saveChanges();
        }
    }

    QTimer::singleShot(0, this, SLOT(processNextUrl()));
}

void App::saveChanges()
{
    if (m_results.isEmpty())
        return;

    m_updatedFiles.clear();

    for (int i = 0; i<m_results.size(); i++) {
        Result& res = m_results[i];
        res.finish();

        m_db.xapianDatabase()->replaceDocument(res.id(), res.document());
        m_updatedFiles << res.inputUrl();
    }

    m_db.xapianDatabase()->commit();
}

void App::slotCommitted()
{
    m_results.clear();
    m_termCount = 0;
    m_updatedFiles.clear();

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(m_updatedFiles);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    Q_EMIT saved();
}


void App::deleteDocument(unsigned docid)
{
    m_db.xapianDatabase()->deleteDocument(docid);
}

