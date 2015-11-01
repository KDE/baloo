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

#include "storagedevices.h"
#include "baloodebug.h"

#include <Solid/DeviceNotifier>
#include <Solid/DeviceInterface>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>
#include <Solid/StorageAccess>
#include <Solid/NetworkShare>
#include <Solid/OpticalDisc>
#include <Solid/Predicate>

using namespace Baloo;

StorageDevices::StorageDevices(QObject* parent)
    : QObject(parent)
{
    initCacheEntries();

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
            this, &StorageDevices::slotSolidDeviceAdded);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved,
            this, &StorageDevices::slotSolidDeviceRemoved);
}


StorageDevices::~StorageDevices()
{
}


void StorageDevices::initCacheEntries()
{
    QList<Solid::Device> devices
        = Solid::Device::listFromQuery(QStringLiteral("StorageVolume.usage=='FileSystem'"))
          + Solid::Device::listFromType(Solid::DeviceInterface::NetworkShare);
    Q_FOREACH (const Solid::Device& dev, devices) {
        createCacheEntry(dev);
    }
}

QList<StorageDevices::Entry> StorageDevices::allMedia() const
{
    return m_metadataCache.values();
}

StorageDevices::Entry* StorageDevices::createCacheEntry(const Solid::Device& dev)
{
    Entry entry(dev);
    if (dev.udi().isEmpty())
        return 0;

    auto it = m_metadataCache.insert(dev.udi(), entry);

    const Solid::StorageAccess* storage = dev.as<Solid::StorageAccess>();
    connect(storage, &Solid::StorageAccess::accessibilityChanged,
            this, &StorageDevices::slotAccessibilityChanged);
    //connect(storage, SIGNAL(teardownRequested(QString)),
    //        this, SLOT(slotTeardownRequested(QString)));
    return &it.value();
}

bool StorageDevices::isEmpty() const
{
    return m_metadataCache.isEmpty();
}


void StorageDevices::slotSolidDeviceAdded(const QString& udi)
{
    qCDebug(BALOO) << udi;
    Entry* e = createCacheEntry(Solid::Device(udi));
    if (e) {
        Q_EMIT deviceAdded(e);
    }
}


void StorageDevices::slotSolidDeviceRemoved(const QString& udi)
{
    QHash< QString, Entry >::iterator it = m_metadataCache.find(udi);
    if (it != m_metadataCache.end()) {
        qCDebug(BALOO) << "Found removable storage volume for Baloo undocking:" << udi;
        Q_EMIT deviceRemoved(&it.value());
        m_metadataCache.erase(it);
    }
}


void StorageDevices::slotAccessibilityChanged(bool accessible, const QString& udi)
{
    qCDebug(BALOO) << accessible << udi;
    Q_UNUSED(accessible);

    //
    // cache new mount path
    //
    Entry* entry = &m_metadataCache[udi];
    Q_ASSERT(entry != 0);
    Q_EMIT deviceAccessibilityChanged(entry);
}

StorageDevices::Entry::Entry()
{
}

StorageDevices::Entry::Entry(const Solid::Device& device)
    : m_device(device)
{
}

QString StorageDevices::Entry::mountPath() const
{
    if (const Solid::StorageAccess* sa = m_device.as<Solid::StorageAccess>()) {
        return sa->filePath();
    } else {
        return QString();
    }
}

bool StorageDevices::Entry::isMounted() const
{
    if (const Solid::StorageAccess* sa = m_device.as<Solid::StorageAccess>()) {
        return sa->isAccessible();
    } else {
        return false;
    }
}

bool StorageDevices::Entry::isUsable() const
{
    if (mountPath().isEmpty()) {
        return false;
    }

    bool usable = true;

    const Solid::Device& dev = m_device;
    if (dev.is<Solid::StorageVolume>() && dev.parent().is<Solid::StorageDrive>()) {
        auto parent = dev.parent().as<Solid::StorageDrive>();
        if (parent->isRemovable() || parent->isHotpluggable()) {
            usable = false;
        }

        const Solid::StorageVolume* volume = dev.as<Solid::StorageVolume>();
        if (volume->isIgnored() || volume->usage() != Solid::StorageVolume::FileSystem) {
            usable = false;
        }
    }

    if (dev.is<Solid::NetworkShare>()) {
        usable = false;
    } else if (dev.is<Solid::OpticalDisc>()) {
        usable = false;
    }

    if (usable) {
        if (const Solid::StorageAccess* sa = dev.as<Solid::StorageAccess>()) {
            usable = sa->isAccessible();
        }
    }

    return usable;
}

