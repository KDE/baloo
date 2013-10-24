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

#include "emailindexer.h"

#include <QApplication>
#include <QTimer>
#include <KDebug>

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/Collection>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Item>

class App : public QApplication {
    Q_OBJECT
public:
    App(int& argc, char** argv, int flags = ApplicationFlags);

private Q_SLOTS:
    void main();

    void slotRootCollectionsFetched(KJob* job);
    void indexNextCollection();
    void itemReceived(const Akonadi::Item::List& item);
    void slotIndexed();

private:
    Akonadi::Collection::List m_collections;
    EmailIndexer m_indexer;

    QTime m_totalTime;
    int m_indexTime;
    int m_numEmails;
};

int main(int argc, char** argv)
{
    App app(argc, argv);
    return app.exec();
}

App::App(int& argc, char** argv, int flags): QApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, SLOT(main()));
}

void App::main()
{
    Akonadi::CollectionFetchJob* job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                       Akonadi::CollectionFetchJob::Recursive);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotRootCollectionsFetched(KJob*)));
    job->start();

    m_numEmails = 0;
    m_indexTime = 0;
    m_totalTime.start();
}

void App::slotRootCollectionsFetched(KJob* kjob)
{
    Akonadi::CollectionFetchJob* job = qobject_cast<Akonadi::CollectionFetchJob*>(kjob);
    m_collections = job->collections();

    QMutableListIterator<Akonadi::Collection> it(m_collections);
    while (it.hasNext()) {
        const Akonadi::Collection& c = it.next();
        const QStringList mimeTypes = c.contentMimeTypes();
        if (!c.contentMimeTypes().contains("message/rfc822"))
            it.remove();
    }

    if (m_collections.size()) {
        indexNextCollection();
    }
    else {
        kDebug() << "No collections to index";
    }
}

void App::indexNextCollection()
{
    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(m_collections.takeFirst(), this);
    fetchJob->fetchScope().fetchAllAttributes(true);
    fetchJob->fetchScope().fetchFullPayload(true);
    fetchJob->fetchScope().setFetchModificationTime(false);

    connect(fetchJob, SIGNAL(itemsReceived(Akonadi::Item::List)), this, SLOT(itemReceived(Akonadi::Item::List)));
    connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(slotIndexed()));
}

void App::itemReceived(const Akonadi::Item::List& itemList)
{
    QTime timer;
    timer.start();

    Q_FOREACH (const Akonadi::Item& item, itemList) {
        m_indexer.index(item);
    }

    m_indexTime += timer.elapsed();
    m_numEmails += itemList.size();

    timer.restart();
    m_indexer.commit();
    m_indexTime += timer.elapsed();

    kDebug() << "Emails:" << m_numEmails;
    kDebug() << "Total Time:" << m_totalTime.elapsed()/1000.0 << " seconds";
    kDebug() << "Index Time:" << m_indexTime/1000.0 << " seconds";
}

void App::slotIndexed()
{
    if (!m_collections.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(indexNextCollection()));
        return;
    }

    m_indexer.commit();

    kDebug() << "Emails:" << m_numEmails;
    kDebug() << "Total Time:" << m_totalTime.elapsed()/1000.0 << " seconds";
    kDebug() << "Index Time:" << m_indexTime/1000.0 << " seconds";
    quit();
}


#include "emailtest.moc"
