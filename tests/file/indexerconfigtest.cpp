/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "fileindexerconfig.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QMimeDatabase>

#include <iostream>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("The file url"));
    parser.process(app);

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    Baloo::FileIndexerConfig config;

    const QString arg = parser.positionalArguments().first();
    const QString url = QFileInfo(arg).absoluteFilePath();

    bool shouldIndex = config.shouldBeIndexed(url);

    QMimeDatabase m_mimeDb;
    QString mimetype = m_mimeDb.mimeTypeForFile(url, QMimeDatabase::MatchContent).name();
    QString fastMimetype = m_mimeDb.mimeTypeForFile(url, QMimeDatabase::MatchExtension).name();

    bool shouldIndexMimetype = config.shouldMimeTypeBeIndexed(fastMimetype);
    std::cout << url.toUtf8().constData() << "\n"
              << "Should Index: " << std::boolalpha << shouldIndex << "\n"
              << "Should Index Mimetype: " << std::boolalpha << shouldIndexMimetype << "\n"
              << "Fast Mimetype: " << fastMimetype.toUtf8().constData() << std::endl
              << "Slow Mimetype: " << mimetype.toUtf8().constData() << std::endl;

    return 0; //app.exec();
}
