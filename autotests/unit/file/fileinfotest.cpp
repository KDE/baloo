/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2015 Vishesh Handa <vhanda@kde.org>

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

#include "fileinfo.h"

#include <QFileInfo>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QTest>

using namespace Baloo;

class FileInfoTest : public QObject
{
    Q_OBJECT

    static void touchFile(const QString& path)
    {
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write("data");
        file.close();
    }

private Q_SLOTS:
    void init()
    {
        file.setFileTemplate(QStringLiteral("baloo-fileinfo-test"));
        file.open();
    }

    void test_mtime()
    {
        FileInfo info(file.fileName().toUtf8());
        QFileInfo qinfo(file.fileName());
        QCOMPARE(info.mtime(), qinfo.lastModified().toTime_t());
    }

    void test_mtime_invalid()
    {
        FileInfo info("/bah/tmp/does-no-exist");
        QCOMPARE(info.mtime(), static_cast<quint32>(0));
    }

    void test_ctime()
    {
        FileInfo info(file.fileName().toUtf8());
        QFileInfo qinfo(file.fileName());
        QCOMPARE(info.ctime(), qinfo.created().toTime_t());
    }

    void test_exists_true()
    {
        FileInfo info(file.fileName().toUtf8());
        QCOMPARE(info.exists(), true);
    }

    void test_exists_false()
    {
        FileInfo info("/bah/tmp/does-no-exist");
        QCOMPARE(info.exists(), false);
    }

    void test_fileName()
    {
        QString path = dir.path() + "/test";
        touchFile(path);
        FileInfo info(path.toUtf8());
        QCOMPARE(info.fileName(), QByteArray("test"));
    }

    void test_hidden_false()
    {
        FileInfo info(file.fileName().toUtf8());
        QCOMPARE(info.isHidden(), false);
    }

    void test_hidden_true()
    {
        QString path = dir.path() + "/.test";
        touchFile(path);
        FileInfo info(path.toUtf8());
        QCOMPARE(info.isHidden(), true);
    }

    void test_isDir_true()
    {
        FileInfo info(dir.path().toUtf8());
        QCOMPARE(info.isDir(), true);
    }

    void test_isDir_false()
    {
        FileInfo info(file.fileName().toUtf8());
        QCOMPARE(info.isDir(), false);
    }

    void test_isFile_true()
    {
        FileInfo info(file.fileName().toUtf8());
        QCOMPARE(info.isFile(), true);
    }

    void test_isFile_false()
    {
        FileInfo info(dir.path().toUtf8());
        QCOMPARE(info.isFile(), false);
    }
private:
    QTemporaryFile file;
    QTemporaryDir dir;
};


QTEST_GUILESS_MAIN(FileInfoTest)

#include "fileinfotest.moc"
