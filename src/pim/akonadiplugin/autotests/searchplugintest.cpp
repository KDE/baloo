/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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

#include <QTest>
#include <Akonadi/Collection>
#include <KABC/Addressee>
#include <KABC/ContactGroup>
#include <QDir>

#include "searchplugin.h"
#include <../pim/agent/emailindexer.h>
#include <../pim/agent/contactindexer.h>
#include <../pim/agent/akonotesindexer.h>
#include <../pim/search/email/emailsearchstore.h>
#include <../pim/search/contact/contactsearchstore.h>
#include <../pim/search/note/notesearchstore.h>
#include <akonadi/searchquery.h>
#include <akonadi/kmime/messageflags.h>

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QList<qint64>)


class SearchPluginTest : public QObject
{
    Q_OBJECT
private:
    QString emailDir;
    QString emailContactsDir;
    QString contactsDir;
    QString noteDir;

    bool removeDir(const QString & dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName)) {
            Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                if (info.isDir()) {
                    result = removeDir(info.absoluteFilePath());
                }
                else {
                    result = QFile::remove(info.absoluteFilePath());
                }

                if (!result) {
                    return result;
                }
            }
            result = dir.rmdir(dirName);
        }
        return result;
    }
    void resultSearch()
    {
        QFETCH(QString, query);
        QFETCH(QList<qint64>, collections);
        QFETCH(QStringList, mimeTypes);
        QFETCH(QSet<qint64>, expectedResult);

        kDebug() << "starting search";
        SearchPlugin plugin;
        const QSet<qint64> result = plugin.search(query, collections, mimeTypes);
        kDebug() << result;
        QCOMPARE(result, expectedResult);
    }

private Q_SLOTS:
    void init() {
        emailDir = QDir::tempPath() + "/searchplugintest/baloo/email/";
        emailContactsDir = QDir::tempPath() + "/searchplugintest/baloo/emailcontacts/";
        contactsDir = QDir::tempPath() + "/searchplugintest/baloo/contacts/";
        noteDir = QDir::tempPath() + "/searchplugintest/baloo/notes/";

        QDir dir;
        QVERIFY(removeDir(emailDir));
        QVERIFY(dir.mkpath(emailDir));
        QVERIFY(removeDir(emailContactsDir));
        QVERIFY(dir.mkpath(emailContactsDir));
        QVERIFY(removeDir(contactsDir));
        QVERIFY(dir.mkpath(contactsDir));
        QVERIFY(removeDir(noteDir));
        QVERIFY(dir.mkpath(noteDir));

        kDebug() << "indexing sample data";
        kDebug() << emailDir;
        kDebug() << emailContactsDir;
        kDebug() << noteDir;

        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        ContactIndexer contactIndexer(contactsDir);
        AkonotesIndexer noteIndexer(noteDir);

        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject1");
            msg->contentType()->setMimeType("text/plain");
            msg->setBody("body1 mälmöö");
            msg->from()->addAddress("john@test.com", "John Doe");
            msg->to()->addAddress("jane@test.com", "Jane Doe");
            msg->date()->setDateTime(KDateTime(QDate(2013,11,10), QTime(12,0,0)));
            msg->assemble();

            Akonadi::Item item("message/rfc822");
            item.setId(1);
            item.setSize(1000);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(1));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Replied << Akonadi::MessageFlags::Encrypted);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject2");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "body2" );
            msg->addContent( b, true );

            msg->from()->addAddress("john@test.com", "John Doe");
            msg->to()->addAddress("jane@test.com", "Jane Doe");
            msg->date()->setDateTime(KDateTime(QDate(2013,11,10), QTime(13,0,0)));
            msg->organization()->from7BitString("kde");
            msg->assemble();

            Akonadi::Item item("message/rfc822");
            item.setId(2);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject3");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "body3" );
            msg->addContent( b, true );

            msg->from()->addAddress("john@test.com", "John Doe");
            msg->to()->addAddress("jane@test.com", "Jane Doe");
            msg->date()->setDateTime(KDateTime(QDate(2014,11,10), QTime(13,0,0)));
            msg->organization()->from7BitString("kde5");
            msg->assemble();

            Akonadi::Item item("message/rfc822");
            item.setId(3);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject4");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "body4" );
            msg->addContent( b, true );

            msg->from()->addAddress("john@test.com", "John Doe");
            msg->to()->addAddress("jane@test.com", "Jane Doe");
            msg->cc()->addAddress("cc@test.com", "Jane Doe");
            msg->bcc()->addAddress("bcc@test.com", "Jane Doe");
            msg->date()->setDateTime(KDateTime(QDate(2014,11,11), QTime(13,0,0)));
            msg->replyTo()->from7BitString("test@kde.org");
            KMime::Headers::Generic *header = new KMime::Headers::Generic( "Resent-From", msg.get(), QLatin1String("resent@kde.org"), "utf-8" );
            msg->setHeader( header );
            header = new KMime::Headers::Generic( "List-Id", msg.get(), QLatin1String("KDE PIM <kde-pim.kde.org>"), "utf-8" );
            msg->setHeader( header );

            msg->assemble();

            Akonadi::Item item("message/rfc822");
            item.setId(4);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("all tags");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "tags" );
            msg->addContent( b, true );

            msg->from()->addAddress("john@test.com", "John Doe");
            msg->to()->addAddress("jane@test.com", "Jane Doe");
            msg->date()->setDateTime(KDateTime(QDate(2014,11,11), QTime(13,0,0)));
            msg->assemble();

            Akonadi::Item item("message/rfc822");
            item.setId(5);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Seen
                          << Akonadi::MessageFlags::Deleted
                          << Akonadi::MessageFlags::Answered
                          << Akonadi::MessageFlags::Flagged
                          << Akonadi::MessageFlags::HasAttachment
                          << Akonadi::MessageFlags::HasInvitation
                          << Akonadi::MessageFlags::Sent
                          << Akonadi::MessageFlags::Queued
                          << Akonadi::MessageFlags::Replied
                          << Akonadi::MessageFlags::Forwarded
                          << Akonadi::MessageFlags::ToAct
                          << Akonadi::MessageFlags::Watched
                          << Akonadi::MessageFlags::Ignored
                          << Akonadi::MessageFlags::Encrypted
                          /*<< Akonadi::MessageFlags::Spam*/
                          << Akonadi::MessageFlags::Ham);
            //Spam is exclude from indexer. So we can't add it.
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("Change in qt/qtx11extras[stable]: remove QtWidgets dependency");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "body5" );
            msg->addContent( b, true );

            msg->from()->addAddress("john@test.com", "John Doe");
            msg->to()->addAddress("jane@test.com", "Jane Doe");
            msg->date()->setDateTime(KDateTime(QDate(2014,11,11), QTime(13,0,0)));
            msg->assemble();

            Akonadi::Item item("message/rfc822");
            item.setId(6);
            item.setSize(50);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        //Contact item
        {
            KABC::Addressee addressee;
            addressee.setUid("uid1");
            addressee.setName("John Doe");
            addressee.setFormattedName("John Doe");
            addressee.setNickName("JD");
            addressee.setEmails(QStringList() << "john@test.com");
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01)));
            Akonadi::Item item(KABC::Addressee::mimeType());
            item.setId(100);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KABC::Addressee addressee;
            addressee.setUid("uid2");
            addressee.setName("Jane Doe");
            addressee.setEmails(QStringList() << "jane@test.com");
            addressee.setBirthday(QDateTime(QDate(2001, 01, 01)));
            Akonadi::Item item(KABC::Addressee::mimeType());
            item.setId(101);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KABC::Addressee addressee;
            addressee.setUid("uid2");
            addressee.setName("Jane Doe");
            addressee.setEmails(QStringList() << "JANE@TEST.COM");
            addressee.setBirthday(QDateTime(QDate(2001, 01, 01)));
            Akonadi::Item item(KABC::Addressee::mimeType());
            item.setId(102);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KABC::ContactGroup group;
            group.setName("group1");
            Akonadi::Item item(KABC::ContactGroup::mimeType());
            item.setId(103);
            item.setPayload(group);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }


        //Note item
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("note");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "body note" );
            msg->addContent( b, true );
            msg->assemble();

            Akonadi::Item item("text/x-vnd.akonadi.note");
            item.setId(1000);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(5));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            noteIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("note2");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "note" );
            msg->addContent( b, true );
            msg->assemble();

            Akonadi::Item item("text/x-vnd.akonadi.note");
            item.setId(1001);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(5));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            noteIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("note3");

            //Multipart message
            KMime::Content *b = new KMime::Content;
            b->contentType()->setMimeType( "text/plain" );
            b->setBody( "note3" );
            msg->addContent( b, true );
            msg->assemble();

            Akonadi::Item item("text/x-vnd.akonadi.note");
            item.setId(1002);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(5));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            noteIndexer.index(item);
        }



        Baloo::EmailSearchStore *emailSearchStore = new Baloo::EmailSearchStore(this);
        emailSearchStore->setDbPath(emailDir);
        Baloo::ContactSearchStore *contactSearchStore = new Baloo::ContactSearchStore(this);
        contactSearchStore->setDbPath(contactsDir);
        Baloo::NoteSearchStore *noteSearchStore = new Baloo::NoteSearchStore(this);
        noteSearchStore->setDbPath(noteDir);

        Baloo::SearchStore::overrideSearchStores(QList<Baloo::SearchStore*>() << emailSearchStore << contactSearchStore << noteSearchStore);
    }
#if 1
    void testNoteSearch_data() {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");
        const QStringList notesMimeTypes = QStringList() << "text/x-vnd.akonadi.note";
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "note", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1000;
            QTest::newRow("find note subject equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "note1", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result ;
            QTest::newRow("find note subject equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "note", Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "note", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1000 << 1001;
            QTest::newRow("find note subject equal or body equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "note3", Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "note3", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1002;
            QTest::newRow("find note subject equal and body equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, "note3", Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);
            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1000 << 1001;
            QTest::newRow("find not subject equal note3") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
    }
    void testNoteSearch() {
        resultSearch();
    }

    void testContactSearch() {
        resultSearch();
    }

    void testContactSearch_data() {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");
        const QStringList contactMimeTypes = QStringList() << KABC::Addressee::mimeType();
        const QStringList contactGroupMimeTypes = QStringList() << KABC::ContactGroup::mimeType();
#if 1
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "John", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "John", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 4;
            QSet<qint64> result;
            QTest::newRow("contact collectionfilter") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "john", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by lowercase name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Nickname, "JD", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by nickname") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, "uid1", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by uid") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, "JANE@TEST.COM", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 101 << 102;
            QTest::newRow("contact by email") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "Doe", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100 << 101 << 102;
            QTest::newRow("contact by name (Doe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "Do", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100 << 101 << 102;
            QTest::newRow("contact by name (Do)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
#endif
        {
#if 0
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "group1", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 103;
            QTest::newRow("contact group by name (group1)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
#endif
        }

#if 0 //Doesn't work for the moment
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "oe", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100 << 101 << 102;
            QTest::newRow("contact by name (oe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
#endif
    }
#endif
    void testEmailSearch_data() {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");
        const QStringList emailMimeTypes = QStringList() << "message/rfc822";
        const QList<qint64> allEmailCollections = QList<qint64>() << 1 << 2;
#if 1
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 1;
            QSet<qint64> result= QSet<qint64>() << 1;
            QTest::newRow("find subject equal") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);
            QList<qint64> collections = QList<qint64>() << 2;
            QSet<qint64> result= QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find subject equal negated") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result= QSet<qint64>() << 1 << 2 << 3 << 4;
            QTest::newRow("find subject contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "body", Akonadi::SearchTerm::CondContains));
            QList<qint64> collections = QList<qint64>() << 1 << 2 << 3 << 4;
            QSet<qint64> result= QSet<qint64>() << 1 << 2 << 3 << 4 << 6;
            QTest::newRow("find body contains") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "mälmöö", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result= QSet<qint64>() << 1;
            QTest::newRow("find utf8 body contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Headers, "From:", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result= QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find header contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, "body", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result= QSet<qint64>() << 1 << 2 << 3 << 4 << 6;
            QTest::newRow("find message contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject2", Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result= QSet<qint64>() << 1 << 2;
            QTest::newRow("or term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "body1", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result= QSet<qint64>() << 1;
            QTest::newRow("and term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "body2", Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result;
            QTest::newRow("and term equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject", Akonadi::SearchTerm::CondContains));
            QList<qint64> collections = QList<qint64>() << 1;
            QSet<qint64> result= QSet<qint64>() << 1;
            QTest::newRow("filter by collection") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Flagged), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by message flag") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by message replied") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Encrypted), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 5;
            QTest::newRow("find by message replied and encrypted") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Seen), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Answered), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Flagged), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasAttachment), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasInvitation), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Sent), Akonadi::SearchTerm::CondContains));
            //query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Queued), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Forwarded), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::ToAct), Akonadi::SearchTerm::CondContains));
            //query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Watched), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Ignored), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Encrypted), Akonadi::SearchTerm::CondContains));
            //Spam is exclude from indexer.
            //query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Spam), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Ham), Akonadi::SearchTerm::CondContains));

            QSet<qint64> result = QSet<qint64>() << 5;
            QTest::newRow("find by message by all status") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by size greater than equal great or equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 1;
            QTest::newRow("find by size equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1002), Akonadi::SearchTerm::CondLessOrEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5;
            QTest::newRow("find by size greater than equal less or equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by size separate") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), Akonadi::SearchTerm::CondGreaterThan));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by size separate (greater than)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, KDateTime(QDate(2013, 11, 10), QTime(12, 30, 0)).dateTime(), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by date") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, KDateTime(QDate(2013, 11, 10), QTime(12, 0, 0)).dateTime(), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 1;
            QTest::newRow("find by date equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014,11,11), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 4 << 5 << 6;
            QTest::newRow("find by date only (equal condition)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2013, 11, 10), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by date only (greater or equal)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 3 << 4  << 5 << 6;
            QTest::newRow("find by date only greater or equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10), Akonadi::SearchTerm::CondGreaterThan));
            QSet<qint64> result = QSet<qint64>() << 4 << 5 << 6;
            QTest::newRow("find by date only greater than") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, "Jane Doe <cc@test.com>", Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header cc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, "cc@test.com", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header cc (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, "kde", Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 2;
            QTest::newRow("find by header organization (equal)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, "kde", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 2 << 3;
            QTest::newRow("find by header organization (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderBCC, "Jane Doe <bcc@test.com>", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header bcc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderReplyTo, "test@kde.org", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by reply to") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, "kde-pim.kde.org", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by list id") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, "kde-pim.kde.org", Akonadi::SearchTerm::CondContains));

            QSet<qint64> result = QSet<qint64>() << 4 << 5;
            QTest::newRow("find by message by deleted status or headerListId") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, "kde-pim.kde.org", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 4 << 5;
            QTest::newRow("find by message by deleted status or headerListId in all collections") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, "kde-pim.kde.org", Akonadi::SearchTerm::CondContains));

            QSet<qint64> result;
            QTest::newRow("find by message by deleted status and headerListId") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, "subject", Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by message term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, "CC@TEST.com", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header cc (contains) with case") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
#if 0 //Can not work for the moment
        {
            Akonadi::SearchQuery query;
            //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "extras", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 6;
            QTest::newRow("search extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
#endif
        {
            Akonadi::SearchQuery query;
            //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "change", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 6;
            QTest::newRow("search \"change\" in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "qtx11extras", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 6;
            QTest::newRow("search qtx11extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
#endif
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, "test.com", Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("search by from email part") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
    }

    void testEmailSearch() {
        resultSearch();
    }

};

QTEST_MAIN(SearchPluginTest)

#include "searchplugintest.moc"

