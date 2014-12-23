/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#ifndef METADATAMOVERTEST_H
#define METADATAMOVERTEST_H

#include "../database.h"
#include <QTemporaryDir>

namespace Baloo {

class MetadataMoverTest : public QObject
{
    Q_OBJECT
public:
    MetadataMoverTest(QObject* parent = 0);

private Q_SLOTS:

    void init();
    void cleanupTestCase();

    void testRemoveFile();
    void testMoveFile();
    void testMoveFolder();

private:
    uint insertUrl(const QString& url);

    Database* m_db;
    QTemporaryDir* m_tempDir;
};

}
#endif // METADATAMOVERTEST_H
