/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "xattrdetector.h"

#include <Solid/Device>
#include <Solid/StorageAccess>
#include <KDebug>
#include <quuid.h>
#include <QFile>
#include <QDir>

#include <attr/xattr.h>

using namespace Baloo;

class XattrDetector::Private {
public:
    QStringList m_unSupportedPaths;
    QStringList m_supportedPaths;
};

XattrDetector::XattrDetector(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    QList<Solid::Device> devices
        = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    QStringList mountPaths;
    Q_FOREACH (const Solid::Device& dev, devices) {
        const Solid::StorageAccess* sa = dev.as<Solid::StorageAccess>();
        if (!sa->isAccessible())
            continue;

        mountPaths << sa->filePath();
    }
    mountPaths << QDir::homePath();

    Q_FOREACH (const QString& mountPath, mountPaths) {
        while (1) {
            QString randFile = QUuid::createUuid().toString();
            const QString url = mountPath + QDir::separator() + randFile;
            if (QFile::exists(url))
                continue;

            QFile file(url);
            if (!file.open(QIODevice::WriteOnly)) {
                d->m_unSupportedPaths << mountPath;
                break;
            }
            file.close();

            QByteArray path = QFile::encodeName(url);
            int ret = setxattr(path.constData(), "test", "0", 1, 0);
            if (ret != -1) {
                // Check the actual error?
                d->m_unSupportedPaths << mountPath;
            }
            else {
                d->m_supportedPaths << mountPath;
            }

            QFile::remove(url);
            break;
        }
    }
    d->m_unSupportedPaths << "/tmp" << "/proc";
    kDebug() << "supportedPaths:" << d->m_supportedPaths;
    kDebug() << "UnsupportedPaths:" << d->m_unSupportedPaths;
}

XattrDetector::~XattrDetector()
{
    delete d;
}

bool XattrDetector::isSupported(const QString& path)
{
    Q_FOREACH (const QString& p, d->m_supportedPaths) {
        if (path.startsWith(p))
            return true;
    }

    Q_FOREACH (const QString& p, d->m_unSupportedPaths) {
        if (path.startsWith(p))
            return false;
    }

    return true;
}
