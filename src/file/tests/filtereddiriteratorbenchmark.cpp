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

#include <QCoreApplication>
#include <QTime>
#include <iostream>

#include "../filtereddiriterator.h"
#include "../fileindexerconfig.h"
#include "util.h"

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QTime timer;
    timer.start();

    int num = 0;
    FileIndexerConfig config;
    for (const QString& dir : config.includeFolders()) {
        FilteredDirIterator it(&config, dir);
        while (!it.next().isEmpty()) {
            num++;
        }
    }

    std::cout << "Num Files: " << num << std::endl;
    std::cout << "Elapsed: " << timer.elapsed() << std::endl;
    printIOUsage();

    return 0;
}
