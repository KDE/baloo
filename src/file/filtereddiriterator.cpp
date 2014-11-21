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

#include "filtereddiriterator.h"
#include "fileindexerconfig.h"

#include <QFileInfo>
#include <QDebug>

using namespace Baloo;

FilteredDirIterator::FilteredDirIterator(FileIndexerConfig* config, const QString& folder, Filter filter)
    : m_config(config)
    , m_filters(QDir::NoDotAndDotDot | QDir::Readable | QDir::NoSymLinks)
{
    if (filter == Dirs) {
        m_filters |= QDir::Dirs;
    } else if (filter == Files) {
        m_filters |= (QDir::Files | QDir::Dirs);
    }

    QDirIterator* it = new QDirIterator(folder, m_filters);
    m_currentIter = it;
    m_firstItem = true;
}

QString FilteredDirIterator::next()
{
    if (m_firstItem) {
        m_firstItem = false;
        m_filePath = m_currentIter->path();
        return m_filePath;
    }

    m_filePath.clear();
    if (!m_currentIter) {
        return QString();
    }

    while (!m_currentIter->hasNext()) {
        delete m_currentIter;
        m_currentIter = 0;

        if (!m_iterators.isEmpty()) {
            m_currentIter = m_iterators.pop();
        } else {
            return QString();
        }
    }

    // vHanda: We do not really need this QFileInfo, we can just use the filters
    m_filePath = m_currentIter->next();
    if (m_filePath.endsWith('.')) {
        m_filePath = m_filePath.mid(0, m_filePath.length() - 2);
    }
    QFileInfo info(m_filePath);

    bool runAgain = false;
    if (info.isDir()) {
        if (m_config->shouldFolderBeIndexed(m_filePath)) {
            QDirIterator* it = new QDirIterator(m_filePath, m_filters);
            m_iterators.push(it);
        } else {
            runAgain = true;
        }
    }
    else if (info.isFile()) {
        runAgain = !m_config->shouldBeIndexed(m_filePath);
    }

    if (runAgain) {
        return next();
    }

    return m_filePath;
}

QString FilteredDirIterator::filePath() const
{
    return m_filePath;
}


