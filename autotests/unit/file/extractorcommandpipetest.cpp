/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "extractor/commandpipe.h"
#include "testsconfig.h"

#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

Q_DECLARE_METATYPE(QProcess::ExitStatus)

namespace Baloo {
namespace Test {

class ExtractorCommandPipeTest : public QObject
{
    Q_OBJECT

public:
    ExtractorCommandPipeTest() : m_controller(&m_worker, &m_worker) {}

private Q_SLOTS:

    void initTestCase();

    void init();
    void cleanup();

    void emptyBatch();
    void singleBatch();
    void singleBatch_data();
    void multipleBatch();
    void multipleSequentialBatches();
    void closePipe();
    void changeSpeed();

private:
    QProcess m_worker;
    QString m_workerPath;
    Baloo::Private::ControllerPipe m_controller;
};

void ExtractorCommandPipeTest::initTestCase()
{
    m_workerPath = QStandardPaths::findExecutable(QStringLiteral("extractorcommandpipe_worker"), {QStringLiteral(EXTRACTOR_TESTS_HELPER_PATH)});
    QVERIFY(!m_workerPath.isEmpty());
}

void ExtractorCommandPipeTest::init()
{
    connect(&m_worker, &QProcess::readyRead, &m_controller, &Baloo::Private::ControllerPipe::processStatusData);

    m_worker.setProgram(m_workerPath);
    m_worker.setProcessChannelMode(QProcess::ForwardedErrorChannel);

    m_worker.start(QIODevice::Unbuffered | QIODevice::ReadWrite);
    m_worker.waitForStarted();
    m_worker.setReadChannel(QProcess::StandardOutput);
}

void ExtractorCommandPipeTest::cleanup()
{
    m_worker.closeWriteChannel();
    m_worker.waitForFinished();
    m_worker.close();
}

void ExtractorCommandPipeTest::emptyBatch()
{
    QSignalSpy spy(&m_controller, &Baloo::Private::ControllerPipe::batchFinished);

    m_controller.processIds({});

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
}

void ExtractorCommandPipeTest::singleBatch()
{
    QFETCH(QVector<quint64>, ids);

    QStringList processedUrls;
    auto connection = connect( //
        &m_controller,
        &Baloo::Private::ControllerPipe::urlProcessed,
        [&processedUrls](const QString &url, bool, Baloo::IndexResult::FileStatus) {
            processedUrls.append(url);
            // qDebug() << "fileFinished" << url;
        });

    QSignalSpy spy(&m_controller, &Baloo::Private::ControllerPipe::batchFinished);

    m_controller.processIds(ids);

    QVERIFY(spy.wait());

    QCOMPARE(processedUrls.size(), ids.size());
    disconnect(connection);
}

void ExtractorCommandPipeTest::singleBatch_data()
{
    QTest::addColumn<QVector<quint64>>("ids");

    QTest::addRow("singleEntry") << QVector<quint64>{3};
    QTest::addRow("multipleEntry") << QVector<quint64>{1, 2, 3, 4, 5, 100, 101, 102, 103, 1000};

    QVector<quint64> longlist;
    for (quint64 i = 0; i < 50; i++) {
        longlist.append(i);
    }
    QTest::addRow("manyEntries") << longlist;
}

void ExtractorCommandPipeTest::multipleBatch()
{
    QSignalSpy spy(&m_controller, &Baloo::Private::ControllerPipe::batchFinished);

    m_controller.processIds({1, 11});
    m_controller.processIds({2, 22});
    m_controller.processIds({3, 33});

    while (spy.count() < 3) {
        QVERIFY(spy.wait(500));
    }
    QCOMPARE(spy.count(), 3);
}

void ExtractorCommandPipeTest::multipleSequentialBatches()
{
    QSignalSpy spy(&m_controller, &Baloo::Private::ControllerPipe::batchFinished);

    m_controller.processIds({1, 11});
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    m_controller.processIds({2, 22});
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 2);

    m_controller.processIds({3, 33});
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 3);
}

void ExtractorCommandPipeTest::closePipe()
{
    qRegisterMetaType<QProcess::ExitStatus>();
    QSignalSpy spy(&m_worker, &QProcess::finished);

    m_worker.closeWriteChannel();
    QVERIFY(spy.wait());
}

void ExtractorCommandPipeTest::changeSpeed()
{
    QSignalSpy spy(&m_controller, &Baloo::Private::ControllerPipe::urlProcessed);

    m_controller.setIndexSpeed(Baloo::Private::IndexSpeed::Fast);
    m_controller.processIds({1});
    QVERIFY(spy.wait());

    m_controller.setIndexSpeed(Baloo::Private::IndexSpeed::Slow);
    m_controller.processIds({2, 3});
    while (spy.count() < 3) {
        QVERIFY(spy.wait(500));
    }

    m_controller.setIndexSpeed(Baloo::Private::IndexSpeed::Fast);
    m_controller.setIndexSpeed(Baloo::Private::IndexSpeed::Slow);
    m_controller.setIndexSpeed(Baloo::Private::IndexSpeed::Fast);
    m_controller.processIds({2034, 5, 6});

    while (spy.count() < 4) {
        QVERIFY(spy.wait(500));
    }

    m_controller.setIndexSpeed(Baloo::Private::IndexSpeed::Slow);
    while (spy.count() < 6) {
        QVERIFY(spy.wait(500));
    }

    m_controller.setIndexSpeed(Baloo::Private::IndexSpeed::Fast);
    m_controller.processIds({7});
    while (spy.count() < 7) {
        QVERIFY(spy.wait(500));
    }

    // Per file, QVariantList of (url, changed, status)
    QCOMPARE(spy.at(0).at(0).toString(), u"F_1");
    QCOMPARE(spy.at(1).at(0).toString(), u"S_2");
    QCOMPARE(spy.at(2).at(0).toString(), u"S_3");
    QCOMPARE(spy.at(3).at(0).toString(), u"F_2034");
    QCOMPARE(spy.at(4).at(0).toString(), u"S_5");
    QCOMPARE(spy.at(5).at(0).toString(), u"S_6");
    QCOMPARE(spy.at(6).at(0).toString(), u"F_7");
}

} // namespace Test
} // namespace Baloo

using namespace Baloo::Test;

QTEST_GUILESS_MAIN(ExtractorCommandPipeTest)

#include "extractorcommandpipetest.moc"
