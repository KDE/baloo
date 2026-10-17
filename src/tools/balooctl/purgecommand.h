/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

*/

#ifndef BALOO_PURGECOMMAND_H
#define BALOO_PURGECOMMAND_H

#include "command.h"

#include <QObject>

#include <memory>

class OrgKdeBalooMainInterface;
class QDBusServiceWatcher;
class QTimer;

namespace Baloo
{
class PurgeCommand : public QObject, public Command
{
    Q_OBJECT
public:
    explicit PurgeCommand(QObject *parent = nullptr);

    QString command() override;
    QString description() override;
    int exec(const QCommandLineParser &parser) override;

private Q_SLOTS:
    void balooIsAvailable();
    void balooIsNotAvailable();

private:
    std::unique_ptr<OrgKdeBalooMainInterface> m_mainDBusInterface;
    std::unique_ptr<QDBusServiceWatcher> m_dbusServiceWatcher;
    std::unique_ptr<QTimer> m_stopTimer;
    bool wasRunning = false;
};
}

#endif // BALOO_PURGECOMMAND_H
