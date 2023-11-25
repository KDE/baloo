/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2023 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "database.h"
#include "transaction.h"
#include "document.h"
#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>
#include <termgenerator.h>

using namespace Baloo;

class TermGeneratorBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void benchmarkTermListEnglish();
    void benchmarkTermListCJK();
    void benchmarkIndexEnglish();
    void benchmarkIndexCJK();

private:
    void benchmarkIndexText(const QString &text);

    QString m_englishString;
    QString m_cjkString;
};

void TermGeneratorBenchmark::initTestCase() {
    // In the worst case, we may run this against a 10MB text file with a single line
    // due the limit in extractor.
    const QString englishSentence = QString::fromUtf8("The quick brown fox jumps over the lazy dog.");
    m_englishString = englishSentence.repeated(10 * 1024 * 1024 / englishSentence.toUtf8().size());
    const QString cjkSentence = QString::fromUtf8(
        "我能吞下玻璃而不伤身体。" \
        "私はガラスを食べられます。それは私を傷つけません。" \
        "나는 유리를 먹을 수 있어요. 그래도 아프지 않아요");
    m_cjkString = cjkSentence.repeated(10 * 1024 * 1024 / cjkSentence.toUtf8().size());
}

void TermGeneratorBenchmark::benchmarkIndexText(const QString &text) {
    QTemporaryDir dir;
    Database db(dir.path());
    db.open(Baloo::Database::CreateDatabase);
    quint64 dirId = filePathToId(QFile::encodeName(dir.path()));
    const QString filePath(dir.path() + QStringLiteral("/file"));
    QByteArray url = QFile::encodeName(filePath);

    unsigned int i = 1;
    QBENCHMARK {
        Transaction tr(db, Transaction::ReadWrite);

        Document doc;
        doc.setId(i++);
        doc.setParentId(dirId);
        doc.setContentIndexing(true);
        doc.addFileNameTerm("link");
        doc.setUrl(url);
        TermGenerator term(doc);
        term.indexText(text);

        tr.addDocument(doc);
        tr.commit();
    }
}

void TermGeneratorBenchmark::benchmarkTermListEnglish()
{
    QBENCHMARK {
        TermGenerator::termList(m_englishString);
    }
}

void TermGeneratorBenchmark::benchmarkTermListCJK()
{
    QBENCHMARK {
        TermGenerator::termList(m_cjkString);
    }
}

void TermGeneratorBenchmark::benchmarkIndexEnglish()
{
    benchmarkIndexText(m_englishString);
}

void TermGeneratorBenchmark::benchmarkIndexCJK()
{
    benchmarkIndexText(m_cjkString);
}

QTEST_MAIN(TermGeneratorBenchmark)

#include "termgeneratorbenchmark.moc"
