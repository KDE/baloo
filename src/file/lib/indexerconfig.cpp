/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "indexerconfig.h"
#include "../fileindexerconfig.h"
#include "../fileexcludefilters.h"
#include "../regexpcache.h"

#include <KConfig>
#include <KConfigGroup>

using namespace Baloo;

class IndexerConfig::Private {
public:
    FileIndexerConfig m_config;
};

IndexerConfig::IndexerConfig()
    : d(new Private)
{
}

IndexerConfig::~IndexerConfig()
{
    delete d;
}

bool IndexerConfig::fileIndexingEnabled() const
{
    KConfig config(QLatin1String("baloofilerc"));
    KConfigGroup group = config.group("Basic Settings");
    return config.group("Basic Settings").readEntry("Indexing-Enabled", true);
}


void IndexerConfig::setFileIndexingEnabled(bool enabled) const
{
    KConfig config(QLatin1String("baloofilerc"));
    KConfigGroup basicSettings = config.group("Basic Settings");
    basicSettings.writeEntry("Indexing-Enabled", enabled);
}

bool IndexerConfig::shouldBeIndexed(const QString& path) const
{
    return d->m_config.shouldBeIndexed(path);
}

QStringList IndexerConfig::excludeFolders() const
{
    return d->m_config.excludeFolders();
}

QStringList IndexerConfig::includeFolders() const
{
    return d->m_config.includeFolders();
}

void IndexerConfig::setExcludeFolders(const QStringList& excludeFolders)
{
    KConfig config(QLatin1String("baloofilerc"));
    config.group("General").writePathEntry("exclude folders", excludeFolders);
}

void IndexerConfig::setIncludeFolders(const QStringList& includeFolders)
{
    KConfig config(QLatin1String("baloofilerc"));
    config.group("General").writePathEntry("folders", includeFolders);
}

bool IndexerConfig::firstRun() const
{
    return d->m_config.isInitialRun();
}

void IndexerConfig::setFirstRun(bool firstRun) const
{
    d->m_config.setInitialRun(firstRun);
}
