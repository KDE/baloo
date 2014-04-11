/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "../contactcompleter.h"

#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KMime/Message>

using namespace Baloo::PIM;

class App : public QCoreApplication {
    Q_OBJECT
public:
    App(int& argc, char** argv, int flags = ApplicationFlags);

    QString m_query;

private Q_SLOTS:
    void main();
};

int main(int argc, char** argv)
{
    App app(argc, argv);

    if (argc != 2) {
        qWarning() << "Proper args required";
    }
    app.m_query = argv[1];

    return app.exec();
}

App::App(int& argc, char** argv, int flags): QCoreApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, SLOT(main()));
}

void App::main()
{
    ContactCompleter com(m_query, 100);

    QTime timer;
    timer.start();

    QStringList emails = com.complete();
    Q_FOREACH (const QString& em, emails)
        std::cout << em.toUtf8().data() << std::endl;

    qDebug() << timer.elapsed();
    quit();
}

#include "contactcompleter.moc"
