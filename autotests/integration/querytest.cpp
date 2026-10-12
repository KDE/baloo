/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "query.h"
#include "database.h"
#include "document.h"
#include "enginequery.h"
#include "filenamequery.h"
#include "idutils.h"
#include "termgenerator.h"
#include "transaction.h"

#include <memory>

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;
using namespace Qt::StringLiterals;

char *toString(const QVector<quint64> &idlist)
{
    QByteArray text("IDs[");
    text += QByteArray::number(idlist.size()) + "]:";
    for (auto id : idlist) {
        text += " " + QByteArray::number(id, 16);
    }
    return qstrdup(text.data());
}

namespace {
template<typename QueryT>
QVector<quint64> execQuery(const Transaction &tr, const QueryT &query)
{
    std::unique_ptr<PostingIterator> it{tr.postingIterator(query)};
    if (!it) {
        return {};
    }

    QVector<quint64> results;
    while (it->next()) {
        results << it->docId();
    }
    return results;
}

struct PropertyTerm {
    QByteArray prefix;
    QString text;
    bool asContent;
};
} // namespace

class QueryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // Make timestamp parsing independent of host locale
        qputenv("TZ", "UTC");

        dir = std::make_unique<QTemporaryDir>();
        m_parentId = filePathToId(QFile::encodeName(dir->path()));
        for (size_t i = 0; i < m_ids.size(); i++) {
            m_ids[i] = m_parentId + i;
        }
    }

    void init()
    {
        dbDir = std::make_unique<QTemporaryDir>();
        db = std::make_unique<Database>(dbDir->path());
        QCOMPARE(db->open(Database::CreateDatabase), Database::OpenResult::Success);
        setenv("BALOO_DB_PATH", dbDir->path().toStdString().c_str(), 1);

        insertDocuments();
        insertTagDocuments();
        insertPropertyDocuments();
    }

    void cleanup() {
        db.reset();
        dbDir.reset();
    }

    void testTermEqual();
    void testTermStartsWith();
    void testContentPhrase_data();
    void testContentPhrase();
    void testFilenamePhrase_data();
    void testFilenamePhrase();
    void testPhrases_dataCommon();

    void testTagTerm_data();
    void testTagTerm();
    void testTagTermPhrase_data();
    void testTagTermPhrase();

    void testSearchstringParser();
    void testSearchstringParser_data();

private:
    std::unique_ptr<QTemporaryDir> dir;
    std::unique_ptr<QTemporaryDir> dbDir;
    std::unique_ptr<Database> db;
    quint64 m_parentId = 0;

    void insertDocuments();
    void addDocument(Transaction *tr, const QStringList &text, quint64 id, const QString &fileName)
    {
        Document doc;
        QString url = dir->path() + QLatin1Char('/') + fileName;
        doc.setUrl(QFile::encodeName(url));

        TermGenerator tg(doc);
        for (const auto &block : text) {
            tg.indexText(block);
        }
        tg.indexFileNameText(fileName);
        doc.setId(id);
        doc.setParentId(m_parentId);
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
        doc.setParentId(m_parentId);
        doc.setUrl(QFile::encodeName(newName));

        tr->replaceDocument(doc, FileNameTerms | DocumentUrl);
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
        doc.setParentId(m_parentId);
        doc.setMTime(3600 * 24 + 30);
        doc.setCTime(3600 * 24 + 40);

        tr->addDocument(doc);
    }

    void insertPropertyDocuments();
    void addPropertyDocument(Transaction *tr, const QList<PropertyTerm> &properties, quint64 id, const QString &fileName)
    {
        Document doc;
        QString url = dir->path() + QLatin1Char('/') + fileName;
        doc.setUrl(QFile::encodeName(url));

        TermGenerator tg(doc);
        for (const auto &property : properties) {
            const QByteArray prefix{'X' + property.prefix + '-'};
            tg.indexText(property.text, prefix);
            if (property.asContent) {
                tg.indexText(property.text);
            }
        }
        tg.indexFileNameText(fileName);
        doc.setId(id);
        doc.setParentId(m_parentId);
        doc.setMTime(1000);
        doc.setCTime(1200);

        tr->addDocument(doc);
    }

    std::array<quint64, 15> m_ids{};
};

void QueryTest::insertDocuments()
{
    Transaction tr(db.get(), Transaction::ReadWrite);
    // clang-format off
    addDocument(&tr, {u"The quick brown fox jumped over the crazy dog"_s},        m_ids[1],  u"file1.txt"_s);
    addDocument(&tr, {u"The quick brown fox jumped over the lazy dog"_s},         m_ids[7],  u"file7_lazy"_s);
    addDocument(&tr, {u"A quick brown fox ran around a easy dog"_s},              m_ids[8],  u"file8_dog"_s);
    addDocument(&tr, {u"The night is dark and full of terror"_s},                 m_ids[2],  u"file2"_s);
    addDocument(&tr, {u"Don't feel sorry for yourself. Only assholes do that"_s}, m_ids[3],  u"file3"_s);
    addDocument(&tr, {u"Only the dead stay 17 forever. crazy"_s},                 m_ids[4],  u"file4"_s);
    addDocument(&tr, {u"Some content with isolated dot . Test it"_s},             m_ids[9],  u"file - with hyphen.txt"_s);
    addDocument(&tr, {u"File with URL: https://www.kde.org"_s},                   m_ids[10], u"url:file.txt"_s);
    addDocument(&tr, {u"First page."_s, u"Second page."_s},                       m_ids[13], u"two_page.txt"_s);
    addDocument(&tr, {u"Soft hy\u00ad"_s, u"phen."_s},                            m_ids[14], u"two_page_hyphenated.txt"_s);
    // clang-format on

    renameDocument(&tr, m_ids[8], QStringLiteral("file8_easy"));
    tr.commit();
}

void QueryTest::insertTagDocuments()
{
    Transaction tr(db.get(), Transaction::ReadWrite);
    addTagDocument(&tr, //
                   {u"One"_s, u"Two"_s, u"Three"_s, u"Four"_s, u"F1"_s},
                   m_ids[5],
                   u"tagFile1"_s);
    addTagDocument(&tr, //
                   {u"One"_s, u"Two-Three"_s, u"Four"_s, u"F2"_s},
                   m_ids[6],
                   u"tagFile2"_s);
    tr.commit();
}

void QueryTest::insertPropertyDocuments()
{
    Transaction tr(db.get(), Transaction::ReadWrite);
    addPropertyDocument(&tr, //
                        {
                            {"9", u"The Artist Number 1"_s, true}, // artist
                            {"10", u"Album Number 2"_s, true}, // album
                        },
                        m_ids[11],
                        u"propertyFile.mp3"_s);
    addPropertyDocument(&tr, //
                        {
                            {"9", u"Photographer Name"_s, true}, // artist
                            {"31", u"Great Camera Model 2"_s, false}, // model
                            {"26", u"1234"_s, false}, // width
                            {"40", u"0.234"_s, false}, // photoExposureTime
                        },
                        m_ids[12],
                        u"propertyFile.jpg"_s);
    tr.commit();
}

void QueryTest::testTermEqual()
{
    EngineQuery q({EngineQuery::PhraseTerm("the", EngineQuery::Equal)});

    QVector<quint64> result{m_ids[1], m_ids[2], m_ids[4], m_ids[7], m_ids[11]};
    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), result);
}

void QueryTest::testTermStartsWith()
{
    EngineQuery q({EngineQuery::PhraseTerm("for", EngineQuery::StartsWith)});

    QVector<quint64> result{m_ids[3], m_ids[4]};
    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), result);
}

void QueryTest::testPhrases_dataCommon()
{
    QTest::addColumn<QString>("phrase");
    QTest::addColumn<QVector<quint64>>("contentMatches");
    QTest::addColumn<QVector<quint64>>("filenameMatches");

    auto addRow = [](const char *name, //
                     const QString &phrase,
                     const QVector<quint64> contentMatches,
                     const QVector<quint64> filenameMatches) {
        QTest::addRow("%s", name) << phrase << contentMatches << filenameMatches;
    };

    // Content matches
    // clang-format off
    addRow("Crazy dog",        {u"crazy dog"_s},  {m_ids[1]},                     {});
    addRow("Lazy dog",         {u"lazy dog"_s},   {m_ids[7]},                     {});
    addRow("Brown fox",        {u"brown fox"_s},  {m_ids[1], m_ids[7], m_ids[8]}, {});
    addRow("Dog",              {u"dog"_s},        {m_ids[1], m_ids[7], m_ids[8]}, {});
    // Filename matches
    addRow("Crazy dog file 1", {u"file1"_s},      {},                             {m_ids[1]});
    addRow("Crazy dog file 2", {u"file1 txt"_s},  {},                             {m_ids[1]});
    addRow("Lazy dog file 1",  {u"file7"_s},      {},                             {m_ids[7]});
    addRow("Lazy dog file 2",  {u"file7 lazy"_s}, {},                             {m_ids[7]});
    // Matches content and filename
    addRow("Lazy both",        {u"lazy"_s},       {m_ids[7]},                     {m_ids[7]});
    addRow("Easy both",        {u"easy"_s},       {m_ids[8]},                     {m_ids[8]});
    // clang-format off
}

void QueryTest::testContentPhrase_data()
{
    testPhrases_dataCommon();
}

void QueryTest::testContentPhrase()
{
    QFETCH(QString, phrase);
    QFETCH(QVector<quint64>, contentMatches);

    QVector<EngineQuery::PhraseTerm> queries;
    for (const QByteArray& term : TermGenerator::termList(phrase)) {
        queries.emplaceBack(term, EngineQuery::Equal);
    }
    EngineQuery q(queries);

    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), contentMatches);
}

void QueryTest::testFilenamePhrase_data()
{
    testPhrases_dataCommon();
}

void QueryTest::testFilenamePhrase()
{
    QFETCH(QString, phrase);
    QFETCH(QVector<quint64>, filenameMatches);

    FilenameQuery qf(phrase);

    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, qf), filenameMatches);
}

void QueryTest::testTagTerm_data()
{
    QTest::addColumn<QByteArray>("term");
    QTest::addColumn<QVector<quint64>>("matchIds");

    QTest::addRow("Simple match") << QByteArray("one")
        << QVector<quint64> { m_ids[5], m_ids[6] };
    QTest::addRow("Only one") << QByteArray("f1")
        << QVector<quint64> { m_ids[5] };
    QTest::addRow("Also from phrase") << QByteArray("three")
        << QVector<quint64> { m_ids[5], m_ids[6] };
}

void QueryTest::testTagTerm()
{
    QFETCH(QByteArray, term);
    QFETCH(QVector<quint64>, matchIds);

    QByteArray prefix{"TA"};
    EngineQuery q({EngineQuery::PhraseTerm(prefix + term, EngineQuery::Equal)});

    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), matchIds);
}

void QueryTest::testTagTermPhrase_data()
{
    QTest::addColumn<QByteArrayList>("terms");
    QTest::addColumn<QVector<quint64>>("matchIds");

    QTest::addRow("Simple match") << QByteArrayList({"one"})
        << QVector<quint64> { m_ids[5], m_ids[6] };
    QTest::addRow("Apart") << QByteArrayList({"two", "four"})
        << QVector<quint64> { };
    QTest::addRow("Adjacent") << QByteArrayList({"three", "four"})
        << QVector<quint64> { };
    QTest::addRow("Only phrase") << QByteArrayList({"two", "three"})
        << QVector<quint64> { m_ids[6] };
}

void QueryTest::testTagTermPhrase()
{
    QFETCH(QByteArrayList, terms);
    QFETCH(QVector<quint64>, matchIds);

    QByteArray prefix{"TA"};
    QVector<EngineQuery::PhraseTerm> queries;
    for (const QByteArray& term : terms) {
        queries.emplaceBack(prefix + term, EngineQuery::Equal);
    }

    EngineQuery q(queries);

    Transaction tr(db.get(), Transaction::ReadOnly);
    auto res = execQuery(tr, q);
    QCOMPARE(res, matchIds);
}

void QueryTest::testSearchstringParser()
{
    QFETCH(QString, searchString);
    QFETCH(QStringList, expectedFiles);

    Query q;
    q.setSearchString(searchString);

    auto res = q.exec();
    QStringList matches;
    while (res.next()) {
        auto path = res.filePath();
        auto name = path.section(QLatin1Char('/'), -1, -1);
        matches.append(name);
    }
    QEXPECT_FAIL("Match 'dot . Test'", "Bug 407664: Tries to match isolated dot", Continue);
    QEXPECT_FAIL("Match 'file - with hyphen.txt'", "Bug 407664: Tries to match hyphen", Continue);
    QEXPECT_FAIL("Match '\"page. second\"'", "Page break breaks matching", Continue);
    QEXPECT_FAIL("Match '\"soft hyphen\"'", "Soft hyphen not joined", Continue);
    QCOMPARE(matches, expectedFiles);
}

void QueryTest::testSearchstringParser_data()
{
    QTest::addColumn<QString>("searchString");
    QTest::addColumn<QStringList>("expectedFiles");

    auto addRow = [](const QString& searchString,
                     const QStringList& filenameMatches)
    {
        QTest::addRow("Match '%s'", qPrintable(searchString)) << searchString << filenameMatches;
    };

    addRow(QStringLiteral("crazy"), { QStringLiteral("file1.txt"), QStringLiteral("file4") });
    addRow(QStringLiteral("content:crazy"), { QStringLiteral("file1.txt"), QStringLiteral("file4") });
    addRow(QStringLiteral("content:dog"), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy"), QStringLiteral("file8_easy") });
    addRow(QStringLiteral("filename:dog"), {});
    addRow(QStringLiteral("filename:easy"), { QStringLiteral("file8_easy") });
    addRow(QStringLiteral("content:for"), { QStringLiteral("file3"), QStringLiteral("file4") });
    addRow(QStringLiteral("content=for"), { QStringLiteral("file3") });
    addRow(QStringLiteral("content=don't"), { QStringLiteral("file3") });
    addRow(QStringLiteral("content=yourself"), { QStringLiteral("file3") });
    addRow(QStringLiteral("content=\"over the\""), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy") });
    addRow(QStringLiteral("content=\"over the crazy dog\""), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("content=\"over the dog\""), {});
    addRow(QStringLiteral("quick AND crazy AND dog"), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("quick crazy dog"), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("\"quick brown\" dog"), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy"), QStringLiteral("file8_easy") });
    addRow(QStringLiteral("\"quick brown\" the dog"), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy") });
    addRow(QStringLiteral("\"quick brown\" content=\"the dog\""), {});
    addRow(QStringLiteral("\"quick brown\" content=\"'the dog'\""), {});
    addRow(QStringLiteral("\"quick brown\" content:\"the dog\""), {});
    addRow(QStringLiteral("\"quick brown\" content:\"'the dog'\""), {});
    addRow(QStringLiteral("\"quick brown\" \"the crazy dog\""), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("content=for OR filename:eas"), { QStringLiteral("file3"), QStringLiteral("file8_easy") });
    addRow(QStringLiteral("for sorry"), { QStringLiteral("file3") });
    addRow(QStringLiteral("over OR terror"), {  QStringLiteral("file1.txt"), QStringLiteral("file2"), QStringLiteral("file7_lazy") });

    addRow(QStringLiteral("tag:f1"), {  QStringLiteral("tagFile1") });
    addRow(QStringLiteral("tag:f2"), {  QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:one"), {  QStringLiteral("tagFile1"), QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:two"), {  QStringLiteral("tagFile1"), QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:two AND tag:three"), {  QStringLiteral("tagFile1"), QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:two-three"), { QStringLiteral("tagFile2") });

    addRow(QStringLiteral("filename:hyphen"), {u"file - with hyphen.txt"_s, u"two_page_hyphenated.txt"_s});
    addRow(QStringLiteral("file with hyphen.txt"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("file - with hyphen.txt"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("\"file - with hyphen.txt\""), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("content:dot"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("dot . Test"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("\"dot . Test\""), { QStringLiteral("file - with hyphen.txt") });
    // clang-format off
    addRow(u"file.txt"_s,                {u"url:file.txt"_s});
    addRow(u"\"url:file\""_s,            {u"url:file.txt"_s});
    addRow(u"url_file"_s,                {u"url:file.txt"_s});
    addRow(u"url:file"_s,                {u"url:file.txt"_s});
    addRow(u"www.kde.org"_s,             {u"url:file.txt"_s});
    addRow(u"\"https://www.kde.org\""_s, {u"url:file.txt"_s});
    addRow(u"https_//www.kde.org"_s,     {u"url:file.txt"_s});
    addRow(u"https://www.kde.org"_s,     {u"url:file.txt"_s});
    addRow(u"modified:0"_s,              {}); // invalid date string
    addRow(u"modified:1970-01-02"_s,     {u"tagFile1"_s, u"tagFile2"_s});
    addRow(u"mtime:1970-01-02"_s,        {u"tagFile1"_s, u"tagFile2"_s});
    addRow(u"mtime:1970-01-01 sorry"_s,  {u"file3"_s});
    addRow(u"mtime:1960-01-01"_s,        {});
    addRow(u"modified=1970-01-02"_s,     {u"tagFile1"_s, u"tagFile2"_s});
    addRow(u"modified>=1970-01-02"_s,    {u"tagFile1"_s, u"tagFile2"_s});
    addRow(u"modified>1970-01-02"_s,     {u"tagFile1"_s, u"tagFile2"_s});
    addRow(u"modified>1970-01-03"_s,     {});
    addRow(u"mtime>1960-01-03 tag:f1"_s, {u"tagFile1"_s});
    addRow(u"mimetype=text tag:f1"_s,    {u"tagFile1"_s});
    addRow(u"mimetype=plain tag:f1"_s,   {u"tagFile1"_s});
    addRow(u"mimetype:pla tag:f1"_s,     {u"tagFile1"_s});
    addRow(u"mimetype:text/plain"_s,     {u"tagFile1"_s, u"tagFile2"_s});
    addRow(u"mimetype:plain/text"_s,     {});
    addRow(u"the artist"_s,              {u"propertyFile.mp3"_s});
    addRow(u"\"the artist\""_s,          {u"propertyFile.mp3"_s});
    addRow(u"artist:\"the artist\""_s,   {u"propertyFile.mp3"_s});
    addRow(u"photographer"_s,            {u"propertyFile.jpg"_s});
    addRow(u"artist:photographer"_s,     {u"propertyFile.jpg"_s});
    addRow(u"model:camera"_s,            {u"propertyFile.jpg"_s});
    addRow(u"width:1234"_s,              {u"propertyFile.jpg"_s});
    addRow(u"width=1234"_s,              {u"propertyFile.jpg"_s});
    addRow(u"width>1230"_s,              {u"propertyFile.jpg"_s});
    addRow(u"photoExposureTime=0.2"_s,   {});
    addRow(u"photoExposureTime:0.2"_s,   {u"propertyFile.jpg"_s});
    addRow(u"photoExposureTime>0.2"_s,   {u"propertyFile.jpg"_s});
    addRow(u"photoExposureTime<0.3"_s,   {u"propertyFile.jpg"_s});
    addRow(u"camera"_s,                  {});
    addRow(u"first"_s,                   {u"two_page.txt"_s});
    addRow(u"first page. second"_s,      {u"two_page.txt"_s});
    addRow(u"\"first page.\""_s,         {u"two_page.txt"_s});
    addRow(u"\"page. second\""_s,        {u"two_page.txt"_s});
    addRow(u"soft"_s,                    {u"two_page_hyphenated.txt"_s});
    addRow(u"\"soft hyphen\""_s,         {u"two_page_hyphenated.txt"_s});
    // clang-format off
}

QTEST_MAIN(QueryTest)

#include "querytest.moc"
