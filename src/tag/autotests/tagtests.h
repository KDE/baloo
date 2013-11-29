/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TAGTESTS_H
#define TAGTESTS_H

#include <QObject>
#include <QHash>
#include <KTempDir>

#include "connection.h"

class TagTests : public QObject
{
    Q_OBJECT
public:
    explicit TagTests(QObject* parent = 0);

private Q_SLOTS:
    void init();
    void initTestCase();
    void cleanupTestCase();

    void testTagFetchFromId();
    void testTagFetchFromName();
    void testTagFetchInvalid();

    void testTagCreate();
    void testTagCreate_duplicate();

    void testTagModify();
    void testTagModify_duplicate();

    void testTagRemove();
    void testTagRemove_notExists();

    void testTagRelationFetchFromTag();
    void testTagRelationFetchFromItem();

    void testTagRelationSaveJob();
    void testTagRelationSaveJob_duplicate();
    void testTagRelationSaveListJobs();

    void testTagRelationRemoveJob();
    void testTagRelationRemoveJob_notExists();

    void testTagStoreFetchAll();
private:
    KTempDir m_tempDir;
    QString m_dbPath;

    Baloo::Tags::Connection* m_con;

    void insertTags(const QStringList& tags);
    void insertRelations(const QHash<int, QByteArray>& relations);
};

#endif // TAGTESTS_H
