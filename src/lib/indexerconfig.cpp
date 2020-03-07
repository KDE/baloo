/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <vhanda@kde.org>
 * Copyright (C) 2020  Benjamin Port <benjamin.port@enioka.com>
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

#include <QDBusConnection>
#include "maininterface.h"
#include "baloosettings.h"

using namespace Baloo;

class IndexerConfig::Private {
public:
    FileIndexerConfig m_config;
    BalooSettings m_settings;
};

IndexerConfig::IndexerConfig()
    : d(new Private)
{
}

IndexerConfig::~IndexerConfig()
{
    d->m_settings.save();
    delete d;
}

bool IndexerConfig::fileIndexingEnabled() const
{
    return d->m_settings.indexingEnabled();
}


void IndexerConfig::setFileIndexingEnabled(bool enabled) const
{
    d->m_settings.setIndexingEnabled(enabled);
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
    d->m_settings.setExcludedFolders(excludeFolders);
}

void IndexerConfig::setIncludeFolders(const QStringList& includeFolders)
{
    d->m_settings.setFolders(includeFolders);
}

void IndexerConfig::setExcludeFilters(const QStringList& excludeFilters)
{
    d->m_settings.setExcludedFilters(excludeFilters);
}

void IndexerConfig::setExcludeMimetypes(const QStringList& excludeMimetypes)
{
    d->m_settings.setExcludedMimetypes(excludeMimetypes);
}

#if BALOO_CORE_BUILD_DEPRECATED_SINCE(5, 69)
bool IndexerConfig::firstRun() const
{
    return false;
}

void IndexerConfig::setFirstRun(bool) const
{
}
#endif

bool IndexerConfig::indexHidden() const
{
    return d->m_settings.indexHiddenFolders();
}

void IndexerConfig::setIndexHidden(bool value) const
{
    d->m_settings.setIndexHiddenFolders(value);
}

bool IndexerConfig::onlyBasicIndexing() const
{
    return d->m_settings.onlyBasicIndexing();
}

void IndexerConfig::setOnlyBasicIndexing(bool value)
{
    d->m_settings.setOnlyBasicIndexing(value);
}

void IndexerConfig::refresh() const
{
    org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                QStringLiteral("/"),
                                                QDBusConnection::sessionBus());
    mainInterface.updateConfig();
}
