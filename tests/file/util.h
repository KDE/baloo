/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_FILE_TEST_UTIL_H
#define BALOO_FILE_TEST_UTIL_H

#include <QDebug>
#include <QFile>

inline void printIOUsage()
{
    // Print the io usage
    QFile file(QStringLiteral("/proc/self/io"));
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream fs(&file);
    QString fileContents = fs.readAll();

    qDebug() << "------- IO ---------";
    QTextStream stream(&fileContents);
    while (!stream.atEnd()) {
        QString str = stream.readLine();

        QString rchar(QStringLiteral("rchar: "));
        if (str.startsWith(rchar)) {
            ulong amt = str.midRef(rchar.size()).toULong();
            qDebug() << "Read:" << amt / 1024  << "kb";
        }

        QString wchar(QStringLiteral("wchar: "));
        if (str.startsWith(wchar)) {
            ulong amt = str.midRef(wchar.size()).toULong();
            qDebug() << "Write:" << amt / 1024  << "kb";
        }

        QString read(QStringLiteral("read_bytes: "));
        if (str.startsWith(read)) {
            ulong amt = str.midRef(read.size()).toULong();
            qDebug() << "Actual Reads:" << amt / 1024  << "kb";
        }

        QString write(QStringLiteral("write_bytes: "));
        if (str.startsWith(write)) {
            ulong amt = str.midRef(write.size()).toULong();
            qDebug() << "Actual Writes:" << amt / 1024  << "kb";
        }
    }
    qDebug() << "\nThe actual read/writes may be 0 because of an existing"
             << "cache and /tmp being memory mapped";
}

#endif
