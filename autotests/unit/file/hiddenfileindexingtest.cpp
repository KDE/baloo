/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fileindexerconfig.h"
#include "fileindexerconfigutils.h"
#include "modifiedfileindexer.h"
#include "newfileindexer.h"

#include "database.h"
#include "idutils.h"
#include "transaction.h"

#include <QTemporaryDir>
#include <QTest>

using namespace Baloo;
using namespace Baloo::Test;

class HiddenFileIndexingTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testNewFileInHiddenFolderIsLeftOut();
    void testModifiedFileInHiddenFolderIsLeftOut();

private:
    QString filePath(const QString &name) const
    {
        return m_files->path() + QLatin1Char('/') + name;
    }

    bool isIndexed(const QString &path)
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        return tr.hasDocument(filePathToId(QFile::encodeName(path)));
    }

    std::unique_ptr<QTemporaryDir> m_files;
    std::unique_ptr<QTemporaryDir> m_indexDir;
    std::unique_ptr<Database> m_db;
};

void HiddenFileIndexingTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void HiddenFileIndexingTest::init()
{
    // A plain file next to a hidden folder holding a file whose own name is plain too.
    m_files = createTmpFilesAndFolders(QStringList{
        QStringLiteral(".hidden/"),
        QStringLiteral(".hidden/inside.txt"),
        QStringLiteral("visible.txt"),
    });

    writeIndexerConfig(QStringList{m_files->path()}, QStringList(), QStringList(), false);

    m_indexDir = std::make_unique<QTemporaryDir>();
    m_db = std::make_unique<Database>(m_indexDir->path());
    QCOMPARE(m_db->open(Database::CreateDatabase), Database::OpenResult::Success);
}

void HiddenFileIndexingTest::cleanup()
{
    m_db.reset();
    m_indexDir.reset();
    m_files.reset();
}

void HiddenFileIndexingTest::testNewFileInHiddenFolderIsLeftOut()
{
    FileIndexerConfig config;
    const QString visible = filePath(QStringLiteral("visible.txt"));
    const QString insideHidden = filePath(QStringLiteral(".hidden/inside.txt"));

    NewFileIndexer indexer(m_db.get(), &config, QStringList{visible, insideHidden});
    indexer.run();

    QVERIFY(isIndexed(visible));
    QVERIFY(!isIndexed(insideHidden));
}

void HiddenFileIndexingTest::testModifiedFileInHiddenFolderIsLeftOut()
{
    FileIndexerConfig config;
    const QString visible = filePath(QStringLiteral("visible.txt"));
    const QString insideHidden = filePath(QStringLiteral(".hidden/inside.txt"));

    ModifiedFileIndexer indexer(m_db.get(), &config, QStringList{visible, insideHidden});
    indexer.run();

    QVERIFY(isIndexed(visible));
    QVERIFY(!isIndexed(insideHidden));
}

QTEST_GUILESS_MAIN(HiddenFileIndexingTest)

#include "hiddenfileindexingtest.moc"
