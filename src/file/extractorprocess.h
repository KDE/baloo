/*
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BALOO_EXTRACTORPROCESS_H
#define BALOO_EXTRACTORPROCESS_H

#include <QProcess>
#include <QObject>
#include <QTimer>
#include <QVector>

namespace Baloo {

class ExtractorProcess : public QObject
{
    Q_OBJECT
public:
    explicit ExtractorProcess(QObject* parent = 0);
    ~ExtractorProcess();

    void index(const QVector<quint64>& fileIds);

Q_SIGNALS:
    void startedIndexingFile(QString filePath);
    void finishedIndexingFile(QString filePath);
    void done();

private Q_SLOTS:
    void slotIndexingFile();

private:
    const QString m_extractorPath;

    QProcess m_extractorProcess;
    QTimer m_timeCurrentFile;
    int m_processTimeout;

    bool m_extractorIdle;
};
}

#endif // BALOO_EXTRACTORPROCESS_H
