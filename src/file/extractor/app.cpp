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
#include "../util.h"
#include "xapiandatabase.h"

#include <QDebug>

#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QSocketNotifier>

#include <KFileMetaData/ExtractorPlugin>
#include <KFileMetaData/PropertyInfo>

#include <iostream>

using namespace Baloo;

App::App(QObject *parent)
    : QObject(parent),
      m_sendBinaryData(false),
      m_store(true),
      m_debugEnabled(false),
      m_followConfig(true),
      m_dbPath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/file")),
      m_db(0),
      m_termCount(0),
      m_stdin(stdin, QIODevice::ReadOnly),
      m_stdout(stdout, QIODevice::WriteOnly)
{
    m_stdinNotifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
    connect(m_stdinNotifier, &QSocketNotifier::activated, this, &App::processNextCommand);
}

App::~App()
{
    if (m_db && !(m_results.isEmpty() && m_docsToDelete.isEmpty())) {
        saveChanges();
    }
}

void App::initDb()
{
    if (m_db) {
        return;
    }

    m_db = new Database(this);
    m_db->setPath(m_dbPath);
    if (!m_db->init(true /*sql db only*/)) {
        exit();
        return;
    }
}

void App::setDatabasePath(const QString &path)
{
    if (m_dbPath != path) {
        saveChanges();
        delete m_db;
        m_db = nullptr;
        m_dbPath = path;
    }
}

void App::indexingCompleted(const QString &pathOrId)
{
    m_stdout << 'i' << pathOrId << "\n";
    m_stdout.flush();
}

void App::exit(const QString &error)
{
    if (!error.isEmpty()) {
        qDebug() << error;
    }

    delete m_stdinNotifier; // make sure we get no more input to process!
    m_stdinNotifier = 0;
    deleteLater();
}

bool App::convertToBool(const QString &command, char code)
{
    if (command == "+") {
        return true;
    } else if (command != "-") {
        exit(QString("Unknown command after code %1: %2").arg(code).arg(command));
    }

    return false;
}

void App::processNextCommand()
{
    bool emptyIsFailure = true;
    while (emptyIsFailure || !m_stdin.atEnd()) {
        emptyIsFailure = false;
        QString command;
        m_stdin >> command;

        if (command.isEmpty()) {
            // stdin has closed on us
            exit();
            return;
        }

        char code = command[0].toLatin1();
        command = command.remove(0, 1).trimmed();

        switch (code) {
            case 'b':
                m_sendBinaryData = convertToBool(command, code);
                break;

            case 'c':
                m_followConfig = convertToBool(command, code);
                break;

            case 'd':
                m_store = convertToBool(command, code);
                break;

            case 'i':
                indexFile(command);
                break;

            case 'f':
                saveChanges();
                break;

            case 's':
                setDatabasePath(command);
                break;

            case 'q':
                exit();
                break;

            case 'z':
                m_debugEnabled = convertToBool(command, code);
                break;

            default:
                exit("Unknown command code: " + code);
                break;
        }
    }
}

void App::indexFile(const QString &pathOrId)
{
    if (m_store) {
        initDb();
    }

    m_lastPathOrId = pathOrId;
    QString path;
    bool isId = false;

    if (m_store) {
        bool ok = false;
        uint id = pathOrId.toUInt(&ok);

        if (ok) {
            m_mapping.setUrl(QString());
            m_mapping.setId(id);

            if (m_mapping.fetch(m_db->sqlDatabase())) {
                isId = true;
                path = m_mapping.url();
            }
        }
    }

    if (!isId) {
        m_fileInfo.setFile(pathOrId);
        path = m_fileInfo.absoluteFilePath();
        m_mapping.setUrl(path);
        m_mapping.setId(0);
    }

    if (!QFile::exists(path)) {
        // id or url was looked up, but file deleted
        qDebug() << path << "does not exist";

        if (m_store) {
            m_mapping.remove(m_db->sqlDatabase());
            m_docsToDelete << m_mapping.id();
        }

        indexingCompleted(pathOrId);
        return;
    }

    QString mimetype = m_mimeDb.mimeTypeForFile(path).name();

    if (m_followConfig) {
        bool shouldIndex = m_config.shouldBeIndexed(path) && m_config.shouldMimeTypeBeIndexed(mimetype);
        if (!shouldIndex) {
            qDebug() << path << "should not be indexed. Ignoring";

            if (m_store) {
                m_mapping.remove(m_db->sqlDatabase());
                m_docsToDelete << m_mapping.id();
            }

            indexingCompleted(pathOrId);
            return;
        }
    }

    //
    // HACK: We only want to index plain text files which end with a .txt
    // Also, we're ignoring txt files which are greater tha 50 Mb as we
    // have trouble processing them
    //
    if (mimetype == QLatin1String("text/plain")) {
        if (!path.endsWith(QLatin1String(".txt"))) {
            qDebug() << "text/plain does not end with .txt. Ignoring";
            mimetype.clear();
        } else {
            m_fileInfo.setFile(path);

            if (m_fileInfo.size() >= 50 * 1024 * 1024 ) {
                mimetype.clear();
            }
        }
    }

    if (!m_store) {
        m_mapping.setId(-1);
    } else if (!isId && !m_mapping.fetch(m_db->sqlDatabase())) {
        // if isId, then we already fetched the mapping and know it exists
        m_mapping.create(m_db->sqlDatabase());
    }

    // We always run the basic indexing again. This is mostly so that the proper
    // mimetype is set and we get proper type information.
    // The mimetype fetched in the BasicIQ is fast but not accurate
    BasicIndexingJob basicIndexer(m_mapping, mimetype);
    basicIndexer.index();

    Xapian::Document doc = basicIndexer.document();

    KFileMetaData::ExtractionResult::Flags flags = m_store ? KFileMetaData::ExtractionResult::ExtractEverything
                                                           : KFileMetaData::ExtractionResult::ExtractMetaData;

    Result result(path, mimetype, flags);
    result.setId(m_mapping.id());
    result.setDocument(doc);
    result.setReadOnly(!m_store);

    QList<KFileMetaData::ExtractorPlugin *> exList = m_manager.fetchExtractors(mimetype);

    for (KFileMetaData::ExtractorPlugin *plugin: exList) {
        plugin->extract(&result);
    }

    updateIndexingLevel(m_mapping.url(), Baloo::PendingSave, m_db->sqlDatabase());
    indexingCompleted(pathOrId);

    if (m_sendBinaryData) {
        sendBinaryData(result);
    }

    if (m_store) {
        m_results << result;
        m_termCount += result.document().termlist_count();

        // Documents with these many terms occupy about 10 mb
        if (m_termCount >= 10000) {
            saveChanges();
        }
    }
}

void App::sendBinaryData(const Result &result)
{
    QVariantMap map;

    QMapIterator<QString, QVariant> it(result.map());
    while (it.hasNext()) {
        it.next();
        int propNum = it.key().toInt();

        using namespace KFileMetaData::Property;
        Property prop = static_cast<Property>(propNum);
        KFileMetaData::PropertyInfo pi(prop);
        map.insert(pi.name(), it.value());
    }

    QByteArray arr;
    {
        QDataStream s(&arr, QIODevice::WriteOnly);
        s << map;
    }

    m_stdout << 'b' << arr.size() << arr << "\n";
    m_stdout.flush();
}

void App::saveChanges()
{
    if (!(m_results.isEmpty() && m_docsToDelete.isEmpty())) {
        XapianDatabase xapDb(m_dbPath);
        for (auto &result: m_results) {
            result.finish();
            xapDb.replaceDocument(result.id(), result.document());
        }
        m_results.clear();

        for (auto docId: m_docsToDelete) {
            xapDb.deleteDocument(docId);
        }
        m_docsToDelete.clear();

        xapDb.commit();

        m_termCount = 0;
    }

    if (m_db) {
        // reset all files that were pending save
        QSqlQuery resetQuery(m_db->sqlDatabase());
        resetQuery.prepare("UPDATE files SET indexingLevel = ? WHERE indexingLevel = ?");
        resetQuery.addBindValue(CompletelyIndexed);
        resetQuery.addBindValue(PendingSave);
        resetQuery.exec();
    }

    if (m_debugEnabled) {
        printDebug();
    }

    m_stdout << 's' << m_lastPathOrId << "\n";
    m_stdout.flush();
    m_lastPathOrId.clear();
}

void App::printDebug()
{
    for (const Result& res: m_results) {
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
    QFile file(QLatin1String("/proc/self/io"));
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream fs(&file);
    QString str = fs.readAll();

    qDebug() << "------- IO ---------";
    QTextStream stream(&str);
    while (!stream.atEnd()) {
        QString str = stream.readLine();

        QString rchar(QLatin1String("rchar: "));
        if (str.startsWith(rchar)) {
            ulong amt = str.mid(rchar.size()).toULong();
            qDebug() << "Read:" << amt / 1024  << "kb";
        }

        QString wchar(QLatin1String("wchar: "));
        if (str.startsWith(wchar)) {
            ulong amt = str.mid(wchar.size()).toULong();
            qDebug() << "Write:" << amt / 1024  << "kb";
        }

        QString read(QLatin1String("read_bytes: "));
        if (str.startsWith(read)) {
            ulong amt = str.mid(read.size()).toULong();
            qDebug() << "Actual Reads:" << amt / 1024  << "kb";
        }

        QString write(QLatin1String("write_bytes: "));
        if (str.startsWith(write)) {
            ulong amt = str.mid(write.size()).toULong();
            qDebug() << "Actual Writes:" << amt / 1024  << "kb";
        }
    }

}

