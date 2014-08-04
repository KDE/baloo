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

#include "emailindexer.h"

#include <QApplication>
#include <QTimer>
#include <QFile>
#include <QDebug>

#include <CollectionFetchJob>
#include <Collection>
#include <ItemFetchJob>
#include <ItemFetchScope>
#include <Item>

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
    void slotCommitTimerElapsed();

private:
    Akonadi::Collection::List m_collections;
    EmailIndexer m_indexer;

    QTime m_totalTime;
    int m_indexTime;
    int m_numEmails;

    QTimer m_commitTimer;
};

int main(int argc, char** argv)
{
    App app(argc, argv);
    return app.exec();
}

App::App(int& argc, char** argv, int flags)
    : QApplication(argc, argv, flags)
    , m_indexer(QLatin1String("/tmp/xap"), QLatin1String("/tmp/xapC"))
{
    QTimer::singleShot(0, this, SLOT(main()));
}

void App::main()
{
    m_commitTimer.setInterval(1000);
    connect(&m_commitTimer, SIGNAL(timeout()), this, SLOT(slotCommitTimerElapsed()));
    m_commitTimer.start();

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
        if (!c.contentMimeTypes().contains(QLatin1String("message/rfc822")))
            it.remove();
    }

    if (m_collections.size()) {
        indexNextCollection();
    }
    else {
        qDebug() << "No collections to index";
    }
}

void App::indexNextCollection()
{
    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(m_collections.takeFirst(), this);
    fetchJob->fetchScope().fetchAllAttributes(true);
    fetchJob->fetchScope().fetchFullPayload(true);
    fetchJob->fetchScope().setFetchModificationTime(false);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    fetchJob->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);

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
}

void App::slotCommitTimerElapsed()
{
    QTime timer;
    timer.start();

    m_indexer.commit();
    m_indexTime += timer.elapsed();

    qDebug() << "Emails:" << m_numEmails;
    qDebug() << "Total Time:" << m_totalTime.elapsed()/1000.0 << " seconds";
    qDebug() << "Index Time:" << m_indexTime/1000.0 << " seconds";
}

void App::slotIndexed()
{
    if (!m_collections.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(indexNextCollection()));
        return;
    }

    m_indexer.commit();

    qDebug() << "Emails:" << m_numEmails;
    qDebug() << "Total Time:" << m_totalTime.elapsed()/1000.0 << " seconds";
    qDebug() << "Index Time:" << m_indexTime/1000.0 << " seconds";

    // Print the io usage
    QFile file(QLatin1String("/proc/self/io"));
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream fs(&file);
    QString str = fs.readAll();

    qDebug() << "------- IO ---------";
    QTextStream stream(&str);
    while (!stream.atEnd()) {
        QString str = stream.readLine();

        QString rchar(QLatin1String("rchar: "));
        if (str.startsWith(rchar)) {
            ulong amt = str.mid(rchar.size()).toULong();
            qDebug() << "Read:" << amt / 1024  << "kb";
        }

        QString wchar(QLatin1String("wchar: "));
        if (str.startsWith(wchar)) {
            ulong amt = str.mid(wchar.size()).toULong();
            qDebug() << "Write:" << amt / 1024  << "kb";
        }

        QString read(QLatin1String("read_bytes: "));
        if (str.startsWith(read)) {
            ulong amt = str.mid(read.size()).toULong();
            qDebug() << "Actual Reads:" << amt / 1024  << "kb";
        }

        QString write(QLatin1String("write_bytes: "));
        if (str.startsWith(write)) {
            ulong amt = str.mid(write.size()).toULong();
            qDebug() << "Actual Writes:" << amt / 1024  << "kb";
        }
    }
    qDebug() << "\nThe actual read/writes may be 0 because of an existing"
             << "cache and /tmp being memory mapped";
    quit();
}


#include "emailtest.moc"
