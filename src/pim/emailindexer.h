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

#ifndef EMAILINDEXER_H
#define EMAILINDEXER_H

#include "database.h"
#include "pim_export.h"

#include <KMime/Message>
#include <Akonadi/Item>
#include <Akonadi/KMime/MessageStatus>

class BALOO_PIM_EXPORT EmailIndexer
{
public:
    EmailIndexer();
    ~EmailIndexer();

    void index(const Akonadi::Item& item);
    void commit();
private:
    Database m_db;

    void process(const KMime::Message::Ptr& msg);
    void processPart(KMime::Content* content, KMime::Content* mainContent);
    void processMessageStatus(const Akonadi::MessageStatus& status);

    void insert(const QByteArray& key, KMime::Headers::Generics::MailboxList* mlist);
    void insert(const QByteArray& key, KMime::Headers::Generics::AddressList* alist);
    void insert(const QByteArray& key, const KMime::Types::Mailbox::List& list);
};

#endif // EMAILINDEXER_H
