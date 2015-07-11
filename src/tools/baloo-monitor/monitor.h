/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
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

#ifndef BALOOMONITOR_MONITOR_H
#define BALOOMONITOR_MONITOR_H

#include <QObject>
#include <QString>
#include <QDBusInterface>

#include "baloo_interface.h"
#include "extractor_interface.h"

namespace org {
    namespace kde {
        typedef OrgKdeBalooInterface balooInterface;
        namespace baloo {
            typedef OrgKdeBalooExtractorInterface extractorInterface;
        }
    }
}

namespace BalooMonitor {
class Monitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url NOTIFY newFileIndexed)
    Q_PROPERTY(QString suspendState READ suspendState NOTIFY indexerStateChanged)
    Q_PROPERTY(bool balooRunning MEMBER m_balooRunning NOTIFY balooStateChanged)
    Q_PROPERTY(uint totalFiles MEMBER m_totalFiles NOTIFY totalFilesChanged)
    Q_PROPERTY(uint filesIndexed MEMBER m_filesIndexed NOTIFY newFileIndexed)
    Q_PROPERTY(QString remainingTime READ remainingTime NOTIFY remainingTimeChanged)
    Q_PROPERTY(QString state READ state NOTIFY indexerStateChanged)
public:
    Monitor(QObject* parent = 0);

    // Property readers
    QString url() const { return m_url; }
    QString suspendState() const;
    QString remainingTime() const { return m_remainingTime; }
    QString state() const { return Baloo::stateString(m_indexerState); }

    // Invokable methods
    Q_INVOKABLE void toggleSuspendState();
    Q_INVOKABLE void startBaloo();

Q_SIGNALS:
    void newFileIndexed();
    void balooStateChanged();
    void totalFilesChanged();
    void remainingTimeChanged();
    void indexerStateChanged();

private Q_SLOTS:
    void newFile(const QString& url);
    void balooStarted(const QString& service);
    void slotIndexerStateChanged(Baloo::IndexerState state);

private:
    void fetchTotalFiles();
    void updateRemainingTime();

    QDBusConnection m_bus;

    QString m_url;
    bool m_balooRunning;
    Baloo::IndexerState m_indexerState;

    org::kde::balooInterface* m_balooInterface;
    org::kde::baloo::extractorInterface* m_extractorInterface;

    uint m_totalFiles;
    uint m_filesIndexed;
    QString m_remainingTime;
};
}
#endif //BALOOMONITOR_MONITOR_H
