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
#include "database.h"

#include <QCoreApplication>
#include <QTimer>
#include <KDebug>

class App : public QCoreApplication {
    Q_OBJECT
public:
    App(int& argc, char** argv, int flags = ApplicationFlags);

private slots:
    void main();
    void slotTagCreated(Tag* tag);

    void slotTagAdded(const Tag& tag);
    void slotTagRemoved(const Tag& tag);
    void slotItemAdded(const Item& item);
    void slotItemRemoved(const Item& item);
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
    Database* db = new Database;
    db->setPath("/tmp/tagDb.sqlite");
    db->init();

    //FIXME: This sucks!
    Tag* tag = new Tag("TagA");
    TagCreateJob* job = tag->create();
    connect(job, SIGNAL(tagCreated(Tag*)), this, SLOT(slotTagCreated(Tag*)));

    job->start();
}

void App::slotTagCreated(Tag* tag)
{
    TagRelationWatcher* watcher1 = new TagRelationWatcher(*tag, this);
    connect(watcher1, SIGNAL(tagAdded(Tag)), this, SLOT(slotTagAdded(Tag)));
    connect(watcher1, SIGNAL(tagRemoved(Tag)), this, SLOT(slotTagRemoved(Tag)));

    Item item;
    item.setId("file:1");

    TagRelationWatcher* watcher2 = new TagRelationWatcher(item, this);
    connect(watcher2, SIGNAL(itemAdded(Item)), this, SLOT(slotItemAdded(Item)));
    connect(watcher2, SIGNAL(itemRemoved(Item)), this, SLOT(slotItemRemoved(Item)));

    TagRelation* rel = new TagRelation(*tag, item);
    rel->create()->start();
}

void App::slotTagAdded(const Tag& tag)
{
    kDebug() << tag.id();

    Item item;
    item.setId("file:1");

    TagRelation* rel = new TagRelation(tag, item);
    rel->remove()->start();
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
