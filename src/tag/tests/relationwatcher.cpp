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

#include "tagrelationwatcher.h"
#include "tagrelation.h"
#include "connection.h"
#include "connection_p.h"

#include <QCoreApplication>
#include <QTimer>
#include <KDebug>
#include <KTempDir>

using namespace Baloo;

class App : public QCoreApplication {
    Q_OBJECT
public:
    App(int& argc, char** argv, int flags = ApplicationFlags);

private Q_SLOTS:
    void main();
    void slotTagCreated(const Baloo::Tag& tag);

    void slotTagAdded(const Baloo::Tag& tag);
    void slotTagRemoved(const Baloo::Tag& tag);
    void slotItemAdded(const Baloo::Item& item);
    void slotItemRemoved(const Baloo::Item& item);

private:
    Baloo::Tags::Connection* m_con;
    KTempDir m_dir;
};

int main(int argc, char** argv)
{
    App app(argc, argv);
    return app.exec();
}

App::App(int& argc, char** argv, int flags): QCoreApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, SLOT(main()));
}

void App::main()
{
    QString dbPath = m_dir.name() + QLatin1String("tagDB.sqlite");
    m_con = new Baloo::Tags::Connection(new Baloo::Tags::ConnectionPrivate(dbPath));

    Tag tag("TagA");
    TagCreateJob* job = new TagCreateJob(tag, m_con);
    connect(job, SIGNAL(tagCreated(Baloo::Tag)), this, SLOT(slotTagCreated(Baloo::Tag)));

    job->start();
}

void App::slotTagCreated(const Tag& tag)
{
    TagRelationWatcher* watcher1 = new TagRelationWatcher(tag, this);
    connect(watcher1, SIGNAL(tagAdded(Baloo::Tag)), this, SLOT(slotTagAdded(Baloo::Tag)));
    connect(watcher1, SIGNAL(tagRemoved(Baloo::Tag)), this, SLOT(slotTagRemoved(Baloo::Tag)));

    Item item;
    item.setId("file:1");

    TagRelationWatcher* watcher2 = new TagRelationWatcher(item, this);
    connect(watcher2, SIGNAL(itemAdded(Baloo::Item)), this, SLOT(slotItemAdded(Baloo::Item)));
    connect(watcher2, SIGNAL(itemRemoved(Baloo::Item)), this, SLOT(slotItemRemoved(Baloo::Item)));

    TagRelationCreateJob* job = new TagRelationCreateJob(TagRelation(tag, item), m_con);
    job->start();
}

void App::slotTagAdded(const Tag& tag)
{
    kDebug() << tag.id();

    Item item;
    item.setId("file:1");

    TagRelationRemoveJob* job = new TagRelationRemoveJob(TagRelation(tag, item), m_con);
    job->start();
}

void App::slotItemAdded(const Item& item)
{
    kDebug() << item.id();
}

void App::slotItemRemoved(const Item& item)
{
    kDebug() << item.id();
}

void App::slotTagRemoved(const Tag& tag)
{
    kDebug() << tag.id();
}

#include "relationwatcher.moc"
