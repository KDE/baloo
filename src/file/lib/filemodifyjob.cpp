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
#include <KDebug>

#include <QTimer>
#include <QFile>
#include <QStringList>

#include <xapian.h>
#include <attr/xattr.h>

using namespace Baloo;

class Baloo::FileModifyJob::Private {
public:
    File file;

    QStringList files;
    int rating;
    QString comment;
    QStringList tags;

    Private() : rating(0) {}
};

FileModifyJob::FileModifyJob(QObject* parent)
    : ItemSaveJob(parent)
    , d(new Private)
{
}

FileModifyJob::FileModifyJob(const File& file, QObject* parent)
    : ItemSaveJob(parent)
    , d(new Private)
{
    d->file = file;
}

FileModifyJob::~FileModifyJob()
{
    delete d;
}

void FileModifyJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

namespace {

void removeTerms(Xapian::Document& doc, const std::string& p) {
    QByteArray prefix = QByteArray::fromRawData(p.c_str(), p.size());

    Xapian::TermIterator it = doc.termlist_begin();
    it.skip_to(p);
    while (it != doc.termlist_end()){
        const std::string t = *it;
        const QByteArray term = QByteArray::fromRawData(t.c_str(), t.size());
        if (!term.startsWith(prefix)) {
            break;
        }

        // The term should not just be the prefix
        if (term.size() <= prefix.size())
            break;

        // The term should not contain any more upper case letters
        if (isupper(term.at(prefix.size())))
            break;

        it++;
        doc.remove_term(t);
    }
}

}

void FileModifyJob::doStart()
{
    if (d->files.isEmpty())
        doSingleFile();
    else
        doMultipleFiles();
}

void FileModifyJob::doSingleFile()
{
    FileMapping fileMap(d->file.url());
    if (!d->file.id().isEmpty()) {
        int id = deserialize("file", d->file.id());
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

    const QByteArray furl = QFile::encodeName(fileMap.url());

    QByteArray rat = QString::number(d->file.rating()).toUtf8();
    setxattr(furl.constData(), "user.baloo.rating", rat.constData(), rat.size(), 0);

    // TODO: Normalize the tags?
    QByteArray tags = d->file.tags().join(",").toUtf8();
    setxattr(furl.constData(), "user.baloo.tags", tags.constData(), tags.size(), 0);

    QByteArray com = d->file.userComment().toUtf8();
    setxattr(furl.constData(), "user.xdg.comment", com.constData(), com.size(), 0);

    // Save in Xapian
    const std::string path = fileIndexDbPath().toStdString();
    Xapian::WritableDatabase db(path, Xapian::DB_CREATE_OR_OPEN);
    Xapian::Document doc;

    try {
        doc = db.get_document(fileMap.id());

        removeTerms(doc, "R");
        removeTerms(doc, "TAG");
        removeTerms(doc, "C");
    }
    catch (const Xapian::DocNotFoundError&) {
    }

    const int rating = d->file.rating();
    if (rating > 0) {
        const QString ratingStr = QLatin1Char('R') + QString::number(d->file.rating());
        doc.add_boolean_term(ratingStr.toStdString());
    }

    Q_FOREACH (const QString& tag, d->file.tags()) {
        const QString tagStr = "TAG" + tag.toLower();
        doc.add_term(tagStr.toStdString());
    }

    if (!d->file.userComment().isEmpty()) {
        Xapian::TermGenerator termGen;
        termGen.set_document(doc);

        const std::string str = d->file.userComment().toLower().toStdString();
        termGen.index_text(str, 1, "C");
    }

    db.replace_document(fileMap.id(), doc);
    db.commit();

    // Notify the world?

    emitResult();
}

void FileModifyJob::doMultipleFiles()
{
    Q_FOREACH (const QString fileUrl, d->files) {
        FileMapping fileMap(fileUrl);

        if (!fileMap.fetch(fileMappingDb())) {
            fileMap.create(fileMappingDb());
        }

        if (!QFile::exists(fileMap.url())) {
            setError(Error_FileDoesNotExist);
            setErrorText("Does not exist " + fileMap.url());
            emitResult();
            return;
        }

        const QByteArray furl = QFile::encodeName(fileMap.url());

        if (d->rating) {
            QByteArray rat = QString::number(d->rating).toUtf8();
            setxattr(furl.constData(), "user.baloo.rating", rat.constData(), rat.size(), 0);
        }

        if (!d->tags.isEmpty()) {
            QByteArray tags = d->tags.join(",").toUtf8();
            setxattr(furl.constData(), "user.baloo.tags", tags.constData(), tags.size(), 0);
        }

        if (!d->comment.isEmpty()) {
            QByteArray com = d->comment.toUtf8();
            setxattr(furl.constData(), "user.xdg.comment", com.constData(), com.size(), 0);
        }

        // Save in Xapian
        const std::string path = fileIndexDbPath().toStdString();
        Xapian::WritableDatabase db(path, Xapian::DB_CREATE_OR_OPEN);
        Xapian::Document doc;

        try {
            doc = db.get_document(fileMap.id());

            removeTerms(doc, "R");
            removeTerms(doc, "TAG");
            removeTerms(doc, "C");
        }
        catch (const Xapian::DocNotFoundError&) {
        }

        const int rating = d->rating;
        if (rating > 0) {
            const QString ratingStr = QLatin1Char('R') + QString::number(d->file.rating());
            doc.add_boolean_term(ratingStr.toStdString());
        }

        Q_FOREACH (const QString& tag, d->tags) {
            const QString tagStr = "TAG" + tag.toLower();
            doc.add_term(tagStr.toStdString());
        }

        if (!d->comment.isEmpty()) {
            Xapian::TermGenerator termGen;
            termGen.set_document(doc);

            const std::string str = d->file.userComment().toLower().toStdString();
            termGen.index_text(str, 1, "C");
        }

        db.replace_document(fileMap.id(), doc);
        db.commit();
    }

    // Notify the world?

    emitResult();

}

FileModifyJob* FileModifyJob::modifyRating(const QStringList& files, int rating)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = files;
    job->d->rating = rating;

    return job;
}

FileModifyJob* FileModifyJob::modifyTags(const QStringList& files, const QStringList& tags)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = files;
    job->d->tags = tags;

    return job;
}

FileModifyJob* FileModifyJob::modifyUserComment(const QStringList& files, const QString& comment)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = files;
    job->d->comment = comment;

    return job;
}
