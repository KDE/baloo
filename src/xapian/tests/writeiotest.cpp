
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

#include <QDebug>
#include <QTemporaryDir>
#include <QTime>
#include <QCoreApplication>

#include "xapiandatabase.h"
#include "xapiandocument.h"
#include "../../file/tests/util.h"

int main(int argc, char** argv)
{
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);

    Baloo::XapianDatabase db(tempDir.path(), true);
    Baloo::XapianDocument doc;

    for (int i=1000; i<3000; i++) {
        doc.indexText("A" + QString::number(i));
    }
    db.replaceDocument(1, doc);
    db.commit();
    printIOUsage();

    for (int i=1000; i<4000; i++) {
        doc.indexText("A" + QString::number(i));
    }
    db.replaceDocument(2, doc);
    db.commit();

    qDebug() << tempDir.path();
    printIOUsage();

    return 0;
}
