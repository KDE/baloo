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

#ifndef BALOO_FILESEARCHSTORETEST_H
#define BALOO_FILESEARCHSTORETEST_H

#include <QObject>
#include <KTempDir>

class Database;

namespace Baloo {

class FileSearchStore;

class FileSearchStoreTest : public QObject
{
    Q_OBJECT
public:
    explicit FileSearchStoreTest(QObject* parent = 0);

private Q_SLOTS:
    void init();
    void initTestCase();
    void cleanupTestCase();

    void testSimpleSearchString();

private:
    KTempDir* m_tempDir;
    Database* m_db;
    FileSearchStore* m_store;

    uint insertUrl(const QString& url);
    void insertText(int id, const QString& text);
};

}

#endif // BALOO_FILESEARCHSTORETEST_H
