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

#include <QFileInfo>
#include <QTimer>

#include <xapian.h>
#include <qjson/parser.h>

#include <KDebug>

using namespace Baloo;

class FileFetchJob::Private {
public:
    FileMapping m_file;
    QVariantMap m_data;
};

FileFetchJob::FileFetchJob(const QString& url, QObject* parent)
    : KJob(parent)
    , d(new Private)
{
    d->m_file.setUrl(url);
}

FileFetchJob::~FileFetchJob()
{

}

void FileFetchJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void FileFetchJob::doStart()
{
    const QString& url = d->m_file.url();
    if (!QFile::exists(url)) {
        setError(Error_FileDoesNotExist);
        setErrorText("File " + url + " does not exist");
        emitResult();
        return;
    }

    if (!d->m_file.fetch(fileMappingDb())) {
        // TODO: Send file for indexing!!
        emitResult();
        return;
    }

    // Fetch data from Xapian
    Xapian::Database db(fileIndexDbPath().toStdString());
    try {
        Xapian::Document doc = db.get_document(d->m_file.id());

        std::string docData = doc.get_data();
        const QByteArray arr(docData.c_str(), docData.length());

        QJson::Parser parser;
        d->m_data = parser.parse(arr).toMap();
    }
    catch (const Xapian::DocNotFoundError&){
        // Send file for indexing to baloo_file
    }
    catch (const Xapian::InvalidArgumentError& err) {
        kError() << err.get_msg().c_str();
    }
    emitResult();
}

QVariantMap FileFetchJob::data() const
{
    return d->m_data;
}

QString FileFetchJob::url() const
{
    return d->m_file.url();
}

