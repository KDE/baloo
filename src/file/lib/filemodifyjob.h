/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#ifndef _BALOO_FILEMODIFYJOB_H
#define _BALOO_FILEMODIFYJOB_H

#include "file_export.h"
#include <KJob>

namespace Baloo {

class File;

class BALOO_FILE_EXPORT FileModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit FileModifyJob(QObject* parent = 0);
    FileModifyJob(const File& file, QObject* parent = 0);
    virtual ~FileModifyJob();

    virtual void start();

    static FileModifyJob* modifyRating(const QStringList& files, int rating);
    static FileModifyJob* modifyUserComment(const QStringList& files, const QString& comment);
    static FileModifyJob* modifyTags(const QStringList& files, const QStringList& tags);

    enum Errors {
        Error_FileDoesNotExist = KJob::UserDefinedError + 1,
        Error_EmptyFile,
    };

private Q_SLOTS:
    void doStart();
    void slotCommitted();

private:
    class Private;
    Private* d;
};

}
#endif // _BALOO_FILEMODIFYJOB_H
