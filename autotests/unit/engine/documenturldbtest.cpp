/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documenturldb.h"
#include "singledbtest.h"
#include "idutils.h"

#include <memory>

using namespace Baloo;

class DocumentUrlDBTest : public QObject
{
    Q_OBJECT

    void touchFile(const QByteArray& path) {
        QFile file(QString::fromUtf8(path));
        file.open(QIODevice::WriteOnly);
        file.write("data");
    }

private Q_SLOTS:
    void init()
    {
        m_tempDir = std::make_unique<QTemporaryDir>();

        mdb_env_create(&m_env);
        mdb_env_set_maxdbs(m_env, 2);

        // The directory needs to be created before opening the environment
        QByteArray path = QFile::encodeName(m_tempDir->path());
        mdb_env_open(m_env, path.constData(), 0, 0664);
        mdb_txn_begin(m_env, nullptr, 0, &m_txn);
    }

    void cleanup()
    {
        mdb_txn_abort(m_txn);
        m_txn = nullptr;
        mdb_env_close(m_env);
        m_env = nullptr;
        m_tempDir.reset();
    }

    void testNonExistingPath() {
        /*
        DocumentUrlDB db(m_txn);
        db.put(1, "/fire/does/not/exist");

        // FIXME: What about error handling?
        QCOMPARE(db.get(1), QByteArray());
        */
    }

    void testSingleFile()
    {
        QByteArray filePath("file1");
        quint64 id = 1;

        DocumentUrlDB db(IdTreeDB::create(m_txn), IdFilenameDB::create(m_txn), m_txn);
        db.put(id, 0, filePath);

        QVERIFY(db.contains(id));
        QCOMPARE(db.get(id), "/" + filePath);

        db.del(id);
        QCOMPARE(db.get(id), QByteArray());
    }

    void testTwoFilesAndAFolder()
    {
        QByteArray dirPath("<root>");
        QByteArray filePath1("file");
        QByteArray filePath2("file2");

        quint64 did = 99;
        quint64 id1 = 1;
        quint64 id2 = 2;

        DocumentUrlDB db(IdTreeDB::create(m_txn), IdFilenameDB::create(m_txn), m_txn);
        db.put(did, 0, dirPath);
        db.put(id1, did, filePath1);
        db.put(id2, did, filePath2);

        QVERIFY(db.contains(id1));
        QVERIFY(db.contains(id2));
        QCOMPARE(db.get(id1), "/<root>/" + filePath1);
        QCOMPARE(db.get(id2), "/<root>/" + filePath2);
        QCOMPARE(db.get(did), "/" + dirPath);

        db.del(id1);

        QCOMPARE(db.get(id1), QByteArray());
        QCOMPARE(db.get(id2), "/<root>/" + filePath2);
        QCOMPARE(db.get(did), "/" + dirPath);

        db.del(id2);

        QCOMPARE(db.get(id1), QByteArray());
        QCOMPARE(db.get(id2), QByteArray());
        QCOMPARE(db.get(did), "/" + dirPath);

        db.del(did);
        QCOMPARE(db.get(id1), QByteArray());
        QCOMPARE(db.get(id2), QByteArray());
        QCOMPARE(db.get(did), QByteArray());
    }

    void testFileRename()
    {
        const QByteArray path{"<root>"};
        quint64 did = 1;
        QByteArray filePath("file");
        quint64 id = 2;

        DocumentUrlDB db(IdTreeDB::create(m_txn), IdFilenameDB::create(m_txn), m_txn);

        db.put(did, 0, path);
        db.put(id, did, filePath);

        QCOMPARE(db.getId(did, filePath), id);
        QCOMPARE(db.get(id), QByteArray("/<root>/file"));

	db.updateUrl(id, did, "file2");

        QCOMPARE(db.getId(did, QByteArray("file2")), id);
        QCOMPARE(db.get(id), QByteArray("/<root>/file2"));

        db.del(id);
        QCOMPARE(db.get(id), QByteArray());
    }

    void testDuplicateId()
    {
    }

    void testGetId()
    {
        quint64 id = 1;

        QByteArray filePath1("file");
        quint64 id1 = 2;
        QByteArray filePath2("file2");
        quint64 id2 = 3;

        DocumentUrlDB db(IdTreeDB::create(m_txn), IdFilenameDB::create(m_txn), m_txn);

        db.put(id1, id, filePath1);
        db.put(id2, id, filePath2);

        QCOMPARE(db.getId(id, QByteArray("file")), id1);
        QCOMPARE(db.getId(id, QByteArray("file2")), id2);
    }

    void testSortedIdInsert()
    {
        // test sorted insert used in Baloo::DocumentUrlDB::add, bug 367991
        std::vector<quint64> test;
        test.push_back(9);

        // shall not crash
        sortedIdInsert(test, quint64(1));

        // stuff shall be ok inserted
        QVERIFY(test.size() == 2);
        QVERIFY(test[0] == 1);
        QVERIFY(test[1] == 9);

        // shall not crash
        sortedIdInsert(test, quint64(1));

        // no insert please
        QVERIFY(test.size() == 2);

        // shall not crash
        sortedIdInsert(test, quint64(10));

        // stuff shall be ok inserted
        QVERIFY(test.size() == 3);
        QVERIFY(test[0] == 1);
        QVERIFY(test[1] == 9);
        QVERIFY(test[2] == 10);

        // shall not crash
        sortedIdInsert(test, quint64(2));

        // stuff shall be ok inserted
        QVERIFY(test.size() == 4);
        QVERIFY(test[0] == 1);
        QVERIFY(test[1] == 2);
        QVERIFY(test[2] == 9);
        QVERIFY(test[3] == 10);

        // shall not crash
        sortedIdInsert(test, quint64(2));

        // no insert please
        QVERIFY(test.size() == 4);
    }

protected:
    MDB_env* m_env = nullptr;
    MDB_txn* m_txn = nullptr;
    std::unique_ptr<QTemporaryDir> m_tempDir;
};

QTEST_MAIN(DocumentUrlDBTest)

#include "documenturldbtest.moc"
