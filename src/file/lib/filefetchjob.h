/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FILEFETCHJOB_H
#define FILEFETCHJOB_H

#include "file_export.h"
#include <KJob>
#include <QVariantMap>

namespace Baloo {

/**
 * The FileFetchJob is responsible for fetching the indexed
 * metadata for a particular file. If the file does not
 * contain any indexed metadata then no data will be returned.
 * However, the file will be sent for indexing.
 */
class BALOO_FILE_EXPORT FileFetchJob : public KJob
{
    Q_OBJECT
public:
    FileFetchJob(const QString& url, QObject* parent = 0);
    ~FileFetchJob();

    virtual void start();

    QString url() const;
    QVariantMap data() const;

    enum Errors {
        Error_FileDoesNotExist = 1
    };

private Q_SLOTS:
    void doStart();

private:
    class Private;
    Private* d;
};

}

#endif // FILEFETCHJOB_H
