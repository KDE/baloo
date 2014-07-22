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

#include "filemodifyjob.h"
#include "filemapping.h"
#include "db.h"
#include "file.h"
#include "searchstore.h"
#include "baloo_xattr_p.h"

#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QStringList>

#include <QDBusMessage>
#include <QDBusConnection>

using namespace Baloo;

class Baloo::FileModifyJob::Private {
public:
    QList<File> files;
    int rating;
    QString comment;
    QStringList tags;

    bool ratingSet;
    bool commentSet;
    bool tagsSet;

    Private()
        : rating(0)
        , ratingSet(false)
        , commentSet(false)
        , tagsSet(false)
    {}
};

FileModifyJob::FileModifyJob(QObject* parent)
    : KJob(parent)
    , d(new Private)
{
}

FileModifyJob::FileModifyJob(const File& file, QObject* parent)
    : KJob(parent)
    , d(new Private)
{
    d->files.append(file);
    d->rating = file.rating();
    d->comment = file.userComment();
    d->tags = file.tags();

    d->ratingSet = d->commentSet = d->tagsSet = true;
}

FileModifyJob::~FileModifyJob()
{
    delete d;
}

void FileModifyJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void FileModifyJob::doStart()
{
    QStringList updatedFiles;

    Q_FOREACH (const File& file, d->files) {
        FileMapping fileMap(file.url());
        if (!file.id().isEmpty()) {
            int id = deserialize("file", file.id());
            fileMap.setId(id);
        }

        if (fileMap.url().isEmpty()) {
            if (fileMap.empty()) {
                setError(Error_EmptyFile);
                setErrorText(QLatin1String("Invalid Argument"));
                emitResult();
                return;
            }

            if (!fileMap.fetch(fileMappingDb())) {
                setError(Error_EmptyFile);
                setErrorText(QLatin1String("Invalid Argument"));
                emitResult();
                return;
            }
        }

        if (!QFile::exists(fileMap.url())) {
            setError(Error_FileDoesNotExist);
            setErrorText(QString::fromLatin1("Does not exist %1").arg(fileMap.url()));
            emitResult();
            return;
        }

        updatedFiles << fileMap.url();
        const QString furl = fileMap.url();

        if (d->ratingSet) {
            const QString rat = QString::number(d->rating);
            baloo_setxattr(furl, QLatin1String("user.baloo.rating"), rat);
        }

        if (d->tagsSet) {
            const QString tags = d->tags.join(QLatin1String(","));
            baloo_setxattr(furl, QLatin1String("user.xdg.tags"), tags);
        }

        if (d->commentSet) {
            baloo_setxattr(furl, QLatin1String("user.xdg.comment"), d->comment);
        }
    }

    // Notify the world?
    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(updatedFiles);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    emitResult();
}

namespace {
    QList<File> convertToFiles(const QStringList& fileurls)
    {
        QList<File> files;
        Q_FOREACH (const QString& url, fileurls) {
            files << File(url);
        }
        return files;
    }
}

FileModifyJob* FileModifyJob::modifyRating(const QStringList& files, int rating)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = convertToFiles(files);
    job->d->rating = rating;
    job->d->ratingSet = true;

    return job;
}

FileModifyJob* FileModifyJob::modifyTags(const QStringList& files, const QStringList& tags)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = convertToFiles(files);
    job->d->tags = tags;
    job->d->tagsSet = true;

    return job;
}

FileModifyJob* FileModifyJob::modifyUserComment(const QStringList& files, const QString& comment)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = convertToFiles(files);
    job->d->comment = comment;
    job->d->commentSet = true;

    return job;
}
