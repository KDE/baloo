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
#include "baloo_xattr_p.h"

#include <Solid/Device>
#include <Solid/StorageAccess>
#include <QDebug>
#include <quuid.h>
#include <QFile>
#include <QDir>

using namespace Baloo;

class XattrDetector::Private {
public:
    QStringList m_unSupportedPaths;
    QStringList m_supportedPaths;

    void init();
    bool m_initialized;
};

XattrDetector::XattrDetector(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->m_initialized = false;
}

void XattrDetector::Private::init()
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
            QString randFile = QLatin1String("baloo-xattr-check-") + QUuid::createUuid().toString();
            const QString url = mountPath + QDir::separator() + randFile;
            if (QFile::exists(url))
                continue;

            QFile file(url);
            if (!file.open(QIODevice::WriteOnly)) {
                m_unSupportedPaths << mountPath;
                break;
            }
            file.close();

            int ret = baloo_setxattr(url, QStringLiteral("test"), QStringLiteral("0"));
            if (ret != -1) {
                // Check the actual error?
                m_unSupportedPaths << mountPath;
            }
            else {
                m_supportedPaths << mountPath;
            }

            QFile::remove(url);
            break;
        }
    }
    m_unSupportedPaths << QStringLiteral("/tmp") << QStringLiteral("/proc");
    qDebug() << "supportedPaths:" << m_supportedPaths;
    qDebug() << "UnsupportedPaths:" << m_unSupportedPaths;
    m_initialized = true;
}

XattrDetector::~XattrDetector()
{
    delete d;
}

bool XattrDetector::isSupported(const QString& path)
{
#ifdef Q_OS_WIN
    return false;
#endif
    if (!d->m_initialized)
        d->init();

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
