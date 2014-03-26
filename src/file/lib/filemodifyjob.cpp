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
#include "filecustommetadata.h"

#include "xapiandatabase.h"
#include "xapiandocument.h"

#include <KDebug>

#include <QTimer>
#include <QFile>
#include <QStringList>

#include <xapian.h>

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

    XapianDatabase* m_db;
    QStringList m_updatedFiles;

    Private()
        : rating(0)
        , ratingSet(false)
        , commentSet(false)
        , tagsSet(false)
        , m_db(0)
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
    Q_FOREACH (const File& file, d->files) {
        FileMapping fileMap(file.url());
        if (!file.id().isEmpty()) {
            int id = deserialize("file", file.id());
            fileMap.setId(id);
        }

        if (!fileMap.fetched()) {
            if (fileMap.empty()) {
                setError(Error_EmptyFile);
                setErrorText(QLatin1String("Invalid Argument"));
                emitResult();
                return;
            }

            if (!fileMap.fetch(fileMappingDb())) {
                fileMap.create(fileMappingDb());
            }
        }

        if (!QFile::exists(fileMap.url())) {
            setError(Error_FileDoesNotExist);
            setErrorText("Does not exist " + fileMap.url());
            emitResult();
            return;
        }

        d->m_updatedFiles << fileMap.url();
        const QString furl = fileMap.url();

        if (d->ratingSet) {
            const QString rat = QString::number(d->rating);
            setCustomFileMetaData(furl, QLatin1String("user.baloo.rating"), rat);
        }

        if (d->tagsSet) {
            QString tags = d->tags.join(",");
            setCustomFileMetaData(furl, QLatin1String("user.xdg.tags"), tags);
        }

        if (d->commentSet) {
            setCustomFileMetaData(furl, QLatin1String("user.xdg.comment"), d->comment);
        }

        // Save in Xapian
        d->m_db = new XapianDatabase(QString::fromUtf8(fileIndexDbPath().c_str()));
        XapianDocument doc = d->m_db->document(fileMap.id());

        doc.removeTermStartsWith("R");
        doc.removeTermStartsWith("TA");
        doc.removeTermStartsWith("TAG");
        doc.removeTermStartsWith("C");

        const int rating = d->rating;
        if (rating > 0) {
            doc.addBoolTerm(rating, "R");
        }

        Q_FOREACH (const QString& tag, d->tags) {
            doc.indexText(tag, "TA");
            doc.addBoolTerm(tag, "TAG-");
        }

        if (!d->comment.isEmpty()) {
            doc.indexText(d->comment, "C");
        }

        d->m_db->replaceDocument(fileMap.id(), doc.doc());
        connect(d->m_db, SIGNAL(committed()), this, SLOT(slotCommitted()));
        d->m_db->commit();
    }
}

void FileModifyJob::slotCommitted()
{
    // Notify the world?
    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(d->m_updatedFiles);
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
