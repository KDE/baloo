/*
 * Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
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
#include "index.h"
#include "emailindexer.h"
#include "contactindexer.h"
#include "akonotesindexer.h"
#include "calendarindexer.h"
#include <AkonadiCore/ServerManager>
#include <KGlobal>
#include <KStandardDirs>
#include <QDir>
#include <xapian/error.h>
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/enquire.h>


namespace {
    QString dbPath(const QString& dbName) {
        QString basePath = QLatin1String("baloo");
        if (Akonadi::ServerManager::hasInstanceIdentifier()) {
            basePath = QString::fromLatin1("baloo/instances/%1").arg(Akonadi::ServerManager::instanceIdentifier());
        }
        return KGlobal::dirs()->localxdgdatadir() + QString::fromLatin1("%1/%2/").arg(basePath, dbName);
    }
    QString emailIndexingPath() {
        return dbPath(QLatin1String("email"));
    }
    QString contactIndexingPath() {
        return dbPath(QLatin1String("contacts"));
    }
    QString emailContactsIndexingPath() {
        return dbPath(QLatin1String("emailContacts"));
    }
    QString akonotesIndexingPath() {
        return dbPath(QLatin1String("notes"));
    }
    QString calendarIndexingPath() {
        return dbPath(QLatin1String("calendars"));
    }
}

Index::Index(QObject* parent)
: QObject(parent)
{
    m_commitTimer.setInterval(1000);
    m_commitTimer.setSingleShot(true);
    connect(&m_commitTimer, SIGNAL(timeout()), this, SLOT(commit()));
}


Index::~Index()
{
    qDeleteAll(m_indexer.values().toSet());
    m_indexer.clear();
}

static void removeDir(const QString& dirName)
{
    QDir dir(dirName);
    if (dir.exists(dirName)) {
        Q_FOREACH(const QFileInfo &info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                removeDir(info.absoluteFilePath());
            }
            else {
                QFile::remove(info.absoluteFilePath());
            }
        }
        dir.rmdir(dirName);
    }
}

void Index::removeDatabase()
{
    qDebug() << "Removing database";
    removeDir(emailIndexingPath());
    removeDir(contactIndexingPath());
    removeDir(emailContactsIndexingPath());
    removeDir(akonotesIndexingPath());
    removeDir(calendarIndexingPath());
}

AbstractIndexer* Index::indexerForItem(const Akonadi::Item& item) const
{
    return m_indexer.value(item.mimeType());
}

QList<AbstractIndexer*> Index::indexersForMimetypes(const QStringList& mimeTypes) const
{
    QList<AbstractIndexer*> indexers;
    Q_FOREACH (const QString& mimeType, mimeTypes) {
        AbstractIndexer *i = m_indexer.value(mimeType);
        if (i) {
            indexers.append(i);
        }
    }
    return indexers;
}

bool Index::haveIndexerForMimeTypes(const QStringList &mimeTypes)
{
    return !indexersForMimetypes(mimeTypes).isEmpty();
}

void Index::index(const Akonadi::Item& item)
{
    AbstractIndexer *indexer = indexerForItem(item);
    if (!indexer) {
        return;
    }

    try {
        indexer->index(item);
    } catch (const Xapian::Error &e) {
        qWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
    }
}

void Index::move(const Akonadi::Item::List& items, const Akonadi::Collection& from, const Akonadi::Collection& to)
{
    //We always get items of the same type
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
       return;
    }
    Q_FOREACH (const Akonadi::Item& item, items) {
        try {
            indexer->move(item.id(), from.id(), to.id());
        } catch (const Xapian::Error &e) {
            qWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::updateFlags(const Akonadi::Item::List& items, const QSet<QByteArray>& addedFlags, const QSet<QByteArray>& removedFlags)
{
    //We always get items of the same type
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }
    Q_FOREACH (const Akonadi::Item& item, items) {
        try {
            indexer->updateFlags(item, addedFlags, removedFlags);
        } catch (const Xapian::Error &e) {
            qWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::remove(const QSet< Akonadi::Entity::Id >& ids, const QStringList& mimeTypes)
{
    const QList<AbstractIndexer*> indexer = indexersForMimetypes(mimeTypes);
    Q_FOREACH (const Akonadi::Item::Id& id, ids) {
        Q_FOREACH (AbstractIndexer *indexer, indexer) {
            try {
                indexer->remove(Akonadi::Item(id));
            } catch (const Xapian::Error &e) {
                qWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
            }
        }
    }
}

void Index::remove(const Akonadi::Item::List& items)
{
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }
    Q_FOREACH (const Akonadi::Item& item, items) {
        try {
            indexer->remove(item);
        } catch (const Xapian::Error &e) {
            qWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::remove(const Akonadi::Collection& col)
{
    Q_FOREACH (AbstractIndexer *indexer, indexersForMimetypes(col.contentMimeTypes())) {
        try {
            indexer->remove(col);
        } catch (const Xapian::Error &e) {
            qWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::addIndexer(AbstractIndexer* indexer)
{
    Q_FOREACH (const QString& mimeType, indexer->mimeTypes()) {
        m_indexer.insert(mimeType, indexer);
    }
}

bool Index::createIndexers()
{
    AbstractIndexer *indexer = 0;
    try {
        QDir().mkpath(emailIndexingPath());
        QDir().mkpath(emailContactsIndexingPath());
        indexer = new EmailIndexer(emailIndexingPath(), emailContactsIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCritical() << "Failed to create email indexer:" << QString::fromStdString(e.get_msg());
    }
    catch (...) {
        delete indexer;
        qCritical() << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(contactIndexingPath());
        indexer = new ContactIndexer(contactIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCritical() << "Failed to create contact indexer:" << QString::fromStdString(e.get_msg());
    }
    catch (...) {
        delete indexer;
        qCritical() << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(akonotesIndexingPath());
        indexer = new AkonotesIndexer(akonotesIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCritical() << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    }
    catch (...) {
        delete indexer;
        qCritical() << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(calendarIndexingPath());
        indexer = new CalendarIndexer(calendarIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCritical() << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    }
    catch (...) {
        delete indexer;
        qCritical() << "Random exception, but we do not want to crash";
    }

    return !m_indexer.isEmpty();
}

void Index::scheduleCommit()
{
    if (!m_commitTimer.isActive()) {
        m_commitTimer.start();
    }
}

void Index::commit()
{
    m_commitTimer.stop();
    Q_FOREACH (AbstractIndexer *indexer, m_indexer) {
        try {
            indexer->commit();
        } catch (const Xapian::Error &e) {
            qWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::findIndexedInDatabase(QSet<Akonadi::Entity::Id> &indexed, Akonadi::Entity::Id collectionId, const QString& dbPath)
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).constData());
    } catch (const Xapian::DatabaseError& e) {
        qCritical() << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return;
    }
    const std::string term = QString::fromLatin1("C%1").arg(collectionId).toStdString();
    Xapian::Query query(term);
    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    Xapian::MSet mset = enquire.get_mset(0, UINT_MAX);
    Xapian::MSetIterator it = mset.begin();
    for (;it != mset.end(); it++) {
        indexed << *it;
    }
}

void Index::findIndexed(QSet<Akonadi::Entity::Id>& indexed, Akonadi::Entity::Id collectionId)
{
    findIndexedInDatabase(indexed, collectionId, emailIndexingPath());
    findIndexedInDatabase(indexed, collectionId, contactIndexingPath());
    findIndexedInDatabase(indexed, collectionId, akonotesIndexingPath());
    findIndexedInDatabase(indexed, collectionId, calendarIndexingPath());
}

qlonglong Index::indexedItems(const qlonglong id)
{
    const std::string term = QString::fromLatin1("C%1").arg(id).toStdString();
    return indexedItemsInDatabase(term, emailIndexingPath())
            + indexedItemsInDatabase(term, contactIndexingPath())
            + indexedItemsInDatabase(term, akonotesIndexingPath())
            + indexedItemsInDatabase(term, calendarIndexingPath());
}

qlonglong Index::indexedItemsInDatabase(const std::string& term, const QString& dbPath) const
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).constData());
    } catch (const Xapian::DatabaseError& e) {
        qCritical() << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return 0;
    }
    return db.get_termfreq(term);
}
