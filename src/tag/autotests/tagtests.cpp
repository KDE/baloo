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
#include "tagrelation.h"
#include "tagstore.h"
#include "connection_p.h"

#include "qtest_kde.h"
#include <KDebug>

#include <QDir>
#include <QTest>
#include <QSignalSpy>

#include <QSqlQuery>
#include <QSqlError>

using namespace Baloo;

TagTests::TagTests(QObject* parent)
    : QObject(parent)
{
}

void TagTests::initTestCase()
{
    m_dbPath = m_tempDir.name() + QLatin1String("tagDB.sqlite");

    qRegisterMetaType<Item>();
    qRegisterMetaType<Tag>();
    qRegisterMetaType<Relation>();
    qRegisterMetaType<TagRelation>();
    qRegisterMetaType<KJob*>();
}

void TagTests::cleanupTestCase()
{
    QFile::remove(m_dbPath);
}

void TagTests::init()
{
    delete m_con;
    cleanupTestCase();

    m_con = new Baloo::Tags::Connection(new Baloo::Tags::ConnectionPrivate(m_dbPath));
}

void TagTests::insertTags(const QStringList& tags)
{
    foreach (const QString& tag, tags) {
        QSqlQuery insertQ(m_con->d->db());
        insertQ.prepare("INSERT INTO tags (name) VALUES (?)");
        insertQ.addBindValue(tag);

        QVERIFY(insertQ.exec());
    }
}

void TagTests::insertRelations(const QHash< int, QString >& relations)
{
    QHash< int, QString >::const_iterator iter = relations.constBegin();
    for (; iter != relations.constEnd(); iter++) {
        QSqlQuery insertQ(m_con->d->db());
        insertQ.prepare("INSERT INTO tagRelations (tid, rid) VALUES (?, ?)");
        insertQ.addBindValue(iter.key());
        insertQ.addBindValue(iter.value().toUtf8());

        QVERIFY(insertQ.exec());
    }
}


void TagTests::testTagFetchFromId()
{
    insertTags(QStringList() << "TagA" << "TagB" << "TagC");

    Tag tag = Tag::fromId("tag:2");
    QVERIFY(tag.name().isEmpty());
    QCOMPARE(tag.id(), QByteArray("tag:2"));

    TagFetchJob* job = new TagFetchJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemReceived(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagReceived(Baloo::Tag)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item>().id(), tag.id());

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);

    Tag rTag = spy2.at(0).first().value<Tag>();
    QCOMPARE(rTag.id(), tag.id());
    QCOMPARE(rTag.name(), QLatin1String("TagB"));
}

void TagTests::testTagFetchFromName()
{
    insertTags(QStringList() << "TagA" << "TagB" << "TagC");

    Tag tag(QLatin1String("TagC"));
    QCOMPARE(tag.name(), QLatin1String("TagC"));
    QVERIFY(tag.id().isEmpty());

    TagFetchJob* job = new TagFetchJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemReceived(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagReceived(Baloo::Tag)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item>().id(), QByteArray("tag:3"));

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);

    Tag rTag = spy2.at(0).first().value<Tag>();
    QCOMPARE(rTag.name(), tag.name());
    QCOMPARE(rTag.id(), QByteArray("tag:3"));
}

void TagTests::testTagFetchInvalid()
{
    Tag tag(QLatin1String("TagC"));
    TagFetchJob* job = new TagFetchJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemReceived(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagReceived(Baloo::Tag)));
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
    TagCreateJob* job = new TagCreateJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemCreated(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagCreated(Baloo::Tag)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item>().id(), QByteArray("tag:1"));

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);

    Tag rTag = spy2.at(0).first().value<Tag>();
    QCOMPARE(rTag.name(), tag.name());
    QCOMPARE(rTag.id(), QByteArray("tag:1"));

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);
}

void TagTests::testTagCreate_duplicate()
{
    testTagCreate();

    Tag tag(QLatin1String("TagA"));
    TagCreateJob* job = new TagCreateJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy(job, SIGNAL(result(KJob*)));
    QVERIFY(!job->exec());

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), (int)TagCreateJob::Error_TagExists);

    QVERIFY(tag.id().isEmpty());
}

void TagTests::testTagModify()
{
    Tag tag(QLatin1String("TagA"));
    TagCreateJob* cjob = new TagCreateJob(tag, m_con);

    QSignalSpy spy(cjob, SIGNAL(tagCreated(Baloo::Tag)));
    cjob->exec();

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    tag = spy.at(0).first().value<Tag>();

    QByteArray id = tag.id();
    tag.setName("TagB");

    TagSaveJob* job = new TagSaveJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemSaved(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagSaved(Baloo::Tag)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item>().id(), tag.id());

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);

    Tag rTag = spy2.at(0).first().value<Tag>();
    QCOMPARE(rTag.name(), QLatin1String("TagB"));
    QCOMPARE(rTag.id(), tag.id());

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);
}

void TagTests::testTagModify_duplicate()
{
    insertTags(QStringList() << "TagB");

    Tag tag(QLatin1String("TagA"));
    TagCreateJob* cjob = new TagCreateJob(tag, m_con);

    QSignalSpy spy(cjob, SIGNAL(tagCreated(Baloo::Tag)));
    cjob->exec();

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    tag = spy.at(0).first().value<Tag>();

    QByteArray id = tag.id();
    tag.setName("TagB");

    TagSaveJob* job = new TagSaveJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemSaved(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagSaved(Baloo::Tag)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(!job->exec());

    QCOMPARE(spy1.size(), 0);
    QCOMPARE(spy2.size(), 0);

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), (int)TagSaveJob::Error_TagExists);
}

void TagTests::testTagRemove()
{
    insertTags(QStringList() << "TagA" << "TagB" << "TagC");

    Tag tag = Tag::fromId(QByteArray("tag:1"));
    TagRemoveJob* job = new TagRemoveJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemRemoved(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagRemoved(Baloo::Tag)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(0).first().value<Item>().id(), QByteArray("tag:1"));

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);

    Tag rTag = spy2.at(0).first().value<Tag>();
    QCOMPARE(rTag.id(), QByteArray("tag:1"));
    QVERIFY(rTag.name().isEmpty());

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);

    QVERIFY(tag.name().isEmpty());

    QSqlQuery query(m_con->d->db());
    query.prepare("SELECT name from tags where id = ?");
    query.addBindValue(1);
    QVERIFY(query.exec());
    QVERIFY(!query.next());
}

void TagTests::testTagRemove_notExists()
{
    Tag tag = Tag::fromId(QByteArray("tag:1"));
    TagRemoveJob* job = new TagRemoveJob(tag, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(itemRemoved(Baloo::Item)));
    QSignalSpy spy2(job, SIGNAL(tagRemoved(Baloo::Tag)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(!job->exec());

    QCOMPARE(spy1.size(), 0);
    QCOMPARE(spy2.size(), 0);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), (int)TagRemoveJob::Error_TagDoesNotExist);
}


void TagTests::testTagRelationFetchFromTag()
{
    insertTags(QStringList() << "TagA");

    QHash<int, QString> rel;
    rel.insert(1, "file:1");
    insertRelations(rel);

    Tag tag(QLatin1String("TagA"));

    TagRelation tagRel(tag);
    TagRelationFetchJob* job = new TagRelationFetchJob(tagRel, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(relationReceived(Baloo::Relation)));
    QSignalSpy spy2(job, SIGNAL(tagRelationReceived(Baloo::TagRelation)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    /*
     * FIXME!!
    QCOMPARE(spy1.at(0).first().value<Relation>().from().id(), QByteArray()
    QCOMPARE(spy1.at(0).first().value<Relation>().to().id(), &tagRel);
    */

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);
    TagRelation tagRel2 = spy2.at(0).first().value<TagRelation>();

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);

    QCOMPARE(tagRel2.item().id(), QByteArray("file:1"));
    QCOMPARE(tagRel2.tag().name(), QLatin1String("TagA"));
    QCOMPARE(tagRel2.tag().id(), QByteArray("tag:1"));
}

void TagTests::testTagRelationFetchFromItem()
{
    insertTags(QStringList() << "TagA" << "TagB");

    QMultiHash<int, QString> rel;
    rel.insert(1, "file:1");
    rel.insert(2, "file:1");
    rel.insert(1, "file:3");
    insertRelations(rel);

    Item item;
    item.setId("file:1");

    TagRelation rela(item);
    TagRelationFetchJob* job = new TagRelationFetchJob(rela, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(relationReceived(Baloo::Relation)));
    QSignalSpy spy2(job, SIGNAL(tagRelationReceived(Baloo::TagRelation)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 2);
    QCOMPARE(spy1.at(0).size(), 1);
    QCOMPARE(spy1.at(1).size(), 1);
    // FIXME: What should we be testing about the original relation?
    //QCOMPARE(spy1.at(0).first().value<Relation*>(), &tagRel);

    QCOMPARE(spy2.size(), 2);
    QCOMPARE(spy2.at(0).size(), 1);
    QCOMPARE(spy2.at(1).size(), 1);
    TagRelation tagRel1 = spy2.at(0).first().value<TagRelation>();
    TagRelation tagRel2 = spy2.at(1).first().value<TagRelation>();

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);

    QCOMPARE(tagRel1.item().id(), QByteArray("file:1"));
    QCOMPARE(tagRel1.tag().id(), QByteArray("tag:1"));
    QCOMPARE(tagRel1.tag().name(), QString("TagA"));

    QCOMPARE(tagRel2.item().id(), QByteArray("file:1"));
    QCOMPARE(tagRel2.tag().id(), QByteArray("tag:2"));
    QCOMPARE(tagRel2.tag().name(), QString("TagB"));
}

void TagTests::testTagRelationSaveJob()
{
    insertTags(QStringList() << "TagA");

    Item item;
    item.setId("file:1");

    Tag tag = Tag::fromId(QByteArray("tag:1"));

    TagRelation rel(tag, item);
    TagRelationCreateJob* job = new TagRelationCreateJob(rel, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(relationCreated(Baloo::Relation)));
    QSignalSpy spy2(job, SIGNAL(tagRelationCreated(Baloo::TagRelation)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    //QCOMPARE(spy1.at(0).first().value<Relation*>(), &rel);

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);

    TagRelation tagRel2 = spy2.at(0).first().value<TagRelation>();
    QCOMPARE(tagRel2.item().id(), rel.item().id());
    QCOMPARE(tagRel2.tag().id(), rel.tag().id());
    QCOMPARE(tagRel2.tag().name(), rel.tag().name());

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);

    QSqlQuery query(m_con->d->db());
    query.prepare("select rid from tagRelations where tid = 1");

    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toByteArray(), QByteArray("file:1"));
}

void TagTests::testTagRelationSaveJob_duplicate()
{
    insertTags(QStringList() << "TagA");

    QHash<int, QString> relHash;
    relHash.insert(1, "file:1");
    insertRelations(relHash);

    Item item;
    item.setId("file:1");

    Tag tag = Tag::fromId(QByteArray("tag:1"));

    TagRelation rel(tag, item);
    TagRelationCreateJob* job = new TagRelationCreateJob(rel, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(relationCreated(Baloo::Relation)));
    QSignalSpy spy2(job, SIGNAL(tagRelationCreated(Baloo::TagRelation)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(!job->exec());

    QCOMPARE(spy1.size(), 0);
    QCOMPARE(spy2.size(), 0);

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), (int)TagRelationCreateJob::Error_RelationExists);
}

void TagTests::testTagRelationRemoveJob()
{
    insertTags(QStringList() << "TagA");

    QHash<int, QString> relHash;
    relHash.insert(1, "file:1");
    insertRelations(relHash);

    Item item;
    item.setId("file:1");

    Tag tag = Tag::fromId(QByteArray("tag:1"));

    TagRelation rel(tag, item);
    TagRelationRemoveJob* job = new TagRelationRemoveJob(rel, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(relationRemoved(Baloo::Relation)));
    QSignalSpy spy2(job, SIGNAL(tagRelationRemoved(Baloo::TagRelation)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy1.at(0).size(), 1);
    //QCOMPARE(spy1.at(0).first().value<Relation*>(), &rel);

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy2.at(0).size(), 1);

    TagRelation tagRel2 = spy2.at(0).first().value<TagRelation>();
    QCOMPARE(tagRel2.item().id(), rel.item().id());
    QCOMPARE(tagRel2.tag().id(), rel.tag().id());
    QVERIFY(tagRel2.tag().name().isEmpty());

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), 0);

    QSqlQuery query(m_con->d->db());
    query.prepare("select rid from tagRelations where tid = ?");
    query.addBindValue(1);
    QVERIFY(query.exec());
    QVERIFY(!query.next());
}

void TagTests::testTagRelationRemoveJob_notExists()
{
    Item item;
    item.setId("file:1");

    Tag tag = Tag::fromId(QByteArray("tag:1"));

    TagRelation rel(tag, item);
    TagRelationRemoveJob* job = new TagRelationRemoveJob(rel, m_con);
    QVERIFY(job);

    QSignalSpy spy1(job, SIGNAL(relationRemoved(Baloo::Relation)));
    QSignalSpy spy2(job, SIGNAL(tagRelationRemoved(Baloo::TagRelation)));
    QSignalSpy spy3(job, SIGNAL(result(KJob*)));
    QVERIFY(!job->exec());

    QCOMPARE(spy1.size(), 0);
    QCOMPARE(spy2.size(), 0);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy3.at(0).size(), 1);
    QCOMPARE(spy3.at(0).first().value<KJob*>(), job);
    QCOMPARE(job->error(), (int)TagRelationRemoveJob::Error_RelationDoesNotExist);
}

void TagTests::testTagStoreFetchAll()
{
    insertTags(QStringList() << "TagA" << "TagB" << "TagC");

    TagFetchJob* job = new TagFetchJob(m_con);

    QSignalSpy spy(job, SIGNAL(tagReceived(Baloo::Tag)));
    QVERIFY(job->exec());
    QCOMPARE(spy.size(), 3);

    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().value<Tag>().id(), QByteArray("tag:1"));
    QCOMPARE(spy.at(0).first().value<Tag>().name(), QString("TagA"));

    QCOMPARE(spy.at(1).size(), 1);
    QCOMPARE(spy.at(1).first().value<Tag>().id(), QByteArray("tag:2"));
    QCOMPARE(spy.at(1).first().value<Tag>().name(), QString("TagB"));

    QCOMPARE(spy.at(2).size(), 1);
    QCOMPARE(spy.at(2).first().value<Tag>().id(), QByteArray("tag:3"));
    QCOMPARE(spy.at(2).first().value<Tag>().name(), QString("TagC"));
}


QTEST_KDEMAIN_CORE(TagTests)
