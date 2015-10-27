/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QTextStream>
#include <KLocalizedString>

#include "command.h"
#include "fileindexerinterface.h"

namespace Baloo {

class MonitorCommand : public QObject, public Command
{
    Q_OBJECT
public:
    explicit MonitorCommand(QObject* parent = 0);

    QString command() Q_DECL_OVERRIDE {
        return QStringLiteral("monitor");
    }

    QString description() Q_DECL_OVERRIDE {
        return i18n("CLI interface for monitoring Baloo");
    }

    int exec(const QCommandLineParser& parser);

private Q_SLOTS:
    void startedIndexingFile(const QString& filePath);
    void finishedIndexingFile(const QString& filePath);

private:
    QTextStream m_out;
    org::kde::baloo::fileindexer* m_interface;

    QString m_currentFile;
};
}
#endif // MONITOR_H
