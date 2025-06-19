/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCrash>
#include <QCoreApplication>
#include <QFile>
#include <QSocketNotifier>
#include <QTimer>

#include "baloodebug.h"
#include "extractor/commandpipe.h"

#include <signal.h>
#include <sys/resource.h>
#include <unistd.h> //for STDIN_FILENO

struct Context {
    using WorkerPipe = Baloo::Private::WorkerPipe;
    Context();

    void processOne();

    QSocketNotifier notifier;
    QFile input;
    QFile output;
    WorkerPipe worker;
    QVector<quint64> pendingIds;
};

Context::Context()
    : notifier(STDIN_FILENO, QSocketNotifier::Read)
    , worker(&input, &output)
{
    input.open(STDIN_FILENO, QIODevice::ReadOnly | QIODevice::Unbuffered);
    output.open(STDOUT_FILENO, QIODevice::WriteOnly | QIODevice::Unbuffered);

    QObject::connect(&notifier, //
                     &QSocketNotifier::activated,
                     &worker,
                     &WorkerPipe::processIdData);

    QObject::connect(&worker, //
                     &WorkerPipe::newDocumentIds,
                     [this](const QVector<quint64> &ids) {
                         qCInfo(BALOO) << "Processing " << ids << " ...";
                         pendingIds = ids;
                         QTimer::singleShot(0, [this]() {
                             processOne();
                         });
                     });

    QObject::connect(&worker, &WorkerPipe::inputEnd, &QCoreApplication::quit);
}

void Context::processOne()
{
    if (pendingIds.empty()) {
        worker.batchFinished();
        qCInfo(BALOO) << "Processing done";
        return;
    }

    auto id = pendingIds.takeFirst();
    worker.urlStarted(QString::number(id));

    if (id == 0) {
        raise(SIGSEGV);
    } else if (id == 1) {
        exit(1);
    } else if (id == 2) {
        exit(2);
    } else if (id < 100) {
        worker.urlFailed(QString::number(id));
    } else {
        worker.urlFinished(QString::number(id));
    }

    QTimer::singleShot(0, [this]() {
        processOne();
    });
}

int main(int argc, char* argv[])
{
    using Baloo::Private::WorkerPipe;

    // Disable writing core dumps, we may crash on purpose
    struct rlimit corelimit{0, 0};
    setrlimit(RLIMIT_CORE, &corelimit);
    // DrKonqi blocks the signal and causes timeout of the QSignalSpy
    KCrash::setDrKonqiEnabled(false);
    QCoreApplication app(argc, argv);

    Context context;

    return app.exec();
}
