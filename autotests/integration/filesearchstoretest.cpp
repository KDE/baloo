/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "filesearchstoretest.h"
#include "filesearchstore.h"
#include "../../file/database.h"
#include "filemapping.h"
#include "query.h"
#include "term.h"

#include <QtTest>

#include <xapian.h>
#include <kfilemetadata/properties.h>
#include <kfilemetadata/propertyinfo.h>


using namespace Baloo;

void FileSearchStoreTest::testSimpleSearchString()
{
    QBENCHMARK {
        QString url1(QLatin1String("/home/t/a"));
        quint64 id1 = insertUrl(url1);
        insertText(id1, QLatin1String("This is sample text"));

        QString url2(QLatin1String("/home/t/b"));
        quint64 id2 = insertUrl(url2);
        insertText(id2, QLatin1String("sample sample more sample text"));

        Query q;
        q.addType(QLatin1String("File"));
        q.setSearchString(QLatin1String("Sample"));

        int qid = m_store->exec(q);
        //QCOMPARE(qid, 1);
        QVERIFY(m_store->next(qid));
        QCOMPARE(m_store->id(qid), serialize("file", id2));
        QCOMPARE(m_store->filePath(qid), url2);

        QVERIFY(m_store->next(qid));
        QCOMPARE(m_store->id(qid), serialize("file", id1));
        QCOMPARE(m_store->filePath(qid), url1);

        QVERIFY(!m_store->next(qid));
        QVERIFY(m_store->id(qid).isEmpty());
        QVERIFY(m_store->filePath(qid).isEmpty());

        m_store->close(qid);
        cleanupTestCase();
        init();
    }
}

void FileSearchStoreTest::testPropertyValueEqual()
{
    QString url1(QLatin1String("/home/t/a"));
    quint64 id1 = insertUrl(url1);
    insertText(id1, QLatin1String("This is sample text"));

    QString url2(QLatin1String("/home/t/b"));
    quint64 id2 = insertUrl(url2);
    insertText(id2, QLatin1String("sample sample more sample but not text"));

    Query q;
    q.setTerm(Term(QString(), "Sample text", Baloo::Term::Equal));
    q.addType(QLatin1String("File"));

    int qid = m_store->exec(q);
    QCOMPARE(qid, 1);
    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", id1));
    QCOMPARE(m_store->filePath(qid), url1);

    QVERIFY(!m_store->next(qid));
    QVERIFY(m_store->id(qid).isEmpty());
    QVERIFY(m_store->filePath(qid).isEmpty());

    m_store->close(qid);
}

void FileSearchStoreTest::testIncludeDir()
{
    QString url1(QLatin1String("/home/t/a"));
    quint64 id1 = insertUrl(url1);
    insertText(id1, QLatin1String("This is sample text"));

    QString url2(QLatin1String("/home/t/b"));
    quint64 id2 = insertUrl(url2);
    insertText(id2, QLatin1String("sample sample more sample text"));

    QString url3(QLatin1String("/home/garden/b"));
    quint64 id3 = insertUrl(url3);
    insertText(id3, QLatin1String("The grass is green in the garden."));

    QString url4(QLatin1String("/home/tt/b"));
    quint64 id4 = insertUrl(url4);
    insertText(id4, QLatin1String("Let's see if this works."));

    QString url5(QLatin1String("/home/t/c"));
    quint64 id5 = insertUrl(url5);
    insertText(id5, QLatin1String("sample sample more sample text"));

    Query q;
    q.addType(QLatin1String("File"));
    q.setIncludeFolder(QStringLiteral("/home/t"));

    int qid = m_store->exec(q);
    QCOMPARE(qid, 1);
    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", id1));
    QCOMPARE(m_store->filePath(qid), url1);

    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", id2));
    QCOMPARE(m_store->filePath(qid), url2);

    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", id5));
    QCOMPARE(m_store->filePath(qid), url5);

    QVERIFY(!m_store->next(qid));
    QVERIFY(m_store->id(qid).isEmpty());
    QVERIFY(m_store->filePath(qid).isEmpty());

    m_store->close(qid);
}

void FileSearchStoreTest::testRatings()
{
    QString url1(QLatin1String("/home/t/a"));
    quint64 id1 = insertUrl(url1);
    insertRating(id1, 2);

    QString url2(QLatin1String("/home/t/b"));
    quint64 id2 = insertUrl(url2);
    insertRating(id2, 4);

    QString url3(QLatin1String("/home/garden/b"));
    quint64 id3 = insertUrl(url3);
    insertRating(id3, 6);

    QString url4(QLatin1String("/home/tt/b"));
    quint64 id4 = insertUrl(url4);
    insertRating(id4, 10);

    QString url5(QLatin1String("/home/tt/c"));
    quint64 id5 = insertUrl(url5);
    insertText(id5, QLatin1String("Test text"));

    //
    // Less than 5
    //
    Query q;
    q.addType(QLatin1String("File"));
    q.setTerm(Term(QLatin1String("rating"), 5, Term::Less));

    int qid1 = m_store->exec(q);
    QCOMPARE(qid1, 1);

    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id1));
    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id2));
    QVERIFY(!m_store->next(qid1));

    //
    // Greater than 6
    //
    q.setTerm(Term(QLatin1String("rating"), 6, Term::Greater));

    int qid2 = m_store->exec(q);
    QCOMPARE(qid2, 2);

    QVERIFY(m_store->next(qid2));
    QCOMPARE(m_store->id(qid2), serialize("file", id4));
    QVERIFY(!m_store->next(qid2));
}

void FileSearchStoreTest::testEmptySearchString()
{
    QString url1(QLatin1String("/home/t/a"));
    quint64 id1 = insertUrl(url1);
    insertText(id1, QLatin1String("File A"));

    QString url2(QLatin1String("/home/t/b"));
    quint64 id2 = insertUrl(url2);
    insertText(id2, QLatin1String("File B"));

    QString url3(QLatin1String("/home/garden/b"));
    quint64 id3 = insertUrl(url3);
    insertText(id3, QLatin1String("Garden B"));

    QString url4(QLatin1String("/home/tt/b"));
    quint64 id4 = insertUrl(url4);
    insertText(id4, QLatin1String("TT B"));

    QString url5(QLatin1String("/home/tt/c"));
    quint64 id5 = insertUrl(url5);
    insertText(id5, QLatin1String("TT C"));

    Query q;
    q.addType(QLatin1String("File"));

    int qid1 = m_store->exec(q);
    QCOMPARE(qid1, 1);

    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id1));
    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id2));
    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id3));
    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id4));
    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id5));
    QVERIFY(!m_store->next(qid1));
}

void FileSearchStoreTest::testAllVideos()
{
    QString url1(QLatin1String("/home/t/a"));
    quint64 id1 = insertUrl(url1);
    insertType(id1, QLatin1String("Video"));

    QString url2(QLatin1String("/home/t/b"));
    quint64 id2 = insertUrl(url2);
    insertType(id2, QLatin1String("Image"));

    QString url3(QLatin1String("/home/garden/b"));
    quint64 id3 = insertUrl(url3);
    insertType(id3, QLatin1String("Video"));

    Query q;
    q.addType(QLatin1String("Video"));

    int qid1 = m_store->exec(q);
    QCOMPARE(qid1, 1);

    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id1));
    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id3));
    QVERIFY(!m_store->next(qid1));
}

void FileSearchStoreTest::testFileNameSearch()
{
    QString url1(QLatin1String("/home/t/a"));
    quint64 id1 = insertUrl(url1);
    insertExactText(id1, QLatin1String("flowering"), "F");
    insertExactText(id1, QLatin1String("dork"), "F");
    insertExactText(id1, QLatin1String("dork"), "A");
    insertExactText(id1, QLatin1String("dork"), "G");

    QString url2(QLatin1String("/home/t/b"));
    quint64 id2 = insertUrl(url2);
    insertExactText(id2, QLatin1String("powering"), "F");
    insertExactText(id2, QLatin1String("fire"), "F");
    insertExactText(id2, QLatin1String("dork"), "A");
    insertExactText(id2, QLatin1String("dork"), "G");

    QString url3(QLatin1String("/home/garden/b"));
    quint64 id3 = insertUrl(url3);
    insertExactText(id3, QLatin1String("does"), "F");
    insertExactText(id3, QLatin1String("not"), "F");
    insertExactText(id3, QLatin1String("dork"), "A");
    insertExactText(id3, QLatin1String("dork"), "G");

    Query q;
    q.addType(QLatin1String("File"));
    q.setTerm(Term("filename", "dork"));

    int qid1 = m_store->exec(q);
    QCOMPARE(qid1, 1);
    QVERIFY(m_store->next(qid1));
    QCOMPARE(m_store->id(qid1), serialize("file", id1));
    QVERIFY(!m_store->next(qid1));

    q.setTerm(Term("filename", "do*"));

    int qid2 = m_store->exec(q);
    QCOMPARE(qid2, 2);
    QVERIFY(m_store->next(qid2));
    QCOMPARE(m_store->id(qid2), serialize("file", id1));
    QVERIFY(m_store->next(qid2));
    QCOMPARE(m_store->id(qid2), serialize("file", id3));
    QVERIFY(!m_store->next(qid2));

    q.setTerm(Term("filename", "*ing"));

    int qid3 = m_store->exec(q);
    QCOMPARE(qid3, 3);
    QVERIFY(m_store->next(qid3));
    QCOMPARE(m_store->id(qid3), serialize("file", id1));
    QVERIFY(m_store->next(qid3));
    QCOMPARE(m_store->id(qid3), serialize("file", id2));
    QVERIFY(!m_store->next(qid3));

    // Exact match
    q.setTerm(Term("filename", "dork", Term::Equal));

    int qid4 = m_store->exec(q);
    QCOMPARE(qid4, 4);
    QVERIFY(m_store->next(qid4));
    QCOMPARE(m_store->id(qid4), serialize("file", id1));
    QVERIFY(!m_store->next(qid4));
}

void FileSearchStoreTest::testSortingNone()
{
    insertText(1, QLatin1String("Power A"));
    insertText(2, QLatin1String("Power Power B"));

    Query q;
    q.addType(QLatin1String("File"));
    q.setSearchString("Power");

    int qid;

    // Auto sort - Based on frequency
    qid = m_store->exec(q);
    QCOMPARE(qid, 1);
    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 2));
    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 1));
    QVERIFY(!m_store->next(qid));

    // no sort
    q.setSortingOption(Query::SortNone);

    qid = m_store->exec(q);
    QCOMPARE(qid, 2);
    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 1));
    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 2));
    QVERIFY(!m_store->next(qid));
}

void FileSearchStoreTest::testInvalidProperties()
{
    Query q;
    q.addType(QLatin1String("File"));
    q.setTerm(Term("NoNExistingProperty", QVariant(1)));

    int qid = m_store->exec(q);
    QCOMPARE(qid, 0);
    QVERIFY(!m_store->next(qid));
}

void FileSearchStoreTest::testModifiedProperty()
{
    QDateTime dt(QDate(2013, 12, 02), QTime(12, 2, 2));
    insertExactText(1, dt.toString(Qt::ISODate), "DT_M");
    insertExactText(2, QDateTime::currentDateTime().toString(Qt::ISODate), "DT_M");

    Query q;
    q.addType(QLatin1String("File"));
    q.setTerm(Term(("modified"), dt.date(), Term::Equal));

    int qid = m_store->exec(q);

    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 1));
    QVERIFY(!m_store->next(qid));

    q.setTerm(Term(("modified"), dt, Term::Equal));

    qid = m_store->exec(q);

    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 1));
    QVERIFY(!m_store->next(qid));
}

void FileSearchStoreTest::testDateTimeProperty()
{
    KFileMetaData::PropertyInfo propInfo(KFileMetaData::Property::CreationDate);
    QString prefix = "X" + QString::number((int)propInfo.property());

    QDateTime dt(QDate(2013, 12, 02), QTime(12, 2, 2));
    insertExactText(1, dt.toString(Qt::ISODate), prefix);
    insertExactText(2, QDateTime::currentDateTime().toString(Qt::ISODate), prefix);

    Query q;
    q.addType(QLatin1String("File"));
    q.setTerm(Term(propInfo.name(), dt.date(), Term::Equal));

    int qid = m_store->exec(q);

    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 1));
    QVERIFY(!m_store->next(qid));

    q.setTerm(Term((propInfo.name()), dt, Term::Equal));

    qid = m_store->exec(q);

    QVERIFY(m_store->next(qid));
    QCOMPARE(m_store->id(qid), serialize("file", 1));
    QVERIFY(!m_store->next(qid));
}

QTEST_MAIN(Baloo::FileSearchStoreTest)
