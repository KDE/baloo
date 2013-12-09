/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NEPOMUK_REMOVABLEMEDIACACHE_H
#define NEPOMUK_REMOVABLEMEDIACACHE_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QSet>

#include <Solid/Device>

#include <KUrl>


namespace Nepomuk2
{

/**
 * The removable media cache provides access to all removable
 * media that are supported by Nepomuk. It allows to convert
 * URLs the way RemovableMediaModel requires it and provides
 * more or less unique URIs for each device allowing to store
 * device-specific configuration.
 */
class RemovableMediaCache : public QObject
{
    Q_OBJECT

public:
    RemovableMediaCache(QObject* parent = 0);
    ~RemovableMediaCache();

    class Entry
    {
    public:
        Entry();
        Entry(const Solid::Device& device);

        /**
         * Does the same as constructRelativeUrl except that no char conversion will ever
         * take place. It is, thus, suitable for queries.
         */
        QString constructRelativeUrlString(const QString& path) const;
        KUrl constructRelativeUrl(const QString& path) const;
        KUrl constructLocalFileUrl(const KUrl& filexUrl) const;

        Solid::Device device() const {
            return m_device;
        }
        QString url() const {
            return m_urlPrefix;
        }

        bool isMounted() const;
        QString mountPath() const;

    private:
        Solid::Device m_device;

        /// The prefix to be used for URLs
        QString m_urlPrefix;
    };

    const Entry* findEntryByFilePath(const QString& path) const;
    const Entry* findEntryByUrl(const KUrl& url) const;

    /**
     * Searches for entries which are mounted at a path which starts with
     * the given one. Example: a \p path \p /media will result in all
     * entries which are mounted under \p /media like \p /media/disk1 or
     * \p /media/cdrom.
     */
    QList<const Entry*> findEntriesByMountPath(const QString& path) const;

    QList<const Entry*> allMedia() const;

    /**
     * Returns true if the URL might be pointing to a file on a
     * removable device as handled by this class, ie. a non-local
     * URL which can be converted to a local one.
     * This method is primarily used for performance gain.
     */
    bool hasRemovableSchema(const KUrl& url) const;

    /**
     * Returns true if they are no devices in the RemoveableMediaCache
     */
    bool isEmpty() const;

signals:
    void deviceAdded(const Nepomuk2::RemovableMediaCache::Entry* entry);
    void deviceRemoved(const Nepomuk2::RemovableMediaCache::Entry* entry);
    void deviceMounted(const Nepomuk2::RemovableMediaCache::Entry* entry);
    void deviceTeardownRequested(const Nepomuk2::RemovableMediaCache::Entry* entry);

private slots:
    void slotSolidDeviceAdded(const QString& udi);
    void slotSolidDeviceRemoved(const QString& udi);
    void slotAccessibilityChanged(bool accessible, const QString& udi);
    void slotTeardownRequested(const QString& udi);

private:
    void initCacheEntries();

    Entry* createCacheEntry(const Solid::Device& dev);

    /// maps Solid UDI to Entry
    QHash<QString, Entry> m_metadataCache;

    /// contains all schemas that are used as url prefixes in m_metadataCache
    /// this is used to avoid trying to convert each and every resource in
    /// convertFilexUrl
    QSet<QString> m_usedSchemas;

    mutable QMutex m_entryCacheMutex;
};

} // namespace Nepomuk2

#endif // NEPOMUK_REMOVABLEMEDIACACHE_H
