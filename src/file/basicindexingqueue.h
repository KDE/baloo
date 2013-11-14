/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef BASICINDEXINGQUEUE_H
#define BASICINDEXINGQUEUE_H

#include "indexingqueue.h"
#include "filemapping.h"

#include <KJob>
#include <QStack>

class Database;

namespace Baloo
{

enum UpdateDirFlag {
    /**
     * No flags, only used to make code more readable
     */
    NoUpdateFlags = 0x0,

    /**
     * The folder should be updated recursive
     */
    UpdateRecursive = 0x1,

    /**
     * The folder has been scheduled to update by the
     * update system, not by a call to updateDir
     */
    AutoUpdateFolder = 0x2,

    /**
     * The files in the folder should be updated regardless
     * of their state.
     */
    ForceUpdate = 0x4
};
Q_DECLARE_FLAGS(UpdateDirFlags, UpdateDirFlag)

/**
 * This class represents a simple queue that iterates over the file system tree
 * and indexes each file which meets certain critera. The indexing performed by this
 * queue is very basic. It just pushes the mimetype, url and stat results of the file.
 */
class BasicIndexingQueue: public IndexingQueue
{
    Q_OBJECT
public:
    explicit BasicIndexingQueue(Database* db, QObject* parent = 0);

    QString currentUrl() const;
    UpdateDirFlags currentFlags() const;

    virtual bool isEmpty();

Q_SIGNALS:
    void beginIndexingFile(const QString& url);
    void endIndexingFile(const QString& url);

public Q_SLOTS:
    void enqueue(const QString& path);
    void enqueue(const QString& path, UpdateDirFlags flags);

    void clear();
    void clear(const QString& path);

protected:
    virtual void processNextIteration();

private Q_SLOTS:
    void slotIndexingFinished(KJob* job);

private:
    /**
     * This method does not need to be synchronous. The indexing operation may be started
     * and on completion, the finishedIndexing method should be called
     */
    void index(const QString& path);

    bool shouldIndex(const QString& path, const QString& mimetype);
    bool shouldIndexContents(const QString& dir);

    /**
     * Check if the \p path needs to be indexed based on the \p flags
     * and the path. If it needs to be indexed, then start indexing
     * it.
     *
     * \return \c true the path is being indexed
     * \return \c false the path did not meet the criteria
     */
    bool process(const QString& path, UpdateDirFlags flags);

    QStack< QPair<QString, UpdateDirFlags> > m_paths;

    FileMapping m_currentFile;
    QString m_currentMimeType;

    UpdateDirFlags m_currentFlags;

    Database* m_db;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Baloo::UpdateDirFlags)

#endif // BASICINDEXINGQUEUE_H
