/*
 * <one line to give the library's name and an idea of what it does.>
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

#ifndef FILEMONITORTEST_H
#define FILEMONITORTEST_H

#include <QObject>
#include "filemonitor.h"
namespace Baloo {

class FileMonitorTest : public QObject
{
    Q_OBJECT


private:
    QString getRandomValidFilePath();
    QString getRandomValidWebUrl();
    QString getRandomString(int length) const;
    FileMonitor* m_sut;
private Q_SLOTS:
    void test();
    void init();
    void cleanup();
    void testAddFileShouldReturnOneFileIfOneFileAdded();
    void testAddFileShouldReturnTwoFilesIfTwoFilesAdded();
    void testAddFileShouldRemoveTailingSlash();
    void testAddFileShouldNotAddNotLocalUrl();
    void testAddFileShouldAddLocalUrl();
    void testClearIfClearAfterOneFileAddedFilesShouldReturn0Items();
    void testSetFilesIfSetFilesWithOneElementFilesShouldReturn1Item();
};

}

#endif // FILEMONITORTEST_H
