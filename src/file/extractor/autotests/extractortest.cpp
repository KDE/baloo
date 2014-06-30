/*
 * This file is part of the KDE Baloo Project
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

#include "extractortest.h"

#include <QTest>
#include <QProcess>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

#include "xapiandatabase.h"

void ExtractorTest::test()
{
    QTemporaryDir dbDir;
    dbDir.setAutoRemove(false);
    const QString fileUrl = dbDir.path() + QLatin1String("/testFile.txt");

    QFile file(fileUrl);
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << "Some words";
    file.close();

    QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor"));

    QStringList args;
    args << fileUrl << QLatin1String("--db") << dbDir.path() << QLatin1String("--ignoreConfig");

    QProcess process;
    process.start(exe, args);
    process.setProcessChannelMode(QProcess::MergedChannels);
    QVERIFY(process.waitForFinished(10000));

    Baloo::XapianDatabase xapDb(dbDir.path());
    Xapian::Database* db = xapDb.db();
    QCOMPARE((int)db->get_doccount(), 1);

    Xapian::Document doc = db->get_document(1);

    QStringList words;
    for (auto it = doc.termlist_begin(); it != doc.termlist_end(); it++) {
        std::string str = *it;
        words << QString::fromUtf8(str.c_str(), str.length());
    }

    QVERIFY(words.contains(QLatin1String("some")));
    QVERIFY(words.contains(QLatin1String("words")));
    QVERIFY(words.contains(QLatin1String("testfile")));
    QVERIFY(words.contains(QLatin1String("txt")));
    QVERIFY(words.contains(QLatin1String("Z2")));
}

void ExtractorTest::testBData()
{
    QTemporaryFile file;
    file.setFileName(QDir::tempPath() + QLatin1String("/baloo_extractor_autotest.txt"));
    QVERIFY(file.open());

    QTextStream stream(&file);
    stream << "Sample\nText";
    stream.flush();

    QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor"));

    QStringList args;
    args << QLatin1String("--bdata");
    args << file.fileName();

    QProcess process;
    process.start(exe, args);
    QVERIFY(process.waitForFinished(1000));

    QByteArray bytearray = QByteArray::fromBase64(process.readAllStandardOutput());
    QVariantMap data;
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in >> data;

    QCOMPARE(data.size(), 1);
    QCOMPARE(data.value(QLatin1String("lineCount")).toInt(), 2);
}

QTEST_MAIN(ExtractorTest)
