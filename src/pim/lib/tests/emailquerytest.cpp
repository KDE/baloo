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

#include "../resultiterator.h"
#include "../emailquery.h"

#include <QCoreApplication>
#include <QTimer>
#include <KDebug>

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
    void itemsReceived(const Akonadi::Item::List& item);
};

int main(int argc, char** argv)
{
    App app(argc, argv);

    if (argc != 2) {
        kError() << "Proper args required";
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
    EmailQuery query;
    query.matches(m_query);
    query.setLimit(100);

    QList<Akonadi::Entity::Id> m_akonadiIds;

    ResultIterator it = query.exec();
    while (it.next()) {
        m_akonadiIds << it.id();
    }
    kDebug() << "Got" << m_akonadiIds.size() << "items";

    if (m_akonadiIds.isEmpty()) {
        quit();
        return;
    }

    Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob(m_akonadiIds);
    job->fetchScope().fetchFullPayload(true);

    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(itemsReceived(Akonadi::Item::List)));
    connect(job, SIGNAL(finished(KJob*)),
            this, SLOT(quit()));

    job->start();
}

void App::itemsReceived(const Akonadi::Item::List& itemList)
{
    Q_FOREACH (const Akonadi::Item& item, itemList) {
        KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
        KDateTime date = message->date()->dateTime().toUtc();
        kDebug() << date.toString(KDateTime::ISODate) << message->subject()->asUnicodeString();
    }
}


#include "emailquerytest.moc"
