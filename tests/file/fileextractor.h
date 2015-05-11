/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#ifndef BALOO_FILEEXTRACTOR_H
#define BALOO_FILEEXTRACTOR_H

#include <KJob>
#include <QProcess>

#include <QTime>

namespace Baloo {

class FileExtractor : public KJob
{
    Q_OBJECT
public:
    FileExtractor(quint64 id, const QString& url);

    void start() Q_DECL_OVERRIDE;
    void setCustomPath(const QString& path);

    QString mimeType() const;
    int elapsed() const;

private Q_SLOTS:
    void doStart();
    void slotIndexedFile(int returnCode, QProcess::ExitStatus status);

private:
    int m_id;
    QString m_url;
    QString m_mimeType;

    QString m_customPath;
    QProcess* m_process;

    QTime m_timer;
    int m_elapsed;
};

}

#endif
