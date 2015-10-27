/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2014 Vishesh Handa <vhanda@kde.org>

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

#ifndef REMOVABLEMEDIACACHE_H
#define REMOVABLEMEDIACACHE_H

#include <QObject>

#include <Solid/Device>

namespace Baloo
{

/**
 * The removable media cache
 * media that are supported by Baloo.
 */
class StorageDevices : public QObject
{
    Q_OBJECT

public:
    explicit StorageDevices(QObject* parent = 0);
    ~StorageDevices();

    class Entry
    {
    public:
        Entry();
        explicit Entry(const Solid::Device& device);

        Solid::Device device() const {
            return m_device;
        }

        bool isMounted() const;
        QString mountPath() const;

        /**
         * Returns true if Baloo should be indexing this
         * Currently we only index permanentaly mounted media
         */
        bool isUsable() const;

        QString udi() const {
            return m_device.udi();
        }
    private:
        Solid::Device m_device;
    };

    QList<Entry> allMedia() const;
    bool isEmpty() const;

Q_SIGNALS:
    void deviceAdded(const Baloo::StorageDevices::Entry* entry);
    void deviceRemoved(const Baloo::StorageDevices::Entry* entry);
    void deviceAccessibilityChanged(const Baloo::StorageDevices::Entry* entry);

private Q_SLOTS:
    void slotSolidDeviceAdded(const QString& udi);
    void slotSolidDeviceRemoved(const QString& udi);
    void slotAccessibilityChanged(bool accessible, const QString& udi);

private:
    void initCacheEntries();

    Entry* createCacheEntry(const Solid::Device& dev);

    /// maps Solid UDI to Entry
    QHash<QString, Entry> m_metadataCache;
};

} // namespace Baloo

#endif // REMOVABLEMEDIACACHE_H
