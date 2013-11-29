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

    void slotTagCreatedFromJob(const Baloo::Tag& tag);
    void slotTagModifiedFromJob(const Baloo::Tag& tag);

    void slotTagCreated(const Baloo::Tag& tag);
    void slotTagModified(const Baloo::Tag& tag);
    void slotTagRemoved(const Baloo::Tag& tag);
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

    TagStore* store = TagStore::instance();
    store->setWatchEnabled(true);
    connect(store, SIGNAL(tagCreated(Baloo::Tag)), this, SLOT(slotTagCreated(Baloo::Tag)));
    connect(store, SIGNAL(tagRemoved(Baloo::Tag)), this, SLOT(slotTagRemoved(Baloo::Tag)));
    connect(store, SIGNAL(tagModified(Baloo::Tag)), this, SLOT(slotTagModified(Baloo::Tag)));

    Tag tag("TagA");
    TagCreateJob* job = new TagCreateJob(tag, m_con);
    connect(job, SIGNAL(tagCreated(Baloo::Tag)), this, SLOT(slotTagCreatedFromJob(Baloo::Tag)));

    job->start();
}

void App::slotTagCreatedFromJob(const Tag& tag)
{
    Tag t(tag);
    t.setName("TagB");

    TagSaveJob* job = new TagSaveJob(t, m_con);
    connect(job, SIGNAL(tagSaved(Baloo::Tag)), this, SLOT(slotTagModifiedFromJob(Baloo::Tag)));
    job->start();
}

void App::slotTagModifiedFromJob(const Tag& tag)
{
    TagRemoveJob* job = new TagRemoveJob(tag, m_con);
    job->start();
}

void App::slotTagCreated(const Tag& tag)
{
    kDebug() << tag.id() << tag.name();
}

void App::slotTagRemoved(const Tag& tag)
{
    kDebug() << tag.id();
}

void App::slotTagModified(const Tag& tag)
{
    kDebug() << tag.id() << tag.name();
}

#include "tagwatcher.moc"
