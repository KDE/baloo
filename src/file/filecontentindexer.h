/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILECONTENTINDEXER_H
#define BALOO_FILECONTENTINDEXER_H

#include <QRunnable>
#include <QObject>
#include <QAtomicInt>
#include <QStringList>

#include <QDBusServiceWatcher>
#include <QDBusMessage>
#include "fileindexerconfig.h"

namespace Baloo {

class FileContentIndexerProvider;

class FileContentIndexer : public QObject, public QRunnable
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.fileindexer")

    Q_PROPERTY(QString currentFile READ currentFile NOTIFY startedIndexingFile)
public:
    FileContentIndexer(FileIndexerConfig* config, FileContentIndexerProvider* provider, uint& finishedCount, QObject* parent = nullptr);

    QString currentFile() { return m_currentFile; }

    void run() override;

    void quit() {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        m_stop.store(true);
#else
        m_stop.storeRelaxed(true);
#endif
    }

public Q_SLOTS:
    Q_SCRIPTABLE void registerMonitor(const QDBusMessage& message);
    Q_SCRIPTABLE void unregisterMonitor(const QDBusMessage& message);

Q_SIGNALS:
    Q_SCRIPTABLE void startedIndexingFile(const QString& filePath);
    Q_SCRIPTABLE void finishedIndexingFile(const QString& filePath);
    Q_SCRIPTABLE void committedBatch(uint time, uint batchSize);

    void done();

private Q_SLOTS:
    void monitorClosed(const QString& service);
    void slotStartedIndexingFile(const QString& filePath);
    void slotFinishedIndexingFile(const QString& filePath);

private:
    FileIndexerConfig *m_config;
    uint m_batchSize;
    FileContentIndexerProvider* m_provider;
    uint& m_finishedCount;

    QAtomicInt m_stop;

    QString m_currentFile;

    QStringList m_registeredMonitors;
    QDBusServiceWatcher m_monitorWatcher;
};

}

#endif // BALOO_FILECONTENTINDEXER_H
