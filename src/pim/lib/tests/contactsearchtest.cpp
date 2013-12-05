/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "contactquery.h"
#include "../resultiterator.h"

#include <QApplication>
#include <QTimer>
#include <KDebug>

#include <Akonadi/Contact/ContactSearchJob>

using namespace Baloo::PIM;

class App : public QApplication {
    Q_OBJECT
public:
    App(int& argc, char** argv, int flags = ApplicationFlags);

private Q_SLOTS:
    void main();
    void slotItemsReceived(const Akonadi::Item::List& list);
private:
};

int main(int argc, char** argv)
{
    App app(argc, argv);
    return app.exec();
}

App::App(int& argc, char** argv, int flags)
    : QApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, SLOT(main()));
}

void App::main()
{
#if 0
    Akonadi::ContactSearchJob* job = new Akonadi::ContactSearchJob();
    job->setQuery(Akonadi::ContactSearchJob::NameOrEmail, "to", Akonadi::ContactSearchJob::StartsWithMatch);

    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotItemsReceived(Akonadi::Item::List)));
    connect(job, SIGNAL(finished(KJob*)),
            this, SLOT(quit()));
    job->start();
    kDebug() << "Query started";
#endif

    ContactQuery q;
    q.matchEmail("t");
    q.setMatchCriteria(ContactQuery::StartsWithMatch);

    ResultIterator iter = q.exec();
    while (iter.next()) {
        kDebug() << iter.id();
    }
}

void App::slotItemsReceived(const Akonadi::Item::List& list)
{
    kDebug() << list.size();
    Q_FOREACH (const Akonadi::Item& item, list) {
        kDebug() << item.id() << item.mimeType();
    }
}


#include "contactsearchtest.moc"
