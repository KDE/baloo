/*
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BALOO_FILECONTENTINDEXER_H
#define BALOO_FILECONTENTINDEXER_H

#include <QRunnable>
#include <QObject>
#include <QAtomicInt>
#include <QStringList>

#include <QDBusServiceWatcher>
#include <QDBusMessage>

namespace Baloo {

class FileContentIndexerProvider;

class FileContentIndexer : public QObject, public QRunnable
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.fileindexer")

    Q_PROPERTY(QString currentFile READ currentFile NOTIFY startedIndexingFile)
public:
    FileContentIndexer(FileContentIndexerProvider* provider, QObject* parent = 0);

    QString currentFile() { return m_currentFile; }

    void run() Q_DECL_OVERRIDE;

    void quit() {
        m_stop.store(true);
    }

public Q_SLOTS:
    Q_SCRIPTABLE void registerMonitor(const QDBusMessage& message);
    Q_SCRIPTABLE void unregisterMonitor(const QDBusMessage& message);

Q_SIGNALS:
    Q_SCRIPTABLE void startedIndexingFile(const QString& filePath);
    Q_SCRIPTABLE void finishedIndexingFile(const QString& filePath);

    void done();
    void newBatchTime(uint time);

private Q_SLOTS:
    void monitorClosed(const QString& service);
    void slotStartedIndexingFile(const QString& filePath);
    void slotFinishedIndexingFile(const QString& filePath);

private:
    FileContentIndexerProvider* m_provider;

    QAtomicInt m_stop;

    QString m_currentFile;

    QStringList m_registeredMonitors;
    QDBusServiceWatcher m_monitorWatcher;
};

}

#endif // BALOO_FILECONTENTINDEXER_H
