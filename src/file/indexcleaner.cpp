/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010-2011 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2010-2013 Vishesh Handa <handa.vish@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "indexcleaner.h"
#include "fileindexerconfig.h"
#include "util.h"

#include <QtCore/QTimer>
#include <QtCore/QMutexLocker>
#include <QtCore/QSet>
#include <KDebug>
#include <KUrl>

using namespace Baloo;

namespace
{
/**
 * Creates one SPARQL filter expression that excludes the include folders.
 * This is necessary since constructFolderSubFilter will append a slash to
 * each folder to make sure it does not match something like
 * '/home/foobar' with '/home/foo'.
 */
QString constructExcludeIncludeFoldersFilter(const QStringList& folders)
{
    QStringList filters;
    QStringList used;
    Q_FOREACH (const QString& folder, folders) {
        if (!used.contains(folder)) {
            used << folder;
            //filters << QString::fromLatin1("(?url!=%1)").arg(Soprano::Node::resourceToN3(KUrl(folder)));
        }
    }
    return filters.join(QLatin1String(" && "));
}
}


IndexCleaner::IndexCleaner(QObject* parent)
    : KJob(parent),
      m_suspended(false),
      m_delay(0)
{
    setCapabilities(Suspendable);
}


void IndexCleaner::start()
{
    kDebug() << "CLEANING!!";
    const QString folderFilter = constructExcludeFolderFilter(FileIndexerConfig::self());

    const int limit = 20;

    /*
    //
    // Create all queries that return indexed data which should not be there anymore.
    //

    //
    // Query the nepomukindexer app resource in order to speed up the queries.
    //
    QUrl appRes;
    Soprano::Model* model = ResourceManager::instance()->mainModel();
    QString appQuery = QString::fromLatin1("select ?app where { ?app nao:identifier %1 . } LIMIT 1")
                       .arg(Soprano::Node::literalToN3(QLatin1String("nepomukindexer")));

    Soprano::QueryResultIterator appIt
        = model->executeQuery(appQuery, Soprano::Query::QueryLanguageSparql);
    if (appIt.next()) {
        appRes = appIt[0].uri();
    }

    //
    // 1. Data that has been created in KDE >= 4.7 using the DMS
    //
    if (!appRes.isEmpty()) {
        m_removalQueries << QString::fromLatin1("select distinct ?r where { "
                                                "graph ?g { ?r nie:url ?url . } . "
                                                "?g nao:maintainedBy %1 . "
                                                " %2 } LIMIT %3")
                         .arg(Soprano::Node::resourceToN3(appRes),
                              folderFilter,
                              QString::number(limit));
    }


    //
    // 2. Build filter query for all exclude filters
    //
    const QString fileFilters = constructExcludeFiltersFilenameFilter(FileIndexerConfig::self());
    const QString includeExcludeFilters = constructExcludeIncludeFoldersFilter(FileIndexerConfig::self()->includeFolders());

    QString filters;
    if (!includeExcludeFilters.isEmpty() && !fileFilters.isEmpty())
        filters = QString::fromLatin1("FILTER((%1) && (%2)) .").arg(includeExcludeFilters, fileFilters);
    else if (!fileFilters.isEmpty())
        filters = QString::fromLatin1("FILTER(%1) .").arg(fileFilters);
    else if (!includeExcludeFilters.isEmpty())
        filters = QString::fromLatin1("FILTER(%1) .").arg(includeExcludeFilters);

    if (!filters.isEmpty()) {
        // 2.1. Data for files which are excluded through filters
        if (!appRes.isEmpty()) {
            m_removalQueries << QString::fromLatin1("select distinct ?r where { "
                                                    "graph ?g { ?r nie:url ?url . } . "
                                                    "?r nfo:fileName ?fn . "
                                                    "?g nao:maintainedBy %1 . "
                                                    "FILTER(REGEX(STR(?url),\"^file:/\")) . "
                                                    "%2 } LIMIT %3")
                             .arg(Soprano::Node::resourceToN3(appRes),
                                  filters)
                             .arg(limit);
        }
    }

    // 2.2. Data for files which have paths that are excluded through exclude filters
    const QString excludeFiltersFolderFilter = constructExcludeFiltersFolderFilter(FileIndexerConfig::self());
    if (!excludeFiltersFolderFilter.isEmpty()) {
        m_removalQueries << QString::fromLatin1("select distinct ?r where { "
                                                "graph ?g { ?r nie:url ?url . } . "
                                                "?g nao:maintainedBy %1 . "
                                                "FILTER(REGEX(STR(?url),\"^file:/\") && %2) . "
                                                "} LIMIT %3")
                         .arg(Soprano::Node::resourceToN3(appRes),
                              excludeFiltersFolderFilter)
                         .arg(limit);
    }

    //
    // Start the removal
    //
    m_query = m_removalQueries.dequeue();
    if (!m_suspended) {
        QTimer::singleShot(m_delay, this, SLOT(clearNextBatch()));
    }*/
}

void IndexCleaner::slotRemoveResourcesDone(KJob* job)
{
    if (job->error()) {
        kDebug() << job->errorString();
    }

    QMutexLocker lock(&m_stateMutex);
    if (!m_suspended) {
        QTimer::singleShot(m_delay, this, SLOT(clearNextBatch()));
    }
}

void IndexCleaner::clearNextBatch()
{
    /*
    QList<QUrl> resources;
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery(m_query, Soprano::Query::QueryLanguageSparqlNoInference);
    while (it.next()) {
        resources << it[0].uri();
    }

    if (!resources.isEmpty()) {
        kDebug() << m_query;
        kDebug() << resources;
        KJob* job = clearIndexedData(resources);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotRemoveResourcesDone(KJob*)), Qt::QueuedConnection);
    }

    else if (!m_removalQueries.isEmpty()) {
        m_query = m_removalQueries.dequeue();
        QTimer::singleShot(m_delay, this, SLOT(clearNextBatch()));
    }

    else {
        emitResult();
    }*/
}

bool IndexCleaner::doSuspend()
{
    QMutexLocker locker(&m_stateMutex);
    m_suspended = true;
    return true;
}

bool IndexCleaner::doResume()
{
    QMutexLocker locker(&m_stateMutex);
    if (m_suspended) {
        m_suspended = false;
        QTimer::singleShot(0, this, SLOT(clearNextBatch()));
    }
    return true;
}

void IndexCleaner::setDelay(int msecs)
{
    m_delay = msecs;
}


namespace
{
QString constructFolderSubFilter(const QList<QPair<QString, bool> > folders, int& index)
{
    QString path = folders[index].first;
    if (!path.endsWith('/'))
        path += '/';
    const bool include = folders[index].second;

    ++index;

    QStringList subFilters;
    while (index < folders.count() &&
            folders[index].first.startsWith(path)) {
        subFilters << constructFolderSubFilter(folders, index);
    }

    QString str = QString::fromLatin1(KUrl(path).toEncoded());
    str.replace('\'', QLatin1String("\\'"));
    QString thisFilter = QString::fromLatin1("REGEX(STR(?url),'^%1')").arg(str);

    // we want all folders that should NOT be indexed
    if (include) {
        thisFilter.prepend('!');
    }
    subFilters.prepend(thisFilter);

    if (subFilters.count() > 1) {
        return '(' + subFilters.join(include ? QLatin1String(" || ") : QLatin1String(" && ")) + ')';
    } else {
        return subFilters.first();
    }
}

/**
 * Returns true if the specified folder f would already be included using the list
 * folders.
 */
bool alreadyIncluded(const QList<QPair<QString, bool> >& folders, const QString& f)
{
    bool included = false;
    for (int i = 0; i < folders.count(); ++i) {
        if (f != folders[i].first &&
                f.startsWith(KUrl(folders[i].first).path(KUrl::AddTrailingSlash))) {
            included = folders[i].second;
        }
    }
    return included;
}

/**
 * Remove useless include entries which would result in regex filters that match everything. This
 * is close to the code in FileIndexerConfig which removes useless exclude entries.
 */
void cleanupList(QList<QPair<QString, bool> >& result)
{
    int i = 0;
    while (i < result.count()) {
        if (result[i].first.isEmpty() ||
                (result[i].second &&
                 alreadyIncluded(result, result[i].first)))
            result.removeAt(i);
        else
            ++i;
    }
}
}

// static
QString IndexCleaner::constructExcludeFolderFilter(FileIndexerConfig* cfg)
{
    //
    // This filter consists of two parts:
    // 1. A set of filter terms which exlude the actual include folders themselves from being removed
    // 2. A set of filter terms which is a recursive sequence of inclusion and exclusion
    //
    QStringList subFilters(constructExcludeIncludeFoldersFilter(cfg->includeFolders()));

    // now add the actual filters
    QList<QPair<QString, bool> > folders = cfg->folders();
    cleanupList(folders);
    int index = 0;
    while (index < folders.count()) {
        subFilters << constructFolderSubFilter(folders, index);
    }
    QString filters = subFilters.join(" && ");
    if (!filters.isEmpty())
        return QString::fromLatin1("FILTER(%1) .").arg(filters);

    return QString();
}


namespace
{
QString excludeFilterToSparqlRegex(const QString& filter)
{
    QString filterRxStr = QRegExp::escape(filter);
    filterRxStr.replace("\\*", QLatin1String(".*"));
    filterRxStr.replace("\\?", QLatin1String("."));
    filterRxStr.replace('\\', "\\\\");
    return filterRxStr;
}
}

// static
QString IndexCleaner::constructExcludeFiltersFilenameFilter(FileIndexerConfig* cfg)
{
    //
    // This is stright-forward: we convert the filters into SPARQL regex syntax
    // and then combine them with the || operator.
    //
    QStringList fileFilters;
    Q_FOREACH (const QString& filter, cfg->excludeFilters()) {
        fileFilters << QString::fromLatin1("REGEX(STR(?fn),\"^%1$\")").arg(excludeFilterToSparqlRegex(filter));
    }
    return fileFilters.join(QLatin1String(" || "));
}


// static
QString IndexCleaner::constructExcludeFiltersFolderFilter(FileIndexerConfig* cfg)
{
    //
    // In order to find the entries which we should remove based on matching exclude filters in path
    // components we need to consider two things:
    // 1. For each exclude filter find entries which contain "/FILTER/" in their URL. The ones which
    //    have it in their file name are already matched in constructExcludeFiltersFilenameFilter.
    // 2. If there are include folders which have a path component matching one of the exclude filters
    //    we need to add additional filter terms to make sure we do not remove any of the files in them.
    // 2.1. The exception are URLs that have a path component which matches one of the exclude filters
    //      in the path relative to the include folder.
    //

    // build our own cache of the exclude filters
    const QStringList excludeFilters = cfg->excludeFilters();
    RegExpCache excludeFilterCache;
    excludeFilterCache.rebuildCacheFromFilterList(excludeFilters);
    QList<QRegExp> excludeRegExps = excludeFilterCache.regExps();

    //
    // Find all the include folders that have a path component which should normally be excluded through
    // the exclude filters.
    // We create a mapping from exclude filter to the include folders in question.
    //
    QMultiHash<QString, QString> includeFolders;
    Q_FOREACH (const QString& folder, cfg->includeFolders()) {
        const QStringList components = folder.split('/', QString::SkipEmptyParts);
        Q_FOREACH (const QString& c, components) {
            for (int i = 0; i < excludeRegExps.count(); ++i) {
                if (excludeRegExps[i].exactMatch(c)) {
                    includeFolders.insert(excludeRegExps[i].pattern(), folder);
                }
            }
        }
    }

    //
    // Build the SPARQL filters that match the urls to remove
    //
    QStringList urlFilters;
    Q_FOREACH (const QString& filter, excludeFilters) {
        QStringList terms;

        // 1. Create the basic filter term to get all urls that match the exclude filter
        terms << QString::fromLatin1("REGEX(STR(?url),'/%1/')").arg(excludeFilterToSparqlRegex(filter));

        // 2. Create special cases for all include folders that have a matching path component
        // (the "10000" is just some random value which should make sure we get all the urls)
        Q_FOREACH (const QString& folder, includeFolders.values(filter)) {
            const QString encodedUrl = QString::fromAscii(KUrl(folder).toEncoded());
            terms << QString::fromLatin1("(!REGEX(STR(?url),'^%1/') || REGEX(bif:substring(STR(?url),%2,10000),'/%3/'))")
                  .arg(encodedUrl)
                  .arg(encodedUrl.length() + 1)
                  .arg(excludeFilterToSparqlRegex(filter));
        }

        // 3. Put all together
        urlFilters << QLatin1String("(") + terms.join(QLatin1String(" && ")) + QLatin1String(")");
    }

    //
    // Combine the generated filter terms with the typical include folder exclusion filter which makes
    // sure that we do not remove the include folders themselves.
    //
    if (!urlFilters.isEmpty()) {
        QString filter;
        if (!includeFolders.values().isEmpty()) {
            filter += constructExcludeIncludeFoldersFilter(includeFolders.values())
                      += QLatin1String(" && ");
        }
        filter += QLatin1String("(") + urlFilters.join(QLatin1String(" || ")) + QLatin1String(")");
        return filter;
    } else {
        return QString();
    }
}

#include "indexcleaner.moc"
