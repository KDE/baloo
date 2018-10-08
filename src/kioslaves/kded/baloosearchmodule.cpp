/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014 Vishesh Handa <me@vhanda.in>
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

#include "baloosearchmodule.h"

#include <QDBusConnection>
#include <QUrl>
#include <QTimer>
#include <QDebug>

#include <kdirnotify.h>
#include <kpluginfactory.h>

namespace
{
    inline bool isSearchUrl(const QUrl& url)
    {
        return url.scheme() == QStringLiteral("baloosearch") ||
               url.scheme() == QStringLiteral("timeline") ||
               url.scheme() == QStringLiteral("tags");
    }
}

using namespace Baloo;

SearchModule::SearchModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , m_dirNotify(nullptr)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

void SearchModule::init()
{
    m_dirNotify = new org::kde::KDirNotify(QString(), QString(),
                                           QDBusConnection::sessionBus(), this);
    connect(m_dirNotify, &OrgKdeKDirNotifyInterface::enteredDirectory,
            this, &SearchModule::registerSearchUrl);
    connect(m_dirNotify, &OrgKdeKDirNotifyInterface::leftDirectory,
            this, &SearchModule::unregisterSearchUrl);


    // FIXME: Listen to changes from Baloo!!
    // Listen to dbChanged
    QDBusConnection con = QDBusConnection::sessionBus();
    con.connect(QString(), QStringLiteral("/files"), QStringLiteral("org.kde.baloo"),
                QStringLiteral("updated"), this, SLOT(slotBalooFileDbChanged()));
    con.connect(QString(), QStringLiteral("/files"), QStringLiteral("org.kde"),
                QStringLiteral("changed"), this, SLOT(slotFileMetaDataChanged(QStringList)));
}


void SearchModule::registerSearchUrl(const QString& urlString)
{
    QUrl url(urlString);
    if (isSearchUrl(url)) {
        m_searchUrls << url;
    }
}

void SearchModule::unregisterSearchUrl(const QString& urlString)
{
    QUrl url(urlString);
    m_searchUrls.removeAll(url);
}

void SearchModule::slotBalooFileDbChanged()
{
    qDebug() << m_searchUrls;
    for (const QUrl& dirUrl : qAsConst(m_searchUrls)) {
        org::kde::KDirNotify::emitFilesAdded(dirUrl);
    }
}

void SearchModule::slotFileMetaDataChanged(const QStringList& list)
{
    qDebug() << m_searchUrls;
    qDebug() << list;
    QList<QUrl> localFileUrls;
    localFileUrls.reserve(list.size());
    for (const QString& path : list) {
        localFileUrls << QUrl::fromLocalFile(path);
    }
    org::kde::KDirNotify::emitFilesChanged(localFileUrls);
    slotBalooFileDbChanged();
}

K_PLUGIN_FACTORY_WITH_JSON(Factory,
                           "baloosearchmodule.json",
                           registerPlugin<SearchModule>();)

#include "baloosearchmodule.moc"
