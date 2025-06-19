/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "extractorprocess.h"
#include "testsconfig.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

namespace Baloo {
namespace Test {

namespace {
QString getWorkerPath()
{
    return QStandardPaths::findExecutable(QStringLiteral("extractorprocess_fake"),
		                          {QStringLiteral(EXTRACTOR_TESTS_HELPER_PATH)});
}
} // <anonymous> namespace

class ExtractorProcessTest : public QObject
{
    Q_OBJECT
public:
    ExtractorProcessTest()
        : m_workerPath(getWorkerPath())
    {}

private Q_SLOTS:
    void initTestCase();

    void testSignals();
    void testFailedSignal();
    void testExit();
    void testExit_data();

private:
    QString m_workerPath;
};

void ExtractorProcessTest::initTestCase()
{
    QVERIFY(!m_workerPath.isEmpty());
}

void ExtractorProcessTest::testSignals()
{
    Baloo::ExtractorProcess extractor{m_workerPath};
    QSignalSpy spyS(&extractor, &ExtractorProcess::startedIndexingFile);
    QSignalSpy spyF(&extractor, &ExtractorProcess::finishedIndexingFile);
    QSignalSpy spyD(&extractor, &ExtractorProcess::done);

    extractor.index({123, 125});
    QVERIFY(spyD.wait());

    extractor.index({127, 129});
    QVERIFY(spyD.wait());
    QCOMPARE(spyF.size(), 4);
}

void ExtractorProcessTest::testFailedSignal()
{
    Baloo::ExtractorProcess extractor{m_workerPath};
    QSignalSpy spyS(&extractor, &ExtractorProcess::startedIndexingFile);
    QSignalSpy spyF(&extractor, &ExtractorProcess::finishedIndexingFile);
    QSignalSpy spyD(&extractor, &ExtractorProcess::done);

    extractor.index({123, 23});
    QVERIFY(spyD.wait());
    QCOMPARE(spyS.size(), 2);
    QCOMPARE(spyF.size(), 2);
    QCOMPARE(spyF.at(0).at(0).toString(), QStringLiteral("123"));
    QCOMPARE(spyF.at(0).at(1).toBool(), true); // success
    QCOMPARE(spyF.at(1).at(0).toString(), QStringLiteral("23"));
    QCOMPARE(spyF.at(1).at(1).toBool(), false); // failed
}

void ExtractorProcessTest::testExit()
{
    QFETCH(quint64, exitCode);

    Baloo::ExtractorProcess extractor{m_workerPath};
    QSignalSpy spyS(&extractor, &ExtractorProcess::startedIndexingFile);
    QSignalSpy spyF(&extractor, &ExtractorProcess::finishedIndexingFile);
    QSignalSpy spyD(&extractor, &ExtractorProcess::done);
    QSignalSpy spyX(&extractor, &ExtractorProcess::failed);

    extractor.index({121});
    QVERIFY(spyD.wait());

    extractor.index({123, exitCode, 23});
    QVERIFY(spyX.wait());
    QCOMPARE(spyS.size(), 3);
    QCOMPARE(spyF.size(), 2);
    QCOMPARE(spyD.size(), 1);
    QCOMPARE(spyS.at(0).at(0).toString(), QStringLiteral("121"));
    QCOMPARE(spyS.at(1).at(0).toString(), QStringLiteral("123"));
    QCOMPARE(spyS.at(2).at(0).toString(), QString::number(exitCode));
}

void ExtractorProcessTest::testExit_data()
{
    QTest::addColumn<quint64>("exitCode");

    QTest::newRow("Crash") << quint64(0);
    QTest::newRow("DB error") << quint64(1);
}

} // namespace Test
} // namespace Baloo

using namespace Baloo::Test;

QTEST_GUILESS_MAIN(ExtractorProcessTest)

#include "extractorprocesstest.moc"
