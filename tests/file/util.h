/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
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
