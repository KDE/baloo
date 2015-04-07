/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "database.h"
#include "transaction.h"
#include "document.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class DatabaseBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void DatabaseBenchmark::test()
{
    /*
    QTime timer;
    timer.start();

    QTemporaryDir dir;
    Database db(dir.path());
    db.open();
    Transaction tr(db, Transaction::ReadWrite);

    QDateTime dt = QDateTime::currentDateTime();
    uint i = 1;
    QBENCHMARK {
        Document doc;
        doc.setId(i++);
        doc.addTerm("Mplain/text");

        QByteArray fileName = "file" + QByteArray::number(i);
        doc.addTerm(fileName);
        doc.addFileNameTerm("F" + fileName);

        QDateTime mod = dt.addDays(-1 * i);
        const QByteArray dtm = mod.toString(Qt::ISODate).toUtf8();

        doc.addBoolTerm(QByteArray("DT_M") + dtm);
        doc.addBoolTerm(QByteArray("DT_MY") + QByteArray::number(mod.date().year()));
        doc.addBoolTerm(QByteArray("DT_MM") + QByteArray::number(mod.date().month()));
        doc.addBoolTerm(QByteArray("DT_MD") + QByteArray::number(mod.date().day()));

        doc.setMTime(1);
        doc.setCTime(2);

        tr.addDocument(doc);
    }
    tr.commit();

    qDebug() << i << timer.elapsed();
    */
}

QTEST_MAIN(DatabaseBenchmark)

#include "databasebenchmark.moc"
