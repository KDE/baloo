/*
 * <one line to give the library's name and an idea of what it does.>
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
#include <QTextStream>
#include <QTemporaryFile>
#include <QDebug>
#include <QDir>

#include <KStandardDirs>

void ExtractorTest::testBData()
{
    QTemporaryFile file;
    file.setFileName(QDir::tempPath() + QLatin1String("/baloo_extractor_autotest.txt"));
    QVERIFY(file.open());

    QTextStream stream(&file);
    stream << "Sample\nText";
    stream.flush();

    static const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file_extractor"));

    QStringList args;
    args << QLatin1String("--bdata");
    args << file.fileName();

    QProcess process;
    process.start(exe, args);
    QVERIFY(process.waitForFinished(1000));

    qDebug() << process.readAllStandardError();
    QByteArray bytearray = QByteArray::fromBase64(process.readAllStandardOutput());
    QVariantMap data;
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in >> data;

    QCOMPARE(data.size(), 1);
    QCOMPARE(data.value(QLatin1String("lineCount")).toInt(), 2);
}


QTEST_MAIN(ExtractorTest)
