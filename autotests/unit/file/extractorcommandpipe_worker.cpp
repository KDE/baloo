/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QFile>
#include <QSocketNotifier>
#include <QTimer>

#include "baloodebug.h"
#include "extractor/commandpipe.h"

#include <unistd.h> //for STDIN_FILENO

using namespace Qt::StringLiterals;

struct Context {
    using WorkerPipe = Baloo::Private::WorkerPipe;
    Context();

    void processOne();

    QSocketNotifier inputNotifier;
    QFile input;
    QFile output;
    WorkerPipe worker;
    QVector<quint64> pendingIds;
    bool indexFast = false;
};

Context::Context()
    : inputNotifier(STDIN_FILENO, QSocketNotifier::Read)
    , worker(&input, &output)
{
    std::ignore = input.open(STDIN_FILENO, QIODevice::ReadOnly | QIODevice::Unbuffered);
    std::ignore = output.open(STDOUT_FILENO, QIODevice::WriteOnly | QIODevice::Unbuffered);

    QObject::connect(&inputNotifier, &QSocketNotifier::activated,
                     &worker, &WorkerPipe::processIdData);

    QObject::connect(&worker, &WorkerPipe::inputEnd, []() {
        qCInfo(BALOO) << "[TEST] Pipe closed, exiting";
        QCoreApplication::quit();
    });

    QObject::connect( //
        &worker,
        &WorkerPipe::newDocumentIds,
        [this](const QVector<quint64> &ids) {
            qCInfo(BALOO) << "[TEST] Processing" << ids << "...";
            pendingIds = ids;
            QTimer::singleShot(0, [this]() {
                processOne();
            });
        });
}

void Context::processOne()
{
    using FileIndexStatus = Baloo::IndexResult::FileStatus;

    if (pendingIds.empty()) {
        worker.batchFinished();
        qCInfo(BALOO) << "[TEST] Processing done";
        return;
    }

    auto id = pendingIds.takeFirst();
    qCInfo(BALOO) << "[TEST] Processing" << id;

    const auto url = QString::number(id);
    worker.urlStarted(url);
    worker.urlProcessed(url, true, FileIndexStatus::Successful);

    QTimer::singleShot(0, [this]() {
        processOne();
    });
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Context context;

    return app.exec();
}
