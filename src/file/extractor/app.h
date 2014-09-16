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

#ifndef EXTRACTOR_APP_H
#define EXTRACTOR_APP_H

#include <QVector>
#include <QFileInfo>
#include <QStringList>
#include <QMimeDatabase>
#include <QTextStream>

#include <KFileMetaData/ExtractorPluginManager>

#include "result.h"
#include "../database.h"
#include "../fileindexerconfig.h"
#include "filemapping.h"

namespace Baloo {

class App : public QObject
{
    Q_OBJECT
public:
    explicit App(QObject *parent = 0);
    ~App();

    void setDebug(bool status);
    void setBData(bool status);
    void setFollowConfig(bool follow);
    bool followConfig() const;
    void indexFile(const QString &urlOrId);

private:
    void setDatabasePath(const QString &path);
    void initDb();
    void saveChanges();
    void processNextCommand();
    void sendBinaryData(const Result &result);
    void indexingCompleted(const QString &pathOrId);
    void printDebug();

    QVector<Result> m_results;
    QStringList m_urls;
    bool m_bData;
    bool m_store;
    bool m_debugEnabled;
    bool m_followConfig;

    QString m_lastPathOrId;
    QString m_dbPath;

    Database *m_db;
    QMimeDatabase m_mimeDb;

    KFileMetaData::ExtractorPluginManager m_manager;

    int m_termCount;
    QVector<uint> m_docsToDelete;

    FileIndexerConfig m_config;

    // short-use variables that we keep around for re-use
    // avoids reallocating more than absolutely necessary
    // all use of these variables must be kept within a single method
    // once the method is left (including calling another method in the class
    // the variables may be set to new values. beware!
    QFileInfo m_fileInfo;
    FileMapping m_mapping;

    QTextStream m_stdout;
};

}
#endif
