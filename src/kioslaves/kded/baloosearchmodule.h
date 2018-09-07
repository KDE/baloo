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

#ifndef KDED_BALOO_SEARCH_MODULE_H_
#define KDED_BALOO_SEARCH_MODULE_H_

#include <KDEDModule>
#include <kdirnotify.h>
#include <QUrl>

namespace Baloo {

class SearchModule : public KDEDModule
{
    Q_OBJECT

public:
    SearchModule(QObject* parent, const QList<QVariant>&);

private Q_SLOTS:
    void init();
    void registerSearchUrl(const QString& url);
    void unregisterSearchUrl(const QString& url);

    void slotBalooFileDbChanged();
    void slotFileMetaDataChanged(const QStringList& list);
private:
    QList<QUrl> m_searchUrls;
    org::kde::KDirNotify* m_dirNotify;
};

}

#endif
