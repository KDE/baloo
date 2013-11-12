/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2010-2011 Vishesh Handa <handa.vish@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "fileindexer.h"
#include "fileindexeradaptor.h"
#include "indexscheduler.h"
#include "fileindexerconfig.h"
#include "util.h"

#include <KDebug>
#include <KDirNotify>

#include "resourcemanager.h"

#include <QtCore/QTimer>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Node>

Nepomuk2::FileIndexer::FileIndexer()
    : Service2()
{
    // Create the configuration instance singleton (for thread-safety)
    // ==============================================================
    (void)new FileIndexerConfig(this);

    // setup the actual index scheduler
    // ==============================================================
    m_indexScheduler = new IndexScheduler(this);

    // setup status connections
    connect(m_indexScheduler, SIGNAL(statusStringChanged()),
            this, SIGNAL(statusStringChanged()));

    // start initial indexing honoring the hidden config option to disable it
    if (FileIndexerConfig::self()->isInitialRun() || !FileIndexerConfig::self()->initialUpdateDisabled()) {
        m_indexScheduler->updateAll();
    }

    // Connect some signals used in the DBus interface
    connect(this, SIGNAL(statusStringChanged()),
            this, SIGNAL(statusChanged()));
    connect(m_indexScheduler, SIGNAL(indexingStarted()),
            this, SIGNAL(indexingStarted()));
    connect(m_indexScheduler, SIGNAL(indexingStopped()),
            this, SIGNAL(indexingStopped()));
    connect(m_indexScheduler, SIGNAL(fileIndexingDone()),
            this, SIGNAL(fileIndexingDone()));
    connect(m_indexScheduler, SIGNAL(basicIndexingDone()),
            this, SLOT(slotIndexingDone()));

    connect(m_indexScheduler, SIGNAL(statusStringChanged()),
            this, SLOT(emitStatusMessage()));

}


Nepomuk2::FileIndexer::~FileIndexer()
{
}

void Nepomuk2::FileIndexer::slotIndexingDone()
{
    FileIndexerConfig::self()->setInitialRun(false);
}

void Nepomuk2::FileIndexer::emitStatusMessage()
{
    QString message = m_indexScheduler->userStatusString();

    emit status((int)m_indexScheduler->currentStatus(), message);
}

QString Nepomuk2::FileIndexer::statusMessage() const
{
    return m_indexScheduler->userStatusString();
}

int Nepomuk2::FileIndexer::currentStatus() const
{
    return (int)m_indexScheduler->currentStatus();
}

QString Nepomuk2::FileIndexer::userStatusString() const
{
    return m_indexScheduler->userStatusString();
}

void Nepomuk2::FileIndexer::setSuspended(bool suspend)
{
    if (suspend) {
        m_indexScheduler->suspend();
    } else {
        m_indexScheduler->resume();
    }
}


bool Nepomuk2::FileIndexer::isSuspended() const
{
    return m_indexScheduler->isSuspended();
}


bool Nepomuk2::FileIndexer::isIndexing() const
{
    return m_indexScheduler->isIndexing();
}

bool Nepomuk2::FileIndexer::isCleaning() const
{
    return m_indexScheduler->isCleaning();
}


void Nepomuk2::FileIndexer::suspend() const
{
    m_indexScheduler->suspend();
}


void Nepomuk2::FileIndexer::resume() const
{
    m_indexScheduler->resume();
}


QString Nepomuk2::FileIndexer::currentFile() const
{
    return m_indexScheduler->currentUrl().toLocalFile();
}


QString Nepomuk2::FileIndexer::currentFolder() const
{
    return KUrl(m_indexScheduler->currentUrl()).directory();
}


void Nepomuk2::FileIndexer::updateFolder(const QString& path, bool recursive, bool forced)
{
    kDebug() << "Called with path: " << path;
    QFileInfo info(path);
    if (info.exists()) {
        QString dirPath;
        if (info.isDir())
            dirPath = info.absoluteFilePath();
        else
            dirPath = info.absolutePath();

        if (FileIndexerConfig::self()->shouldFolderBeIndexed(dirPath)) {
            indexFolder(path, recursive, forced);
        }
    }
}

int Nepomuk2::FileIndexer::indexedFiles() const
{
    QString query = QString::fromLatin1("select count(distinct ?r) where { ?r kext:indexingLevel ?t. "
                                        " FILTER(?t >= %1) . }")
                    .arg(Soprano::Node::literalToN3(Soprano::LiteralValue(2)));

    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    if (it.next())
        return it[0].literal().toInt();

    return 0;
}

int Nepomuk2::FileIndexer::totalFiles() const
{
    QString query = QString::fromLatin1("select count(distinct ?r) where { ?r kext:indexingLevel ?t. }");

    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    if (it.next())
        return it[0].literal().toInt();

    return 0;
}


void Nepomuk2::FileIndexer::updateAllFolders(bool forced)
{
    m_indexScheduler->updateAll(forced);
}


void Nepomuk2::FileIndexer::indexFile(const QString& path)
{
    m_indexScheduler->analyzeFile(path);
}


void Nepomuk2::FileIndexer::indexFolder(const QString& path, bool recursive, bool forced)
{
    QFileInfo info(path);
    if (info.exists()) {
        QString dirPath;
        if (info.isDir())
            dirPath = info.absoluteFilePath();
        else
            dirPath = info.absolutePath();

        kDebug() << "Updating : " << dirPath;

        Nepomuk2::UpdateDirFlags flags;
        if (recursive)
            flags |= Nepomuk2::UpdateRecursive;
        if (forced)
            flags |= Nepomuk2::ForceUpdate;

        m_indexScheduler->updateDir(dirPath, flags);
    }
}


int main(int argc, char** argv)
{
    KAboutData aboutData("nepomukfileindexer",
                         "nepomukfileindexer",
                         ki18n("Nepomuk File Indexer"),
                         NEPOMUK_VERSION_STRING,
                         ki18n("Nepomuk File Indexer"),
                         KAboutData::License_GPL,
                         ki18n("(c) 2008-2013, Sebastian Trüg"),
                         KLocalizedString(),
                         "http://nepomuk.kde.org");
    aboutData.addAuthor(ki18n("Sebastian Trüg"), ki18n("Developer"), "trueg@kde.org");
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    Nepomuk2::Service2::initUI<Nepomuk2::FileIndexer>(argc, argv, aboutData);
}
#include "fileindexer.moc"

