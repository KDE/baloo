/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "database.h"
#include "databasetests.h"

#include "qtest_kde.h"
#include <KDebug>

#include <QDir>
#include <QTest>
#include <QSignalSpy>

DatabaseTests::DatabaseTests(QObject* parent)
    : QObject(parent)
    , m_db(0)
    , m_tempDir(0)
{
}

void DatabaseTests::initTestCase()
{
}

void DatabaseTests::cleanupTestCase()
{
    delete m_db;
    delete m_tempDir;
}

void DatabaseTests::init()
{
    cleanupTestCase();

    m_db = new Database();
    m_tempDir = new KTempDir();
    m_db->setPath(m_tempDir->name());
}

void DatabaseTests::testSet()
{
    m_db->beginDocument(1);
    m_db->set("email", QByteArray("abc@xyz.com"));
    m_db->setText("subject", QByteArray("Booga is the title"));
    m_db->endDocument();
    m_db->commit();

    {
        QString termDbName = m_tempDir->name() + "subject";
        QVERIFY(QFile::exists(termDbName));

        Xapian::Database termDb(termDbName.toStdString());

        QCOMPARE(termDb.get_doccount(), (unsigned)1);
        QVERIFY(!termDb.has_positions());

        Xapian::TermIterator iter = termDb.termlist_begin(1);
        QCOMPARE((*iter).c_str(), "booga"); iter++;
        QCOMPARE((*iter).c_str(), "is"); iter++;
        QCOMPARE((*iter).c_str(), "the"); iter++;
        QCOMPARE((*iter).c_str(), "title"); iter++;
        QCOMPARE(iter, termDb.termlist_end(1));

        Xapian::Document doc = termDb.get_document(1);
        QVERIFY(doc.get_data().empty());
    }

    {
        QString termDbName = m_tempDir->name() + "email";
        QVERIFY(QFile::exists(termDbName));

        Xapian::Database termDb(termDbName.toStdString());
        QVERIFY(!termDb.has_positions());
        QCOMPARE(termDb.get_doccount(), (unsigned)1);

        Xapian::TermIterator iter = termDb.termlist_begin(1);
        QCOMPARE((*iter).c_str(), "abc@xyz.com"); iter++;
        QCOMPARE(iter, termDb.termlist_end(1));

        Xapian::Document doc = termDb.get_document(1);
        QVERIFY(doc.get_data().empty());
    }
}

void DatabaseTests::testAppend()
{
    m_db->beginDocument(1);
    m_db->append("subject", QLatin1String("abc@xyz.com"));
    m_db->appendText("subject", QLatin1String("Booga is"));
    m_db->appendText("subject", QLatin1String("the title"));
    m_db->appendBool("subject", "isRead", true);
    m_db->appendBool("subject", "isSpam", false);
    m_db->endDocument();
    m_db->commit();

    {
        QString termDbName = m_tempDir->name() + "subject";
        QVERIFY(QFile::exists(termDbName));

        Xapian::Database termDb(termDbName.toStdString());
        QVERIFY(!termDb.has_positions());
        QCOMPARE(termDb.get_doccount(), (unsigned)1);

        Xapian::TermIterator iter = termDb.termlist_begin(1);
        QCOMPARE((*iter).c_str(), "XisRead"); iter++;
        QCOMPARE((*iter).c_str(), "abc@xyz.com"); iter++;
        QCOMPARE((*iter).c_str(), "booga"); iter++;
        QCOMPARE((*iter).c_str(), "is"); iter++;
        QCOMPARE((*iter).c_str(), "the"); iter++;
        QCOMPARE((*iter).c_str(), "title"); iter++;
        QCOMPARE(iter, termDb.termlist_end(1));

        Xapian::Document doc = termDb.get_document(1);
        QVERIFY(doc.get_data().empty());
    }
}


QTEST_KDEMAIN_CORE(DatabaseTests)
