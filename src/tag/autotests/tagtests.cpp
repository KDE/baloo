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

#include "tagtests.h"

#include "tag.h"
#include "tagfetchjob.h"
#include "database.h"

#include "qtest_kde.h"
#include <KDebug>

#include <QDir>
#include <QTest>
#include <QSignalSpy>

#include <QSqlQuery>

TagTests::TagTests(QObject* parent)
    : QObject(parent)
    , m_db(0)
{
}

void TagTests::initTestCase()
{
    m_dbPath = m_tempDir.name() + QDir::separator() + QLatin1String("tagDB.sqlite");
}

void TagTests::cleanupTestCase()
{
    QFile::remove(m_dbPath);
}

void TagTests::init()
{
    delete m_db;
    cleanupTestCase();

    m_db = new Database(this);
    m_db->setPath(m_dbPath);
    m_db->init();
}

void TagTests::testTagFetchFromId()
{
    QStringList list;
    list << "TagA" << "TagB" << "TagC";

    foreach (const QString& tag, list) {
        QSqlQuery insertQ;
        insertQ.prepare("INSERT INTO tags (name) VALUES (?)");
        insertQ.addBindValue(tag);

        QVERIFY(insertQ.exec());
    }

    Tag tag(QByteArray("tag:2"));
    QVERIFY(tag.name().isEmpty());
    QCOMPARE(tag.id(), QByteArray("tag:2"));

    TagFetchJob* job = tag.fetch();
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemReceived(Item*)));
    QSignalSpy spy2(job, SIGNAL(tagReceived(Tag*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item*>(), &tag);

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);
    QCOMPARE(spy2.at(0).first().value<Tag*>(), &tag);

    QCOMPARE(tag.name(), QLatin1String("TagB"));
}

void TagTests::testTagFetchFromName()
{
    QStringList list;
    list << "TagA" << "TagB" << "TagC";

    foreach (const QString& tag, list) {
        QSqlQuery insertQ;
        insertQ.prepare("INSERT INTO tags (name) VALUES (?)");
        insertQ.addBindValue(tag);

        QVERIFY(insertQ.exec());
    }

    Tag tag(QLatin1String("TagC"));
    QCOMPARE(tag.name(), QLatin1String("TagC"));
    QVERIFY(tag.id().isEmpty());

    TagFetchJob* job = tag.fetch();
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemReceived(Item*)));
    QSignalSpy spy2(job, SIGNAL(tagReceived(Tag*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item*>(), &tag);

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);
    QCOMPARE(spy2.at(0).first().value<Tag*>(), &tag);

    QCOMPARE(tag.id(), QByteArray("tag:3"));
}

void TagTests::testTagFetchInvalid()
{
    Tag tag(QLatin1String("TagC"));
    TagFetchJob* job = tag.fetch();
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemReceived(Item*)));
    QSignalSpy spy2(job, SIGNAL(tagReceived(Tag*)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(!job->exec());

    QCOMPARE(spy1.size(), 0);
    QCOMPARE(spy2.size(), 0);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);

    QCOMPARE(job->error(), (int)TagFetchJob::Error_TagDoesNotExist);
    QVERIFY(tag.id().isEmpty());
}

void TagTests::testTagCreate()
{
    Tag tag(QLatin1String("TagA"));
    TagCreateJob* job = tag.create();
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemCreated(Item*)));
    QSignalSpy spy2(job, SIGNAL(tagCreated(Tag*)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item*>(), &tag);

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);
    QCOMPARE(spy2.at(0).first().value<Tag*>(), &tag);

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);

    QCOMPARE(tag.id(), QByteArray("tag:1"));
}

void TagTests::testTagCreate_duplicate()
{
    testTagCreate();

    Tag tag(QLatin1String("TagA"));
    TagCreateJob* job = tag.create();
    QVERIFY(job);

    QSignalSpy spy(job, SIGNAL(result(KJob*)));
    QVERIFY(!job->exec());

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), (int)TagCreateJob::Error_TagExists);

    QVERIFY(tag.id().isEmpty());
}



QTEST_KDEMAIN_CORE(TagTests)
