/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Vishesh Handa <handa.vish@gmail.com>
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

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

#ifndef INDEXCLEANER_H
#define INDEXCLEANER_H

#include <QtCore/QString>
#include <QtCore/QQueue>
#include <QtCore/QMutex>

#include <KJob>

namespace Baloo
{
class FileIndexerConfig;

class IndexCleaner : public KJob
{
    Q_OBJECT

public:
    IndexCleaner(QObject* parent = 0);

    virtual void start();

    /**
     * Construct a SPARQL filter which matches all URLs (variable ?url) that should
     * not be indexed according to the configured folders in \p cfg. This does not
     * take exclude filters into account and ignores hidden folders alltogether.
     */
    static QString constructExcludeFolderFilter(FileIndexerConfig* cfg);

    /**
     * Construct a SPARQL filter which matches all filenames (variable ?fn) that
     * match one of the exclude filters set in \p cfg.
     */
    static QString constructExcludeFiltersFilenameFilter(FileIndexerConfig* cfg);

    /**
     * Construct a SPARQL filter which matches all file URLs (variable ?url) that should
     * not be indexed according to the exclude filters. In contrast to the above method
     * the filter checks for matching exclude filters in path components instead of the
     * file name.
     * This method does take include folders into account but does not specifically handle
     * hidden folders.
     */
    static QString constructExcludeFiltersFolderFilter(FileIndexerConfig* cfg);

public Q_SLOTS:
    /**
     * Set the delay between the cleanup queries.
     * Used for throtteling the cleaner to not grab too
     * many resources. Default is 0.
     *
     * \sa IndexScheduler::setIndexingSpeed()
     */
    void setDelay(int msecs);

protected:
    virtual bool doSuspend();
    virtual bool doResume();

private Q_SLOTS:
    void clearNextBatch();
    void slotRemoveResourcesDone(KJob* job);

private:
    QQueue<QString> m_removalQueries;

    QString m_query;

    int m_delay;
};
}

#endif // INDEXCLEANER_H
