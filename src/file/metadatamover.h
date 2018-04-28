/* This file is part of the KDE Project
   Copyright (c) 2009-2011 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2013-2014 Vishesh Handa <vhanda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef BALOO_METADATA_MOVER_H_
#define BALOO_METADATA_MOVER_H_

#include <QObject>
#include <QDBusServiceWatcher>
#include <QMap>

class OrgKdeBalooWatcherApplicationInterface;

namespace org
{

namespace kde
{

typedef ::OrgKdeBalooWatcherApplicationInterface BalooWatcherApplication;

}

}

namespace Baloo
{

class Database;
class Transaction;

class MetadataMover : public QObject
{
    Q_OBJECT

public:
    MetadataMover(Database* db, QObject* parent = nullptr);
    ~MetadataMover();

    bool hasWatcher() const;

public Q_SLOTS:
    void moveFileMetadata(const QString& from, const QString& to);
    void removeFileMetadata(const QString& file);

    void registerBalooWatcher(const QString &service);

Q_SIGNALS:
    /**
     * Emitted for files (and folders) that have been moved but
     * do not have metadata to be moved. This allows the file indexer
     * service to pick them up in case they are of interest. The
     * typical example would be moving a file from a non-indexed into
     * an indexed folder.
     */
    void movedWithoutData(const QString& path);

    void fileRemoved(const QString& path);

private Q_SLOTS:

    void watcherServiceUnregistered(const QString &serviceName);

private:
    /**
     * Remove the metadata for file \p url
     */
    void removeMetadata(Transaction* tr, const QString& url);

    /**
     * Recursively update the nie:url and nie:isPartOf properties
     * of the resource describing \p from.
     */
    void updateMetadata(Transaction* tr, const QString& from, const QString& to);

    void notifyWatchers(const QString &from, const QString &to, const QList<QString> &filesList);

    QMap<QString, org::kde::BalooWatcherApplication*> m_watcherApplications;

    QDBusServiceWatcher m_serviceWatcher;

    Database* m_db;
};
}

#endif
