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

#include "../src/tag/tag.h"
#include "../src/tag/tagmanager.h"
#include "../src/tag/tagrelation.h"
#include "../src/tag/tagrelationwatcher.h"

#include <QTest>

void testTagCreation()
{
    // Check if the tag already exists
    Tag tag("TagA");

    TagFetchJob* fetchJob = tag.load();
    fetchJob->exec();

    QVERIFY(!fetchJob->error());

    QCOMPARE(fetchJob->tag(), &tag);
    QVERIFY(!tag.exists();

    // Create the tag
    TagCreateJob* createJob = tag.create();
    createJob->exec();

    QVERIFY(!createJob->error());
    QVERIFY(tag.exists());
    QCOMPARE(tag.name(), QStringLiteral("TagA"));
    QVERIFY(tag.id());
}

void testTagModification() {
    // Creation
    Tag tag("TagA");
    TagCreateJob* createJob = tag.create();
    createJob->exec();

    QVERIFY(!createJob->error());
    QVERIFY(tag.exists());
    QCOMPARE(tag.name(), QStringLiteral("TagA"));
    QVERIFY(tag.id());

    QString id = tag.id();

    // Modification
    tag.setName("TagB");
    TagModifyJob* modifyJob = tag.modify();
    modifyJob->exec();

    QVERIFY(!modifyJob->error());
    QVERIFY(id, tag.id());
}

void testTagAssignment() {
    Tag tag("TagA");

    // Can be a file/email/anything
    Item item;
    TagRelation tagRelation(tag, item);

    TagCreateJob* tagCreateJob = tagRelation.create();
    tagCreateJob->exec();
    QVERIFY(!tagCreateJob->error());
    QVERIFY(tag.exists());

    // Removal
    TagRemoveJob* tagRemoveJob = tagRelation.remove();
    tagRemoveJob->exec();
    QVERIFY(!tagRemoveJob->error());
}

void testTagItemListing() {
    Tag tag("TagA");

    TagRelation tagRelation(tag, Item());

    TagFetchJob* tagFetchJob = tagRelation.fetch();

    connect(tagFetchJob, SIGNAL(itemReceived(Item)),
            this, SLOT(slotTaggedItemReceived(Item)));
}

vodi testItemTagsListing() {
    Item item;

    TagRelation tagRelation(Tag(), item);

    TagFetchJob* tagFetchJob = tagRelation.fetch();

    connect(tagFetchJob, SIGNAL(tagReceived(Tag)),
            this, SLOT(slotItemTagReceived(Tag)));
}

void testTagListing() {
    TagManager* tagManager = TagManager::instance();

    TagFetchJob* tagListJob = tagManager->listTags();
    tagListJob->exec();
    QVERIFY(!tagListJob->error());

    connect(tagListJob, SIGNAL(tagReceived(Tag)),
            this, SLOT(slotTagReceived(Tag)));
}

void testTagCreationWatching() {
    TagManager* tagManager = TagManager::instance();
    tagManager->setWatchEnabled(true);

    connect(tagManager, SIGNAL(tagCreated(Tag)),
            this, SLOT(slotTagCreated(Tag)));
    connect(tagManager, SIGNAL(tagRemoved(Tag)),
            this, SLOT(slotTagRemoved(Tag)));
}

void testTagRelationWatching() {
    Tag tag("TagA");

    TagRelationWatcher tagWatcher(tag);
    connect(tagWatcher, SIGNAL(itemAdded(Item)),
            this, SLOT(slotItemAdded(Item)));
    connect(tagWatcher, SIGNAL(itemRemoved(Item)),
            this, SLOT(slotItemRemoved(Item)));

    Item item;
    TagRelationWatcher tagWatcher2(item);
    connect(tagWatcher2, SIGNAL(tagAdded(Tag)),
            this, SLOT(slotTagAdded(Tag)));
    connect(tagWatcher2, SIGNAL(tagRemoved(Tag)),
            this, SLOT(slotTagRemoved(Tag)));
}
