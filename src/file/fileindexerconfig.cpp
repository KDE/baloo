/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2013      Vishesh Handa <me@vhanda.in>

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

#include "fileindexerconfig.h"
#include "fileexcludefilters.h"

#include <QStringList>
#include <QDir>
#include <QWriteLocker>
#include <QReadLocker>

#include <KDirWatch>
#include <KStandardDirs>
#include <KConfigGroup>
#include <KDebug>


namespace
{
/// recursively check if a folder is hidden
bool isDirHidden(QDir& dir)
{
    if (QFileInfo(dir.path()).isHidden())
        return true;
    else if (dir.cdUp())
        return isDirHidden(dir);
    else
        return false;
}
}

using namespace Baloo;

FileIndexerConfig::FileIndexerConfig(QObject* parent)
    : QObject(parent)
    , m_config("baloofilerc")
    , m_indexHidden(false)
{
    KDirWatch* dirWatch = KDirWatch::self();
    connect(dirWatch, SIGNAL(dirty(const QString&)),
            this, SLOT(slotConfigDirty()));
    connect(dirWatch, SIGNAL(created(const QString&)),
            this, SLOT(slotConfigDirty()));
    dirWatch->addFile(KStandardDirs::locateLocal("config", m_config.name()));

    forceConfigUpdate();
}


FileIndexerConfig::~FileIndexerConfig()
{
}


QList<QPair<QString, bool> > FileIndexerConfig::folders() const
{
    return m_folderCache;
}


QStringList FileIndexerConfig::includeFolders() const
{
    QStringList fl;
    for (int i = 0; i < m_folderCache.count(); ++i) {
        if (m_folderCache[i].second)
            fl << m_folderCache[i].first;
    }
    return fl;
}


QStringList FileIndexerConfig::excludeFolders() const
{
    QStringList fl;
    for (int i = 0; i < m_folderCache.count(); ++i) {
        if (!m_folderCache[i].second)
            fl << m_folderCache[i].first;
    }
    return fl;
}


QStringList FileIndexerConfig::excludeFilters() const
{
    KConfigGroup cfg = m_config.group("General");

    // read configured exclude filters
    QSet<QString> filters = cfg.readEntry("exclude filters", defaultExcludeFilterList()).toSet();

    // make sure we always keep the latest default exclude filters
    // TODO: there is one problem here. What if the user removed some of the default filters?
    if (cfg.readEntry("exclude filters version", 0) < defaultExcludeFilterListVersion()) {
        filters += defaultExcludeFilterList().toSet();

        // write the config directly since the KCM does not have support for the version yet
        // TODO: make this class public and use it in the KCM
        cfg.writeEntry("exclude filters", QStringList::fromSet(filters));
        cfg.writeEntry("exclude filters version", defaultExcludeFilterListVersion());
    }

    // remove duplicates
    return QStringList::fromSet(filters);
}


bool FileIndexerConfig::indexHiddenFilesAndFolders() const
{
    return m_indexHidden;
}

void FileIndexerConfig::slotConfigDirty()
{
    if (forceConfigUpdate())
        Q_EMIT configChanged();
}


bool FileIndexerConfig::isInitialRun() const
{
    return m_config.group("General").readEntry("first run", true);
}


bool FileIndexerConfig::shouldBeIndexed(const QString& path) const
{
    QFileInfo fi(path);
    if (fi.isDir()) {
        return shouldFolderBeIndexed(path);
    } else {
        return (shouldFolderBeIndexed(fi.absolutePath()) &&
                (!fi.isHidden() || indexHiddenFilesAndFolders()) &&
                shouldFileBeIndexed(fi.fileName()));
    }
}


bool FileIndexerConfig::shouldFolderBeIndexed(const QString& path) const
{
    QString folder;
    if (folderInFolderList(path, folder)) {
        // we always index the folders in the list
        // ignoring the name filters
        if (folder == path)
            return true;

        // check for hidden folders
        QDir dir(path);
        if (!indexHiddenFilesAndFolders() && isDirHidden(dir))
            return false;

        // reset dir, cause isDirHidden modifies the QDir
        dir = path;

        // check the exclude filters for all components of the path
        // after folder
        const QStringList pathComponents = path.mid(folder.count()).split('/', QString::SkipEmptyParts);
        Q_FOREACH (const QString& c, pathComponents) {
            if (!shouldFileBeIndexed(c)) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}


bool FileIndexerConfig::shouldFileBeIndexed(const QString& fileName) const
{
    // check the filters
    QWriteLocker lock(&m_folderCacheMutex);
    return !m_excludeFilterRegExpCache.exactMatch(fileName);
}

bool FileIndexerConfig::shouldMimeTypeBeIndexed(const QString& mimeType) const
{
    QReadLocker lock(&m_mimetypeMutex);
    return !m_excludeMimetypes.contains(mimeType);
}


bool FileIndexerConfig::folderInFolderList(const QString& path)
{
    QString str;
    return folderInFolderList(path, str);
}

bool FileIndexerConfig::folderInFolderList(const QString& path, QString& folder) const
{
    QReadLocker lock(&m_folderCacheMutex);

    const QString p = KUrl(path).path(KUrl::RemoveTrailingSlash);

    // we traverse the list backwards to catch all exclude folders
    int i = m_folderCache.count();
    while (--i >= 0) {
        const QString& f = m_folderCache[i].first;
        const bool include = m_folderCache[i].second;
        if (p.startsWith(f)) {
            folder = f;
            return include;
        }
    }
    // path is not in the list, thus it should not be included
    folder.clear();
    return false;
}


namespace
{
/**
 * Returns true if the specified folder f would already be excluded using the list
 * folders.
 */
bool alreadyExcluded(const QList<QPair<QString, bool> >& folders, const QString& f)
{
    bool included = false;
    for (int i = 0; i < folders.count(); ++i) {
        if (f != folders[i].first &&
                f.startsWith(KUrl(folders[i].first).path(KUrl::AddTrailingSlash))) {
            included = folders[i].second;
        }
    }
    return !included;
}

/**
 * Simple insertion sort
 */
void insertSortFolders(const QStringList& folders, bool include, QList<QPair<QString, bool> >& result)
{
    Q_FOREACH (const QString& f, folders) {
        int pos = 0;
        QString path = KUrl(f).path(KUrl::RemoveTrailingSlash);
        while (result.count() > pos &&
                result[pos].first < path)
            ++pos;
        result.insert(pos, qMakePair(path, include));
    }
}

/**
 * Remove useless exclude entries which would confuse the folderInFolderList algo.
 * This makes sure all top-level items are include folders.
 * This runs in O(n^2) and could be optimized but what for.
 */
void cleanupList(QList<QPair<QString, bool> >& result)
{
    int i = 0;
    while (i < result.count()) {
        if (result[i].first.isEmpty() ||
                (!result[i].second &&
                 alreadyExcluded(result, result[i].first)))
            result.removeAt(i);
        else
            ++i;
    }
}
}


void FileIndexerConfig::fillIncludeFolderChanges(const FileIndexerConfig::Entry& entry, const QSet<QString>& include, QStringList* includeAdded, QStringList* includeRemoved)
{
    QStringList added = QSet<QString>(include).subtract(entry.includes).toList();
    QStringList removed = QSet<QString>(entry.includes).subtract(include).toList();

    if (includeAdded)
        includeAdded->append(added);

    if (includeRemoved)
        includeRemoved->append(removed);
}

void FileIndexerConfig::fillExcludeFolderChanges(const FileIndexerConfig::Entry& entry, const QSet<QString>& exclude, QStringList* excludeAdded, QStringList* excludeRemoved)
{
    QStringList added = QSet<QString>(exclude).subtract(entry.excludes).toList();
    QStringList removed = QSet<QString>(entry.excludes).subtract(exclude).toList();

    if (excludeAdded)
        excludeAdded->append(added);

    if (excludeRemoved)
        excludeRemoved->append(removed);
}


bool FileIndexerConfig::buildFolderCache()
{
    QWriteLocker lock(&m_folderCacheMutex);

    //
    // General folders
    //
    KConfigGroup group = m_config.group("General");
    QStringList includeFoldersPlain = group.readPathEntry("folders", QStringList() << QDir::homePath());
    QStringList excludeFoldersPlain = group.readPathEntry("exclude folders", QStringList());

    m_folderCache.clear();
    insertSortFolders(includeFoldersPlain, true, m_folderCache);
    insertSortFolders(excludeFoldersPlain, false, m_folderCache);

    QStringList includeAdded;
    QStringList includeRemoved;
    QStringList excludeAdded;
    QStringList excludeRemoved;;

    QSet<QString> includeSet = includeFoldersPlain.toSet();
    QSet<QString> excludeSet = excludeFoldersPlain.toSet();

    Entry& generalEntry = m_entries[ "General" ];
    fillIncludeFolderChanges(generalEntry, includeSet, &includeAdded, &includeRemoved);
    fillExcludeFolderChanges(generalEntry, excludeSet, &excludeAdded, &excludeRemoved);

    generalEntry.includes = includeSet;
    generalEntry.excludes = excludeSet;

    //
    // Removable Media
    //
    QStringList groupList = m_config.groupList();
    Q_FOREACH (const QString& groupName, groupList) {
        if (!groupName.startsWith("Device-"))
            continue;

        KConfigGroup group = m_config.group(groupName);
        QString mountPath = group.readEntry("mount path", QString());
        if (mountPath.isEmpty())
            continue;

        QStringList includes = group.readPathEntry("folders", QStringList());
        QStringList excludes = group.readPathEntry("exclude folders", QStringList());

        QStringList includeFoldersPlain;
        Q_FOREACH (const QString& path, includes)
            includeFoldersPlain << mountPath + path;

        QStringList excludeFoldersPlain;
        Q_FOREACH (const QString& path, excludes)
            excludeFoldersPlain << mountPath + path;

        insertSortFolders(includeFoldersPlain, true, m_folderCache);
        insertSortFolders(excludeFoldersPlain, false, m_folderCache);

        QSet<QString> includeSet = includeFoldersPlain.toSet();
        QSet<QString> excludeSet = excludeFoldersPlain.toSet();

        Entry& cacheEntry = m_entries[ groupName ];
        fillIncludeFolderChanges(cacheEntry, includeSet, &includeAdded, &includeRemoved);
        fillExcludeFolderChanges(cacheEntry, excludeSet, &excludeAdded, &excludeRemoved);

        cacheEntry.includes = includeSet;
        cacheEntry.excludes = excludeSet;
    }

    cleanupList(m_folderCache);

    bool changed = false;
    if (!includeAdded.isEmpty() || !includeRemoved.isEmpty()) {
        Q_EMIT includeFolderListChanged(includeAdded, includeRemoved);
        changed = true;
    }

    if (!excludeAdded.isEmpty() || !excludeRemoved.isEmpty()) {
        Q_EMIT excludeFolderListChanged(excludeAdded, excludeRemoved);
        changed = true;
    }

    return changed;
}


bool FileIndexerConfig::buildExcludeFilterRegExpCache()
{
    QWriteLocker lock(&m_folderCacheMutex);
    QStringList newFilters = excludeFilters();
    m_excludeFilterRegExpCache.rebuildCacheFromFilterList(newFilters);

    QSet<QString> newFilterSet = newFilters.toSet();
    if (m_prevFileFilters != newFilterSet) {
        m_prevFileFilters = newFilterSet;
        Q_EMIT fileExcludeFiltersChanged();
        return true;
    }

    return false;
}

bool FileIndexerConfig::buildMimeTypeCache()
{
    QWriteLocker lock(&m_mimetypeMutex);
    QStringList newMimeExcludes = m_config.group("General").readPathEntry("exclude mimetypes", QStringList());

    QSet<QString> newMimeExcludeSet = newMimeExcludes.toSet();
    if (m_excludeMimetypes != newMimeExcludeSet) {
        m_excludeMimetypes = newMimeExcludeSet;
        Q_EMIT mimeTypeFiltersChanged();
        return true;
    }

    return false;
}


bool FileIndexerConfig::forceConfigUpdate()
{
    m_config.reparseConfiguration();
    bool changed = false;

    changed = buildFolderCache() || changed;
    changed = buildExcludeFilterRegExpCache() || changed;
    changed = buildMimeTypeCache() || changed;

    bool hidden = m_config.group("General").readEntry("index hidden folders", false);
    if (hidden != m_indexHidden) {
        m_indexHidden = hidden;
        changed = true;
    }

    return changed;
}

void FileIndexerConfig::setInitialRun(bool isInitialRun)
{
    m_config.group("General").writeEntry("first run", isInitialRun);
}

bool FileIndexerConfig::initialUpdateDisabled() const
{
    return m_config.group("General").readEntry("disable initial update", false);
}

bool FileIndexerConfig::suspendOnPowerSaveDisabled() const
{
    return m_config.group("General").readEntry("disable suspend on powersave", false);
}

bool FileIndexerConfig::isDebugModeEnabled() const
{
    return m_config.group("General").readEntry("debug mode", false);
}
