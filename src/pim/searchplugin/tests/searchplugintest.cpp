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
#include <QDir>

#include "searchplugin.h"
#include <../pim/agent/emailindexer.h>
#include <../pim/agent/contactindexer.h>
#include <../pim/search/email/emailsearchstore.h>
#include <../pim/search/contact/contactsearchstore.h>
#include <akonadi/searchquery.h>
#include <akonadi/kmime/messageflags.h>

Q_DECLARE_METATYPE(QSet<qint64>);
Q_DECLARE_METATYPE(QList<qint64>);

class SearchPluginTest : public QObject
{
    Q_OBJECT
private:
    QString emailDir;
    QString emailContactsDir;
    QString contactsDir;

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

private Q_SLOTS:
    void init() {
        emailDir = QDir::tempPath() + "/searchplugintest/baloo/email/";
        emailContactsDir = QDir::tempPath() + "/searchplugintest/baloo/emailcontacts/";
        contactsDir = QDir::tempPath() + "/searchplugintest/baloo/contacts/";

        QDir dir;
        removeDir(emailDir);
        QVERIFY(dir.mkpath(emailDir));
        removeDir(emailContactsDir);
        QVERIFY(dir.mkpath(emailContactsDir));
        removeDir(contactsDir);
        QVERIFY(dir.mkpath(contactsDir));

        kDebug() << "indexing sample data";
        kDebug() << emailDir;
        kDebug() << emailContactsDir;

        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        ContactIndexer contactIndexer(contactsDir);

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
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Replied);
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
            KABC::Addressee addressee;
            addressee.setUid("uid1");
            addressee.setName("John Doe");
            addressee.setFormattedName("John Doe");
            addressee.setNickName("JD");
            addressee.setEmails(QStringList() << "john@test.com");
            Akonadi::Item item(KABC::Addressee::mimeType());
            item.setId(3);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KABC::Addressee addressee;
            addressee.setUid("uid2");
            addressee.setName("Jane Doe");
            addressee.setEmails(QStringList() << "jane@test.com");
            Akonadi::Item item(KABC::Addressee::mimeType());
            item.setId(4);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }



        Baloo::EmailSearchStore *emailSearchStore = new Baloo::EmailSearchStore(this);
        emailSearchStore->setDbPath(emailDir);
        Baloo::ContactSearchStore *contactSearchStore = new Baloo::ContactSearchStore(this);
        contactSearchStore->setDbPath(contactsDir);
        Baloo::SearchStore::overrideSearchStores(QList<Baloo::SearchStore*>() << emailSearchStore << contactSearchStore);
    }

    void testEmailSearch_data() {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 1;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1;
            QTest::newRow("find subject equal") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);

            QList<qint64> collections = QList<qint64>() << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 2;
            QTest::newRow("find subject equal negated") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1 << 2;
            QTest::newRow("find subject contains") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "body", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1 << 2;
            QTest::newRow("find body contains") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, "mälmöö", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1;
            QTest::newRow("find utf8 body contains") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Headers, "From:", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1 << 2;
            QTest::newRow("find header contains") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, "body", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1 << 2;
            QTest::newRow("find message contains") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject1", Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject2", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1 << 2;
            QTest::newRow("or term") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, "subject", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 1;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result= QSet<qint64>() << 1;
            QTest::newRow("filter by collection") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Flagged), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result = QSet<qint64>() << 2;
            QTest::newRow("find by message flag") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondGreaterOrEqual));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result = QSet<qint64>() << 1 << 2;
            QTest::newRow("find by size greate than equal") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1002), Akonadi::SearchTerm::CondLessOrEqual));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result = QSet<qint64>() << 1 << 2;
            QTest::newRow("find by size greate than equal") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), Akonadi::SearchTerm::CondGreaterOrEqual));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result = QSet<qint64>() << 2;
            QTest::newRow("find by size separate") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, KDateTime(QDate(2013, 11, 10), QTime(12, 30, 0)).toString(KDateTime::ISODate), Akonadi::SearchTerm::CondGreaterOrEqual));

            QList<qint64> collections = QList<qint64>() << 1 << 2;
            QStringList mimeTypes = QStringList() << "message/rfc822";
            QSet<qint64> result = QSet<qint64>() << 2;
            QTest::newRow("find by date") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "John", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QStringList mimeTypes = QStringList() << KABC::Addressee::mimeType();
            QSet<qint64> result = QSet<qint64>() << 3;
            QTest::newRow("contact by name") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "John", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 4;
            QStringList mimeTypes = QStringList() << KABC::Addressee::mimeType();
            QSet<qint64> result;
            QTest::newRow("contact collectionfilter") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, "john", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 3;
            QStringList mimeTypes = QStringList() << KABC::Addressee::mimeType();
            QSet<qint64> result = QSet<qint64>() << 3;
            QTest::newRow("contact by lowercase name") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Nickname, "JD", Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 3;
            QStringList mimeTypes = QStringList() << KABC::Addressee::mimeType();
            QSet<qint64> result = QSet<qint64>() << 3;
            QTest::newRow("contact by nickname") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, "uid1", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 3;
            QStringList mimeTypes = QStringList() << KABC::Addressee::mimeType();
            QSet<qint64> result = QSet<qint64>() << 3;
            QTest::newRow("contact by uid") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
        }
//         {
//             Akonadi::SearchQuery query;
//             query.addTerm(Akonadi::SearchTerm("*", "", Akonadi::SearchTerm::CondEqual));
//
//             QList<qint64> collections;
//             QStringList mimeTypes = QStringList() << KABC::Addressee::mimeType();
//             QSet<qint64> result = QSet<qint64>() << 3 << 4;
//             QTest::newRow("retrieve all contacts") << QString::fromLatin1(query.toJSON()) << collections << mimeTypes << result;
//         }

    }

    void testEmailSearch() {
        QFETCH(QString, query);
        QFETCH(QList<qint64>, collections);
        QFETCH(QStringList, mimeTypes);
        QFETCH(QSet<qint64>, expectedResult);

        kDebug() << "starting search";
        SearchPlugin plugin;
        QSet<qint64> result = plugin.search(query, collections, mimeTypes);
        kDebug() << result;
        QCOMPARE(result, expectedResult);
    }

};

QTEST_MAIN(SearchPluginTest)

#include "searchplugintest.moc"