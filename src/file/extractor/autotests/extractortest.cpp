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
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>

#include "xapiandatabase.h"

void ExtractorTest::test()
{
    QTemporaryDir dbDir;
    dbDir.setAutoRemove(false);
    const QString fileUrl = dbDir.path() + "/testFile.txt";

    QFile file(fileUrl);
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << "Some words";
    file.close();

    QString exe = QStandardPaths::findExecutable("baloo_file_extractor");

    QStringList args;
    args << fileUrl << "--db" << dbDir.path() << "--ignoreConfig";

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

    QVERIFY(words.contains("some"));
    QVERIFY(words.contains("words"));
    QVERIFY(words.contains("testfile"));
    QVERIFY(words.contains("txt"));
    QVERIFY(words.contains("Z2"));
}

QTEST_MAIN(ExtractorTest)

#include "extractortest.moc"
