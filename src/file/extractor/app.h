/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXTRACTOR_APP_H
#define EXTRACTOR_APP_H

#include <QVector>
#include <QMimeDatabase>
#include <QDataStream>
#include <QTextStream>
#include <QSocketNotifier>
#include <QFile>

#include <KFileMetaData/ExtractorCollection>

#include "database.h"
#include "../fileindexerconfig.h"

class KIdleTime;
class QString;

namespace Baloo {

class Transaction;

class App : public QObject
{
    Q_OBJECT

public:
    explicit App(QObject* parent = nullptr);

private Q_SLOTS:
    void slotNewInput();
    void processNextFile();

private:
    bool index(Transaction* tr, const QString& filePath, quint64 id);

    QMimeDatabase m_mimeDb;

    KFileMetaData::ExtractorCollection m_extractorCollection;

    FileIndexerConfig m_config;

    QSocketNotifier m_notifyNewData;
    QFile m_input;
    QDataStream m_inputStream;
    QTextStream m_outputStream;

    KIdleTime* m_idleTime = nullptr;
    bool m_isBusy = true;

    QVector<quint64> m_ids;
    Transaction* m_tr;
};

}
#endif
