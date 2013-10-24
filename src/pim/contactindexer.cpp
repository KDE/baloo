/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "contactindexer.h"

#include <KABC/Addressee>

ContactIndexer::ContactIndexer()
{
    m_db.setPath("/tmp/cxap/");
}

ContactIndexer::~ContactIndexer()
{

}

void ContactIndexer::index(const Akonadi::Item& item)
{
    if (!item.hasPayload()) {
        kDebug() << "No payload";
        return;
    }

    if (!item.hasPayload<KABC::Addressee>()) {
        return;
    }

    m_db.beginDocument(item.id());
    KABC::Addressee addresse = item.payload<KABC::Addressee>();

    QString name;
    if (!addresse.formattedName().isEmpty()) {
        name = addresse.formattedName();
    }
    else {
        name = addresse.assembledName();
    }

    m_db.setText("name", name);
    m_db.setText("nick", addresse.nickName());
    m_db.set("uid", addresse.uid());

    // We duplicate all the data in a key called "all"
    m_db.setText("all", name);
    m_db.setText("all", addresse.nickName());
    m_db.set("all", addresse.uid());

    Q_FOREACH (const QString& email, addresse.emails()) {
        m_db.append("email", email);
        m_db.append("all", email);
    }

    // TODO: Contact Groups?
    m_db.endDocument();
}

void ContactIndexer::commit()
{
    m_db.commit();
}


