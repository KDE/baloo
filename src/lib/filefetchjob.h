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

#ifndef _BALOO_FILEFETCHJOB_H
#define _BALOO_FILEFETCHJOB_H

#include "core_export.h"
#include <KJob>

namespace Baloo {

class File;

/**
 * The FileFetchJob is responsible for fetching the indexed
 * metadata for a particular file. If the file does not
 * contain any indexed metadata then no data will be returned.
 * However, the file will be sent for indexing.
 */
class BALOO_CORE_EXPORT FileFetchJob : public KJob
{
    Q_OBJECT
public:
    FileFetchJob(const QString& url, QObject* parent = 0);
    FileFetchJob(const File& file, QObject* parent = 0);
    FileFetchJob(const QStringList& urls, QObject* parent = 0);
    ~FileFetchJob();

    virtual void start();

    enum Errors {
        Error_FileDoesNotExist = 1,
        Error_InvalidId
    };

    File file() const;
    QList<File> files() const;

Q_SIGNALS:
    void fileReceived(const Baloo::File& file);

private Q_SLOTS:
    void doStart();

private:
    class Private;
    Private* d;
};

}

#endif // _BALOO_FILEFETCHJOB_H
