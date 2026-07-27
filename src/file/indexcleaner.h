/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_INDEXCLEANER_H
#define BALOO_INDEXCLEANER_H

#include <QRunnable>
#include <QObject>

namespace Baloo {

class Database;
class FileIndexerConfig;

class IndexCleaner : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /**
     * What to do about the hidden files and folders that are in the index.
     */
    enum class HiddenFiles {
        /**
         * Leave them where they are. They are either wanted in the index, or they are
         * already gone from it.
         */
        LeaveAlone,
        /**
         * Take them out of the index, as the user has just asked for hidden files to be
         * left out. This walks every document below the included folders, so ask for it
         * when the setting has changed rather than on every run.
         */
        RemoveFromIndex,
    };

    IndexCleaner(Database *db, FileIndexerConfig *config, HiddenFiles hiddenFiles = HiddenFiles::LeaveAlone);
    void run() override;

Q_SIGNALS:
    void done();

private:
    Database* m_db;
    FileIndexerConfig* m_config;
    HiddenFiles m_hiddenFiles;
};
}

#endif // BALOO_INDEXCLEANER_H
