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

#include "term.h"
#include <QVariant>

using namespace Baloo;

class Baloo::Term::Private {
public:
    Operation m_op;
    Comparator m_comp;

    QString m_property;
    QVariant m_value;

    bool m_isNegated;

    QList<Term> m_subTerms;

    Private() {
        m_op = None;
        m_comp = Auto;
        m_isNegated = false;
    }
};

Term::Term()
    : d(new Private)
{
}

Term::Term(const QString& property)
    : d(new Private)
{
    d->m_property = property;
}

Term::Term(const QString& property, const QVariant& value, Term::Comparator c)
    : d(new Private)
{
    d->m_property = property;
    d->m_value = value;

    if (c == Auto) {
        if (value.type() == QVariant::String)
            d->m_comp = Contains;
        else if (value.type() == QVariant::DateTime)
            d->m_comp = Contains;
        else
            d->m_comp = Equal;
    }
    else {
        d->m_comp = c;
    }
}

/*
Term::Term(const QString& property, const QVariant& start, const QVariant& end)
    : d(new Private)
{
    d->m_property = property;
    d->m_op = Range;

    // FIXME: How to save range queries?
}
*/

Term::Term(Term::Operation op)
    : d(new Private)
{
    d->m_op = op;
}

Term::Term(Term::Operation op, const Term& t)
    : d(new Private)
{
    d->m_op = op;
    d->m_subTerms << t;
}

Term::Term(Term::Operation op, const QList<Term>& t)
    : d(new Private)
{
    d->m_op = op;
    d->m_subTerms = t;
}

void Term::setNegation(bool isNegated)
{
    d->m_isNegated = isNegated;
}

bool Term::isNegated() const
{
    return d->m_isNegated;
}

bool Term::negated() const
{
    return d->m_isNegated;
}

void Term::addSubTerm(const Term& term)
{
    d->m_subTerms << term;
}

void Term::setSubTerms(const QList<Term>& terms)
{
    d->m_subTerms = terms;
}

Term Term::subTerm() const
{
    if (d->m_subTerms.size())
        return d->m_subTerms.first();

    return Term();
}

QList<Term> Term::subTerms() const
{
    return d->m_subTerms;
}

void Term::setOperation(Term::Operation op)
{
    d->m_op = op;
}

Term::Operation Term::operation() const
{
    return d->m_op;
}

QString Term::property() const
{
    return d->m_property;
}

void Term::setProperty(const QString& property)
{
    d->m_property = property;
}

void Term::setValue(const QVariant& value)
{
    d->m_value = value;
}

QVariant Term::value() const
{
    return d->m_value;
}

Term::Comparator Term::comparator() const
{
    return d->m_comp;
}

void Term::setComparator(Term::Comparator c)
{
    d->m_comp = c;
}

QVariantMap Term::toVariantMap() const
{
    QVariantMap map;
    if (d->m_op != None) {
        QVariantList variantList;
        Q_FOREACH (const Term& term, d->m_subTerms) {
            variantList << QVariant(term.toVariantMap());
        }

        if (d->m_op == And)
            map["$and"] = variantList;
        else
            map["$or"] = variantList;

        return map;
    }

    QString op;
    switch (d->m_comp) {
    case Equal:
        map[d->m_property] = d->m_value;
        return map;

    case Contains:
        op = "$ct";
        break;

    case Greater:
        op = "$gt";
        break;

    case GreaterEqual:
        op = "$gte";
        break;

    case Less:
        op = "$lt";
        break;

    case LessEqual:
        op = "$lte";
        break;

    default:
        return QVariantMap();
    }

    QVariantMap m;
    m[op] = d->m_value;
    map[d->m_property] = QVariant(m);
    return map;
}

Term Term::fromVariantMap(const QVariantMap& map)
{
    if (map.size() != 1)
        return Term();

    Term term;

    QString andOrString;
    if (map.contains("$and")) {
        andOrString = "$and";
        term.setOperation(And);
    }
    else if (map.contains("$or")) {
        andOrString = "$or";
        term.setOperation(Or);
    }

    if (andOrString.size()) {
        QList<Term> subTerms;

        QVariantList list = map[andOrString].toList();
        Q_FOREACH (const QVariant& var, list)
            subTerms << Term::fromVariantMap(var.toMap());

        term.setSubTerms(subTerms);
        return term;
    }

    QString prop = map.keys().first();
    term.setProperty(prop);

    QVariant value = map.value(prop);
    if (value.type() == QVariant::Map) {
        QVariantMap map = value.toMap();
        if (map.size() != 1)
            return term;

        QString op = map.keys().first();
        Term::Comparator com;
        if (op == "$ct")
            com = Contains;
        else if (op == "$gt")
            com = Greater;
        else if (op == "$gte")
            com = GreaterEqual;
        else if (op == "$lt")
            com = Less;
        else if (op == "$lte")
            com = LessEqual;
        else
            return term;

        term.setComparator(com);
        term.setValue(map.value(op));
        return term;
    }

    term.setComparator(Equal);
    term.setValue(value);

    return term;
}
