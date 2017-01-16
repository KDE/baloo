/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014 Raphael Kubo da Costa <rakuco@FreeBSD.org>
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

#ifndef BALOO_XATTR_P_H
#define BALOO_XATTR_P_H

#include <QByteArray>
#include <QFile>
#include <QString>

#if defined(Q_OS_LINUX) || defined(__GLIBC__)
#include <sys/types.h>
#include <sys/xattr.h>
#elif defined(Q_OS_MAC)
#include <sys/types.h>
#include <sys/xattr.h>
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include <sys/types.h>
#include <sys/extattr.h>
#endif

inline ssize_t baloo_getxattr(const QString& path, const QString& name, QString* value)
{
    const QByteArray p = QFile::encodeName(path);
    const char* encodedPath = p.constData();

    const QByteArray n = name.toUtf8();
    const char* attributeName = n.constData();

    // First get the size of the data we are going to get to reserve the right amount of space.
#if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_getxattr))
    const ssize_t size = getxattr(encodedPath, attributeName, nullptr, 0);
#elif defined(Q_OS_MAC)
    const ssize_t size = getxattr(encodedPath, attributeName, NULL, 0, 0, 0);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const ssize_t size = extattr_get_file(encodedPath, EXTATTR_NAMESPACE_USER, attributeName, NULL, 0);
#else
    const ssize_t size = 0;
#endif

    if (size <= 0) {
        if (value) {
            value->clear();
        }
        return size;
    }

    QByteArray data(size, Qt::Uninitialized);

#if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_getxattr))
    const ssize_t r = getxattr(encodedPath, attributeName, data.data(), size);
#elif defined(Q_OS_MAC)
    const ssize_t r = getxattr(encodedPath, attributeName, data.data(), size, 0, 0);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const ssize_t r = extattr_get_file(encodedPath, EXTATTR_NAMESPACE_USER, attributeName, data.data(), size);
#else
    const ssize_t r = 0;
#endif

    *value = QString::fromUtf8(data);
    return r;
}

inline int baloo_setxattr(const QString& path, const QString& name, const QString& value)
{
    const QByteArray p = QFile::encodeName(path);
    const char* encodedPath = p.constData();

    const QByteArray n = name.toUtf8();
    const char* attributeName = n.constData();

    const QByteArray v = value.toUtf8();
    const void* attributeValue = v.constData();

    const size_t valueSize = v.size();

#if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_setxattr))
    return setxattr(encodedPath, attributeName, attributeValue, valueSize, 0);
#elif defined(Q_OS_MAC)
    return setxattr(encodedPath, attributeName, attributeValue, valueSize, 0, 0);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const ssize_t count = extattr_set_file(encodedPath, EXTATTR_NAMESPACE_USER, attributeName, attributeValue, valueSize);
    return count == -1 ? -1 : 0;
#else
    return -1;
#endif
}


inline int baloo_removexattr(const QString& path, const QString& name)
{
    const QByteArray p = QFile::encodeName(path);
    const char* encodedPath = p.constData();

    const QByteArray n = name.toUtf8();
    const char* attributeName = n.constData();

    #if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_removexattr))
        return removexattr(encodedPath, attributeName);
    #elif defined(Q_OS_MAC)
        return removexattr(encodedPath, attributeName, XATTR_NOFOLLOW );
    #elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
        return extattr_delete_file (encodedPath, EXTATTR_NAMESPACE_USER, attributeName);
    #else
        return -1;
    #endif

}

#endif // BALOO_XATTR_P_H
