/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documenturldb.h"
#include "singledbtest.h"
#include "idutils.h"


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
        m_tempDir = new QTemporaryDir();

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
        mdb_env_close(m_env);
        delete m_tempDir;
    }

    void testNonExistingPath() {
        /*
        DocumentUrlDB db(m_txn);
        db.put(1, "/fire/does/not/exist");

        // FIXME: What about error handling?
        QCOMPARE(db.get(1), QByteArray());
        */
    }

    void testSingleFile() {
        QTemporaryDir dir;

        QByteArray filePath(dir.path().toUtf8() + "/file");
        touchFile(filePath);
        quint64 id = filePathToId(filePath);

        DocumentUrlDB db(IdTreeDB::create(m_txn), IdFilenameDB::create(m_txn), m_txn);
        db.put(id, filePath);

        QCOMPARE(db.get(id), filePath);

        db.del(id, [](quint64) { return true; });
        QCOMPARE(db.get(id), QByteArray());
    }

    void testTwoFilesAndAFolder() {
        QTemporaryDir dir;

        QByteArray dirPath = QFile::encodeName(dir.path());
        QByteArray filePath1(dirPath + "/file");
        QByteArray filePath2(dirPath + "/file2");

        touchFile(filePath1);
        touchFile(filePath2);
        quint64 did = filePathToId(dirPath);
        quint64 id1 = filePathToId(filePath1);
        quint64 id2 = filePathToId(filePath2);

        DocumentUrlDB db(IdTreeDB::create(m_txn), IdFilenameDB::create(m_txn), m_txn);
        db.put(did, dirPath);
        db.put(id1, filePath1);
        db.put(id2, filePath2);

        QCOMPARE(db.get(id1), filePath1);
        QCOMPARE(db.get(id2), filePath2);
        QCOMPARE(db.get(did), dirPath);

        db.del(id1, [=](quint64 id) { return id != did; });

        QCOMPARE(db.get(id1), QByteArray());
        QCOMPARE(db.get(id2), filePath2);
        QCOMPARE(db.get(did), dirPath);

        db.del(id2, [=](quint64 id) { return id != did; });

        QCOMPARE(db.get(id1), QByteArray());
        QCOMPARE(db.get(id2), QByteArray());
        QCOMPARE(db.get(did), dirPath);

        db.del(did, [=](quint64 id) { return id != did; });

        QCOMPARE(db.get(id1), QByteArray());
        QCOMPARE(db.get(id2), QByteArray());
        QCOMPARE(db.get(did), QByteArray());
    }

    void testDuplicateId() {
    }

    void testGetId() {
        QTemporaryDir dir;
        const QByteArray path = QFile::encodeName(dir.path());
        quint64 id = filePathToId(path);

        QByteArray filePath1(path + "/file");
        touchFile(filePath1);
        quint64 id1 = filePathToId(filePath1);
        QByteArray filePath2(path + "/file2");
        touchFile(filePath2);
        quint64 id2 = filePathToId(filePath2);

        DocumentUrlDB db(IdTreeDB::create(m_txn), IdFilenameDB::create(m_txn), m_txn);
        db.put(id, path);
        db.put(id1, filePath1);
        db.put(id2, filePath2);

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
    MDB_env* m_env;
    MDB_txn* m_txn;
    QTemporaryDir* m_tempDir;
};

QTEST_MAIN(DocumentUrlDBTest)

#include "documenturldbtest.moc"
