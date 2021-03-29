/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "database.h"
#include "transaction.h"
#include "document.h"
#include "termgenerator.h"
#include "enginequery.h"
#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class SortedIdVector : public QVector<quint64> {
    public:
        SortedIdVector(const QVector<quint64>& list)
        : QVector<quint64>(list) {
            std::sort(begin(), end());
        }
        SortedIdVector(std::initializer_list<quint64> args)
        : SortedIdVector(QVector<quint64>(args)) {}
};

char *toString(const QVector<quint64> &idlist)
{
    QByteArray text("IDs[");
    text += QByteArray::number(idlist.size()) + "]:";
    for (auto id : idlist) {
        text += " " + QByteArray::number(id, 16);
    }
    return qstrdup(text.data());
}

class QueryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
        dir.reset(new QTemporaryDir());

        auto touchFile = [](const QString& path) {
            QFile file(path);
            file.open(QIODevice::WriteOnly);
            file.write("data");
            file.close();

            return filePathToId(QFile::encodeName(path));
        };

        m_id1 = touchFile(dir->path() + QStringLiteral("/file1.txt"));
        m_id2 = touchFile(dir->path() + QStringLiteral("/file2"));
        m_id3 = touchFile(dir->path() + QStringLiteral("/file3"));
        m_id4 = touchFile(dir->path() + QStringLiteral("/file4"));
        m_id7 = touchFile(dir->path() + QStringLiteral("/file7_lazy"));
        m_id8 = touchFile(dir->path() + QStringLiteral("/file8_dog"));

        m_id5 = touchFile(dir->path() + QStringLiteral("/tagFile1"));
        m_id6 = touchFile(dir->path() + QStringLiteral("/tagFile2"));
    }

    void init() {
        dbDir = new QTemporaryDir();
        db = new Database(dbDir->path());
        db->open(Database::CreateDatabase);

        insertDocuments();
    }

    void cleanup() {
        delete db;
        delete dbDir;
    }

    void testTermEqual();
    void testTermStartsWith();
    void testTermAnd();
    void testTermOr();
    void testTermPhrase_data();
    void testTermPhrase();

    void testTagTermAnd_data();
    void testTagTermAnd();
    void testTagTermPhrase_data();
    void testTagTermPhrase();

private:
    QScopedPointer<QTemporaryDir> dir;
    QTemporaryDir* dbDir;
    Database* db;

    void insertDocuments();
    void addDocument(Transaction* tr,const QString& text, quint64 id, const QString& fileName)
    {
        Document doc;
        QString url = dir->path() + QLatin1Char('/') + fileName;
        doc.setUrl(QFile::encodeName(url));

        TermGenerator tg(doc);
        tg.indexText(text);
        tg.indexFileNameText(fileName);
        doc.setId(id);
        doc.setMTime(1);
        doc.setCTime(2);

        tr->addDocument(doc);
    }

    void renameDocument(Transaction* tr, quint64 id, const QString& newName)
    {
        Document doc;

        TermGenerator tg(doc);
        tg.indexFileNameText(newName);
        doc.setId(id);

        tr->replaceDocument(doc, FileNameTerms);
    }

    void insertTagDocuments();
    void addTagDocument(Transaction* tr,const QStringList& tags, quint64 id, const QString& fileName)
    {
        Document doc;
        QString url = dir->path() + QLatin1Char('/') + fileName;
        doc.setUrl(QFile::encodeName(url));

        TermGenerator tg(doc);
        tg.indexText(QStringLiteral("text/plain"), QByteArray("M"));
        for (const QString& tag : tags) {
            tg.indexXattrText(tag, QByteArray("TA"));
        }
        tg.indexFileNameText(fileName);
        doc.setId(id);
        doc.setMTime(3);
        doc.setCTime(4);

        tr->addDocument(doc);
    }

    quint64 m_id1;
    quint64 m_id2;
    quint64 m_id3;
    quint64 m_id4;
    quint64 m_id5;
    quint64 m_id6;
    quint64 m_id7;
    quint64 m_id8;
};


void QueryTest::insertDocuments()
{
    Transaction tr(db, Transaction::ReadWrite);
    addDocument(&tr, QStringLiteral("The quick brown fox jumped over the crazy dog"), m_id1, QStringLiteral("file1.txt"));
    addDocument(&tr, QStringLiteral("The quick brown fox jumped over the lazy dog"), m_id7, QStringLiteral("file7_lazy"));
    addDocument(&tr, QStringLiteral("A quick brown fox ran around a easy dog"), m_id8, QStringLiteral("file8_dog"));
    addDocument(&tr, QStringLiteral("The night is dark and full of terror"), m_id2, QStringLiteral("file2"));
    addDocument(&tr, QStringLiteral("Don't feel sorry for yourself. Only assholes do that"), m_id3, QStringLiteral("file3"));
    addDocument(&tr, QStringLiteral("Only the dead stay 17 forever. crazy"), m_id4, QStringLiteral("file4"));

    renameDocument(&tr, m_id8, QStringLiteral("file8_easy"));
    tr.commit();
}

void QueryTest::insertTagDocuments()
{
    Transaction tr(db, Transaction::ReadWrite);
    addTagDocument(&tr, {
	QStringLiteral("One"),
	QStringLiteral("Two"),
	QStringLiteral("Three"),
	QStringLiteral("Four"),
	QStringLiteral("F1")
    }, m_id5, QStringLiteral("tagFile1"));
    addTagDocument(&tr, {
	QStringLiteral("One"),
	QStringLiteral("Two-Three"),
	QStringLiteral("Four"),
	QStringLiteral("F2")
    }, m_id6, QStringLiteral("tagFile2"));
    tr.commit();
}

void QueryTest::testTermEqual()
{
    EngineQuery q("the");

    QVector<quint64> result = SortedIdVector{m_id1, m_id2, m_id4, m_id7};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermStartsWith()
{
    EngineQuery q("for", EngineQuery::StartsWith);

    QVector<quint64> result = SortedIdVector{m_id3, m_id4};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermAnd()
{
    QVector<EngineQuery> queries;
    queries << EngineQuery("for");
    queries << EngineQuery("sorry");

    EngineQuery q(queries, EngineQuery::And);

    QVector<quint64> result = {m_id3};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermOr()
{
    QVector<EngineQuery> queries;
    queries << EngineQuery("over");
    queries << EngineQuery("terror");

    EngineQuery q(queries, EngineQuery::Or);

    QVector<quint64> result = SortedIdVector{m_id1, m_id2, m_id7};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermPhrase_data()
{
    QTest::addColumn<QByteArrayList>("phrase");
    QTest::addColumn<QVector<quint64>>("contentMatches");
    QTest::addColumn<QVector<quint64>>("filenameMatches");

    auto addRow = [](const char* name, const QByteArrayList& phrase,
                     const QVector<quint64> contentMatches,
                     const QVector<quint64> filenameMatches)
        { QTest::addRow("%s", name) << phrase << contentMatches << filenameMatches;};

    // Content matches
    addRow("Crazy dog",        {QByteArrayLiteral("crazy"), QByteArrayLiteral("dog")},  SortedIdVector{ m_id1 }, {});
    addRow("Lazy dog",         {QByteArrayLiteral("lazy"),  QByteArrayLiteral("dog")},  SortedIdVector{ m_id7 }, {});
    addRow("Brown fox",        {QByteArrayLiteral("brown"), QByteArrayLiteral("fox")},  SortedIdVector{ m_id1, m_id7, m_id8 }, {});
    addRow("Dog",              {QByteArrayLiteral("dog")},                              SortedIdVector{ m_id1, m_id7, m_id8 }, {});
    // Filename matches
    addRow("Crazy dog file 1", {QByteArrayLiteral("file1")},                            {}, SortedIdVector{ m_id1 });
    addRow("Crazy dog file 2", {QByteArrayLiteral("file1"), QByteArrayLiteral("txt")},  {}, SortedIdVector{ m_id1 });
    addRow("Lazy dog file 1",  {QByteArrayLiteral("file7")},                            {}, SortedIdVector{ m_id7 });
    addRow("Lazy dog file 2",  {QByteArrayLiteral("file7"), QByteArrayLiteral("lazy")}, {}, SortedIdVector{ m_id7 });
    // Matches content and filename
    addRow("Lazy both",        {QByteArrayLiteral("lazy")},                             { m_id7 }, { m_id7 });
    addRow("Easy both",        {QByteArrayLiteral("easy")},                             { m_id8 }, { m_id8 });
}

void QueryTest::testTermPhrase()
{
    QFETCH(QByteArrayList, phrase);
    QFETCH(QVector<quint64>, contentMatches);
    QFETCH(QVector<quint64>, filenameMatches);

    QVector<EngineQuery> queries;
    for (const QByteArray& term : phrase) {
        queries << EngineQuery(term);
    }
    EngineQuery q(queries, EngineQuery::Phrase);

    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), contentMatches);

    queries.clear();
    const QByteArray fPrefix = QByteArrayLiteral("F");
    for (QByteArray term : phrase) {
        term = fPrefix + term;
        queries << EngineQuery(term);
    }
    EngineQuery qf(queries, EngineQuery::Phrase);
    QCOMPARE(tr.exec(qf), filenameMatches);
}

void QueryTest::testTagTermAnd_data()
{
    QTest::addColumn<QByteArrayList>("terms");
    QTest::addColumn<QVector<quint64>>("matchIds");

    QTest::addRow("Simple match") << QByteArrayList({"one", "four"})
        << QVector<quint64> { m_id5, m_id6 };
    QTest::addRow("Only one") << QByteArrayList({"one", "f1"})
        << QVector<quint64> { m_id5 };
    QTest::addRow("Also from phrase") << QByteArrayList({"two", "three"})
        << QVector<quint64> { m_id5, m_id6 };
}

void QueryTest::testTagTermAnd()
{
    insertTagDocuments();
    QFETCH(QByteArrayList, terms);
    QFETCH(QVector<quint64>, matchIds);

    QByteArray prefix{"TA"};
    QVector<EngineQuery> queries;
    for (const QByteArray& term : terms) {
        queries << EngineQuery(prefix + term);
    }

    EngineQuery q(queries, EngineQuery::And);

    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), matchIds);
}

void QueryTest::testTagTermPhrase_data()
{
    QTest::addColumn<QByteArrayList>("terms");
    QTest::addColumn<QVector<quint64>>("matchIds");

    QTest::addRow("Simple match") << QByteArrayList({"one"})
        << QVector<quint64> { m_id5, m_id6 };
    QTest::addRow("Apart") << QByteArrayList({"two", "four"})
        << QVector<quint64> { };
    QTest::addRow("Adjacent") << QByteArrayList({"three", "four"})
        << QVector<quint64> { };
    QTest::addRow("Only phrase") << QByteArrayList({"two", "three"})
        << QVector<quint64> { m_id6 };
}

void QueryTest::testTagTermPhrase()
{
    insertTagDocuments();
    QFETCH(QByteArrayList, terms);
    QFETCH(QVector<quint64>, matchIds);

    QByteArray prefix{"TA"};
    QVector<EngineQuery> queries;
    for (const QByteArray& term : terms) {
        queries << EngineQuery(prefix + term);
    }

    EngineQuery q(queries, EngineQuery::Phrase);

    Transaction tr(db, Transaction::ReadOnly);
    auto res = tr.exec(q);
    QCOMPARE(res, matchIds);
}

QTEST_MAIN(QueryTest)

#include "querytest.moc"
