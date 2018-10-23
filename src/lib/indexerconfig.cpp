/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <vhanda@kde.org>
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
#include "../file/fileindexerconfig.h"
#include "../file/fileexcludefilters.h"
#include "../file/regexpcache.h"

#include <KConfig>
#include <KConfigGroup>

#include <QDBusConnection>
#include "maininterface.h"

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
    return d->m_config.indexingEnabled();
}


void IndexerConfig::setFileIndexingEnabled(bool enabled) const
{
    KConfig config(QStringLiteral("baloofilerc"));
    KConfigGroup basicSettings = config.group("Basic Settings");
    basicSettings.writeEntry("Indexing-Enabled", enabled);
}

bool IndexerConfig::shouldBeIndexed(const QString& path) const
{
    return d->m_config.shouldBeIndexed(path);
}


bool IndexerConfig::canBeSearched(const QString& folder) const
{
    return d->m_config.canBeSearched(folder);
}

QStringList IndexerConfig::excludeFolders() const
{
    return d->m_config.excludeFolders();
}

QStringList IndexerConfig::includeFolders() const
{
    return d->m_config.includeFolders();
}

QStringList IndexerConfig::excludeFilters() const
{
    return d->m_config.excludeFilters();
}

QStringList IndexerConfig::excludeMimetypes() const
{
    return d->m_config.excludeMimetypes();
}

void IndexerConfig::setExcludeFolders(const QStringList& excludeFolders)
{
    KConfig config(QStringLiteral("baloofilerc"));
    config.group("General").writePathEntry("exclude folders", excludeFolders);
}

void IndexerConfig::setIncludeFolders(const QStringList& includeFolders)
{
    KConfig config(QStringLiteral("baloofilerc"));
    config.group("General").writePathEntry("folders", includeFolders);
}

void IndexerConfig::setExcludeFilters(const QStringList& excludeFilters)
{
    KConfig config(QStringLiteral("baloofilerc"));
    config.group("General").writeEntry("exclude filters", excludeFilters);
}

void IndexerConfig::setExcludeMimetypes(const QStringList& excludeMimetypes)
{
    KConfig config(QStringLiteral("baloofilerc"));
    config.group("General").writeEntry("exclude mimetypes", excludeMimetypes);
}

bool IndexerConfig::firstRun() const
{
    return d->m_config.isInitialRun();
}

void IndexerConfig::setFirstRun(bool firstRun) const
{
    d->m_config.setInitialRun(firstRun);
}

bool IndexerConfig::indexHidden() const
{
    KConfig config(QStringLiteral("baloofilerc"));
    return config.group("General").readEntry("index hidden folders", false);
}

void IndexerConfig::setIndexHidden(bool value) const
{
    KConfig config(QStringLiteral("baloofilerc"));
    config.group("General").writeEntry("index hidden folders", value);
}

bool IndexerConfig::onlyBasicIndexing() const
{
    KConfig config(QStringLiteral("baloofilerc"));
    return config.group("General").readEntry("only basic indexing", false);
}

void IndexerConfig::setOnlyBasicIndexing(bool value)
{
    KConfig config(QStringLiteral("baloofilerc"));
    config.group("General").writeEntry("only basic indexing", value);
}

void IndexerConfig::refresh() const
{
    org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                QStringLiteral("/"),
                                                QDBusConnection::sessionBus());
    mainInterface.updateConfig();
}
