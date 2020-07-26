/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
