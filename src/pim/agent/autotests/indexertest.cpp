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
#include <AkonadiCore/Collection>
#include <KABC/Addressee>
#include <QDir>

#include "emailindexer.h"
#include "contactindexer.h"
#include <../pim/search/email/emailsearchstore.h>
#include <../pim/search/contact/contactsearchstore.h>
#include <query.h>

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QList<qint64>)

class IndexerTest : public QObject
{
    Q_OBJECT
private:
    QString emailDir;
    QString emailContactsDir;
    QString contactsDir;
    QString notesDir;

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
        emailDir = QDir::tempPath() + QLatin1String("/searchplugintest/baloo/email/");
        emailContactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/baloo/emailcontacts/");
        contactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/baloo/contacts/");
        notesDir = QDir::tempPath() + QLatin1String("/searchplugintest/baloo/notes/");

        QDir dir;
        removeDir(emailDir);
        QVERIFY(dir.mkpath(emailDir));
        removeDir(emailContactsDir);
        QVERIFY(dir.mkpath(emailContactsDir));
        removeDir(contactsDir);
        QVERIFY(dir.mkpath(contactsDir));
        removeDir(notesDir);
        QVERIFY(dir.mkpath(notesDir));

        qDebug() << "indexing sample data";
        qDebug() << emailDir;
        qDebug() << emailContactsDir;
        qDebug() << notesDir;

//         EmailIndexer emailIndexer(emailDir, emailContactsDir);
//         ContactIndexer contactIndexer(contactsDir);

//         Baloo::EmailSearchStore *emailSearchStore = new Baloo::EmailSearchStore(this);
//         emailSearchStore->setDbPath(emailDir);
//         Baloo::ContactSearchStore *contactSearchStore = new Baloo::ContactSearchStore(this);
//         contactSearchStore->setDbPath(contactsDir);
//         Baloo::SearchStore::overrideSearchStores(QList<Baloo::SearchStore*>() << emailSearchStore << contactSearchStore);
    }

    QSet<qint64> getAllItems() {
        QSet<qint64> resultSet;

        Baloo::Term term(Baloo::Term::Or);
        term.addSubTerm(Baloo::Term(QLatin1String("collection"), QLatin1String("1"), Baloo::Term::Equal));
        term.addSubTerm(Baloo::Term(QLatin1String("collection"), QLatin1String("2"), Baloo::Term::Equal));
        Baloo::Query query(term);
        query.setType(QLatin1String("Email"));

        Baloo::EmailSearchStore *emailSearchStore = new Baloo::EmailSearchStore(this);
        emailSearchStore->setDbPath(emailDir);
        int res = emailSearchStore->exec(query);
        qDebug() << res;
        while (emailSearchStore->next(res)) {
            const int fid = Baloo::deserialize("akonadi", emailSearchStore->id(res));
            resultSet << fid;
        }
        qDebug() << resultSet;
        return resultSet;
    }

    void testEmailRemoveByCollection() {
        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject1");
            msg->assemble();

            Akonadi::Item item(QLatin1String("message/rfc822"));
            item.setId(1);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(1));
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject2");
            msg->assemble();

            Akonadi::Item item(QLatin1String("message/rfc822"));
            item.setId(2);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            emailIndexer.index(item);
        }
        emailIndexer.commit();
        QCOMPARE(getAllItems(), QSet<qint64>() << 1 << 2);
        emailIndexer.remove(Akonadi::Collection(2));
        emailIndexer.commit();
        QCOMPARE(getAllItems(), QSet<qint64>() << 1);
    }
};

QTEST_MAIN(IndexerTest)

#include "indexertest.moc"

