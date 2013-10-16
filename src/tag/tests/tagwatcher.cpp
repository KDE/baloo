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
#include "tagstore.h"
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

    void slotTagCreatedFromJob(const Tag& tag);
    void slotTagCreated(const Tag& tag);
    void slotTagRemoved(const Tag& tag);
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

    TagStore* store = new TagStore(this);
    store->setWatchEnabled(true);
    connect(store, SIGNAL(tagCreated(Tag)), this, SLOT(slotTagCreated(Tag)));
    connect(store, SIGNAL(tagRemoved(Tag)), this, SLOT(slotTagRemoved(Tag)));

    Tag tag("TagA");
    TagCreateJob* job = tag.create();
    connect(job, SIGNAL(tagCreated(Tag)), this, SLOT(slotTagCreatedFromJob(Tag)));

    job->start();
}

void App::slotTagCreatedFromJob(const Tag& tag)
{
    tag.remove()->start();
}

void App::slotTagCreated(const Tag& tag)
{
    kDebug() << tag.id() << tag.name();
}

void App::slotTagRemoved(const Tag& tag)
{
    kDebug() << tag.id();
}

#include "tagwatcher.moc"
