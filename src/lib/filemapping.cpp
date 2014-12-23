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

#include "filemapping.h"
#include "xapiandocument.h"

#include <QDebug>

using namespace Baloo;

FileMapping::FileMapping()
    : m_id(0)
{
}

FileMapping::FileMapping(const QString& url)
    : m_id(0)
{
    m_url = url;
}

FileMapping::FileMapping(uint id)
{
    m_id = id;
}

uint FileMapping::id() const
{
    return m_id;
}

QString FileMapping::url() const
{
    return m_url;
}

void FileMapping::setId(uint id)
{
    m_id = id;
}

void FileMapping::setUrl(const QString& url)
{
    m_url = url;
}

bool FileMapping::fetched()
{
    if (m_id == 0 || m_url.isEmpty())
        return false;

    return true;
}

bool FileMapping::fetch(Xapian::Database* db)
{
    if (fetched())
        return true;

    if (m_id == 0 && m_url.isEmpty())
        return false;

    try {
        if (m_url.isEmpty()) {
            Xapian::Document doc = db->get_document(m_id);
            std::string str = doc.get_value(3);
            m_url = QString::fromUtf8(str.c_str(), str.length());

            return !m_url.isEmpty();
        }
        else {
            // FIXME: Need to catch exceptions!
            QByteArray arr = m_url.toUtf8();
            if (arr.size() > 240) {
                QByteArray p1 = "P1" + arr.mid(0, 240);
                QByteArray p2 = "P2" + arr.mid(240);

                Xapian::Query q1(p1.constData());
                Xapian::Query q2(p2.constData());

                Xapian::Enquire enquire(*db);
                enquire.set_query(Xapian::Query(Xapian::Query::OP_AND, q1, q2));
                enquire.set_weighting_scheme(Xapian::BoolWeight());

                Xapian::MSet mset = enquire.get_mset(0, 1);
                Xapian::MSetIterator it = mset.begin();
                if (it == mset.end())
                    return false;

                m_id = *it;
                return true;
            }
            else {
                Xapian::Enquire enquire(*db);
                QByteArray arrWithPrefix = "P-" + arr;
                enquire.set_query(Xapian::Query(arrWithPrefix.constData()));
                enquire.set_weighting_scheme(Xapian::BoolWeight());

                Xapian::MSet mset = enquire.get_mset(0, 1);
                Xapian::MSetIterator it = mset.begin();
                if (it == mset.end())
                    return false;

                m_id = *it;
                return true;
            }
        }
    }
    catch (...) {
        return false;
    }

    return true;

}

void FileMapping::clear()
{
    m_id = 0;
    m_url.clear();
}

bool FileMapping::empty() const
{
    return m_url.isEmpty() && m_id == 0;
}

bool FileMapping::operator==(const FileMapping& rhs) const
{
    if (rhs.empty() && empty())
        return true;

    if (!rhs.url().isEmpty() && !url().isEmpty())
        return rhs.url() == url();

    if (rhs.id() && id())
        return rhs.id() == id();

    return false;
}

