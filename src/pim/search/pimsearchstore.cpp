/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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
#include "pimsearchstore.h"
#include "term.h"
#include "query.h"

#include <KDebug>
#include <KUrl>

using namespace Baloo;

PIMSearchStore::PIMSearchStore(QObject* parent) : XapianSearchStore(parent)
{
}

QStringList PIMSearchStore::types()
{
    return QStringList() << "Akonadi";
}

Xapian::Query PIMSearchStore::constructQuery(const QString& property, const QVariant& value,
                                               Term::Comparator com)
{
    if (value.isNull())
        return Xapian::Query();

    QString prop = property.toLower();
    if (m_boolProperties.contains(prop)) {
        QString p = m_prefix.value(prop);
        if (p.isEmpty())
            return Xapian::Query();

        std::string term("B");
        bool isTrue = false;

        if (value.isNull() )
            isTrue = true;

        if (value.type() == QVariant::Bool) {
            isTrue = value.toBool();
        }

        if (isTrue)
            term += p.toStdString();
        else
            term += 'N' + p.toStdString();

        return Xapian::Query(term);
    }

    if (com == Term::Contains) {
        Xapian::QueryParser parser;
        parser.set_database(*xapianDb());

        std::string p = m_prefix.value(property.toLower()).toStdString();
        std::string str(value.toString().toUtf8().constData());
        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        return parser.parse_query(str, flags, p);
    }

    if (com == Term::Greater || com == Term::GreaterEqual || com == Term::Less || com == Term::LessEqual) {
        if (!m_valueProperties.contains(property.toLower())) {
            return Xapian::Query();
        }
        qlonglong numVal = value.toLongLong();
        if (com == Term::Greater) {
            numVal++;
        }
        if (com == Term::Less) {
            numVal--;
        }
        int valueNumber = m_valueProperties.value(property.toLower());
        if (com == Term::GreaterEqual || com == Term::Greater) {
            return Xapian::Query(Xapian::Query::OP_VALUE_GE, valueNumber, QString::number(numVal).toStdString());
        }
        else if (com == Term::LessEqual || com == Term::Less) {
            return Xapian::Query(Xapian::Query::OP_VALUE_LE, valueNumber, QString::number(numVal).toStdString());
        }
    }

    return Xapian::Query(value.toString().toStdString());
}

QUrl PIMSearchStore::constructUrl(const Xapian::docid& docid)
{
    KUrl url;
    url.setProtocol(QLatin1String("akonadi"));
    url.addQueryItem(QLatin1String("item"), QString::number(docid));

    return url;
}