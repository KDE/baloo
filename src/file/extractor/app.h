/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXTRACTOR_APP_H
#define EXTRACTOR_APP_H

#include <QDataStream>
#include <QFile>
#include <QMimeDatabase>
#include <QSocketNotifier>
#include <QStringList>
#include <QTextStream>
#include <QVector>

#include <KFileMetaData/ExtractorCollection>

#include "../fileindexerconfig.h"
#include "database.h"

class KIdleTime;

namespace Baloo
{
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
