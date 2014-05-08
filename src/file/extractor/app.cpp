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
#include "xapiandatabase.h"
#include "../fileindexerconfig.h"

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
    } else {
        m_path = KGlobal::dirs()->localxdgdatadir() + "baloo/file";
    }

    m_db.setPath(m_path);
    if (!m_db.init(true /*sql db only*/)) {
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    m_bData = args->isSet("bdata");
    m_debugEnabled = args->isSet("debug");

    FileIndexerConfig config;

    m_results.reserve(args->count());
    for (int i=0; i<args->count(); i++) {
        FileMapping mapping = FileMapping(args->arg(i).toUInt());
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
            url = args->url(i).toLocalFile();
        }

        if (QFile::exists(url) && config.shouldBeIndexed(url)) {
            m_urls << url;
        } else {
            // id or url was looked up, but file deleted
            kDebug() << url << "does not exist or should not be indexed";

            // Try to delete it as an id:
            // it may have been deleted from the FileMapping db as well.
            // The worst that can happen is deleting nothing.
            mapping.remove(m_db.sqlDatabase());
            m_docsToDelete << mapping.id();
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
    QString mimetype = KMimeType::findByUrl(KUrl::fromLocalFile(url))->name();

    //
    // HACK: We only want to index plain text files which end with a .txt
    // Also, we're ignoring txt files which are greater tha 50 Mb as we
    // have trouble processing them
    //
    if (mimetype == QLatin1String("text/plain")) {
        if (!url.endsWith(".txt")) {
            mimetype.clear();
        }

        QFileInfo fileInfo(url);
        if (fileInfo.size() >= 50 * 1024 * 1024 ) {
            mimetype.clear();
        }
    }

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

    XapianDatabase xapDb(m_path);
    for (int i = 0; i<m_results.size(); i++) {
        Result& res = m_results[i];
        res.finish();

        xapDb.replaceDocument(res.id(), res.document());
        m_updatedFiles << res.inputUrl();
    }

    Q_FOREACH (int docid, m_docsToDelete) {
        xapDb.deleteDocument(docid);
    }

    xapDb.commit();
    m_db.sqlDatabase().commit();

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

    // Print the io usage
    QFile file("/proc/self/io");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream fs(&file);
    QString str = fs.readAll();

    qDebug() << "------- IO ---------";
    QTextStream stream(&str);
    while (!stream.atEnd()) {
        QString str = stream.readLine();

        QString rchar("rchar: ");
        if (str.startsWith(rchar)) {
            ulong amt = str.mid(rchar.size()).toULong();
            qDebug() << "Read:" << amt / 1024  << "kb";
        }

        QString wchar("wchar: ");
        if (str.startsWith(wchar)) {
            ulong amt = str.mid(wchar.size()).toULong();
            qDebug() << "Write:" << amt / 1024  << "kb";
        }

        QString read("read_bytes: ");
        if (str.startsWith(read)) {
            ulong amt = str.mid(read.size()).toULong();
            qDebug() << "Actual Reads:" << amt / 1024  << "kb";
        }

        QString write("write_bytes: ");
        if (str.startsWith(write)) {
            ulong amt = str.mid(write.size()).toULong();
            qDebug() << "Actual Writes:" << amt / 1024  << "kb";
        }
    }

}
