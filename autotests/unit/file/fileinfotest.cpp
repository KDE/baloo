/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
        QCOMPARE(info.mtime(), qinfo.lastModified().toSecsSinceEpoch());
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
        QCOMPARE(info.ctime(), qinfo.metadataChangeTime().toSecsSinceEpoch());
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
