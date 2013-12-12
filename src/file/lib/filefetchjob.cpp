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

#include "filefetchjob.h"
#include "filemapping.h"
#include "db.h"
#include "file.h"
#include "file_p.h"

#include <QTimer>
#include <QFile>

#include <xapian.h>
#include <qjson/parser.h>

#include <KDebug>

using namespace Baloo;

class FileFetchJob::Private {
public:
    QString url;
    QByteArray id;

    File m_file;
};

FileFetchJob::FileFetchJob(const QString& url, QObject* parent)
    : ItemFetchJob(parent)
    , d(new Private)
{
    d->url = url;
}

FileFetchJob::FileFetchJob(const File& file, QObject* parent)
    : ItemFetchJob(parent)
    , d(new Private)
{
    d->id = file.id();
    d->url = file.url();
}

FileFetchJob::~FileFetchJob()
{
    delete d;
}

void FileFetchJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void FileFetchJob::doStart()
{
    const QString& url = d->url;
    if (url.size() && !QFile::exists(url)) {
        setError(Error_FileDoesNotExist);
        setErrorText("File " + url + " does not exist");
        emitResult();
        return;
    }

    if (d->id.size() && !d->id.startsWith("file")) {
        setError(Error_InvalidId);
        setErrorText("Invalid Id " + QString::fromUtf8(d->id));
        emitResult();
        return;
    }

    FileMapping fileMap;
    fileMap.setId(deserialize("file", d->id));
    fileMap.setUrl(d->url);

    if (!fileMap.fetch(fileMappingDb())) {
        // TODO: Send file for indexing!!
        emitResult();
        return;
    }

    const int id = fileMap.id();
    d->m_file.setId(serialize("file", id));
    d->m_file.d->url = fileMap.url();

    // Fetch data from Xapian
    Xapian::Database db(fileIndexDbPath().toStdString());
    try {
        Xapian::Document doc = db.get_document(id);

        std::string docData = doc.get_data();
        const QByteArray arr(docData.c_str(), docData.length());

        QJson::Parser parser;
        d->m_file.d->variantMap = parser.parse(arr).toMap();
    }
    catch (const Xapian::DocNotFoundError&){
        // Send file for indexing to baloo_file
    }
    catch (const Xapian::InvalidArgumentError& err) {
        kError() << err.get_msg().c_str();
    }

    Q_EMIT itemReceived(d->m_file);
    Q_EMIT fileReceived(d->m_file);
    emitResult();
}

File FileFetchJob::file() const
{
    return d->m_file;
}

