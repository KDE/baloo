/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#ifndef BALOO_PIM_EMAIL_SEARCHSTORE_H
#define BALOO_PIM_EMAIL_SEARCHSTORE_H

#include "../pimsearchstore.h"

namespace Baloo {

class EmailSearchStore : public PIMSearchStore
{
    Q_OBJECT
    Q_INTERFACES(Baloo::SearchStore)
#ifndef BALOO_NO_PLUGINS
    Q_PLUGIN_METADATA(IID "org.kde.baloo.email" FILE "balooemailsearchstore.json")
#endif
public:
    EmailSearchStore(QObject* parent = 0);

    virtual QStringList types();
    virtual QString text(int queryId);
    virtual QString icon(int) {
        return QLatin1String("internet-mail");
    }

protected:
    virtual Xapian::Query constructQuery(const QString& property, const QVariant& value,
                                         Term::Comparator com);
    virtual Xapian::Query finalizeQuery(const Xapian::Query& query);
};

}
#endif // BALOO_PIM_EMAIL_SEARCHSTORE_H
