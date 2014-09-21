/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
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

#ifndef _BALOO_EXTRACTORCLIENT
#define _BALOO_EXTRACTORCLIENT

#include <QObject>

namespace Baloo
{

class ExtractorClient : public QObject
{
    Q_OBJECT

public:
    ExtractorClient(QObject *parent = 0);

    bool isValid() const;

    void setBinaryOutput(bool binaryOutput);
    void setFollowConfig(bool followConfig);
    void setSaveToDatabase(bool saveToDatabase);
    void setDatabasePath(bool path);
    void enableDebuging(bool debugging);

    void indexFile(const QString &file);

Q_SIGNALS:
    void extractorDied();
    void fileIndexed(const QString &file);
    void dataSaved(const QString &lastFile);
    void binaryData(const QVariantMap &data);

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void extractorStarted())
    Q_PRIVATE_SLOT(d, void extractorDead())
    Q_PRIVATE_SLOT(d, void readResponse())
};

} // namespace Baloo

#endif

