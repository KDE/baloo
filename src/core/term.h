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

#ifndef TERM_H
#define TERM_H

#include <QString>
#include <QVariant>
#include <QDebug>

#include "core_export.h"

namespace Baloo {

class BALOO_CORE_EXPORT Term
{
public:
    enum Comparator {
        Auto,
        Equal,
        Contains,
        Greater,
        GreaterEqual,
        Less,
        LessEqual
    };

    enum Operation {
        None,
        And,
        Or
    };

    Term();
    Term(const Term& t);

    /**
     * The Item must contain the property \p property
     */
    Term(const QString& property);

    /**
     * The Item must contain the property \p property with
     * value \value.
     *
     * The default comparator is Auto which has the following behavior
     * For Strings - Contains
     * For DateTime - Contains
     * For any other type - Equals
     */
    Term(const QString& property, const QVariant& value, Comparator c = Auto);

    // Removing Range based terms for now
    // FIXME: Add range based terms
    /**
     * The property must fall within the range \p start <= \p value <= \p end
     */
    //Term(const QString& property, const QVariant& start, const QVariant& end);

    /**
     * This term is a combination of other terms
     */
    Term(Operation op);
    Term(Operation op, const Term& t);
    Term(Operation op, const QList<Term>& t);
    ~Term();

    bool isValid() const;

    /**
     * Negate this term. Negation only applies for Equal or Contains
     * For other Comparators you must invert it yourself
     */
    void setNegation(bool isNegated);

    bool negated() const;
    bool isNegated() const;

    void addSubTerm(const Term& term);
    void setSubTerms(const QList<Term>& terms);

    /**
     * Returns the first subTerm in the list of subTerms
     */
    Term subTerm() const;
    QList<Term> subTerms() const;

    void setOperation(Operation op);
    Operation operation() const;

    /**
     * Return the property this term is targetting
     */
    QString property() const;
    void setProperty(const QString& property);

    QVariant value() const;
    void setValue(const QVariant& value);

    Comparator comparator() const;
    void setComparator(Comparator c);

    QVariantMap toVariantMap() const;
    static Term fromVariantMap(const QVariantMap& map);

    bool operator == (const Term& rhs) const;

private:
    class Private;
    Private* d;
};

inline Term operator &&(const Term& lhs, const Term& rhs) {
    Term t(Term::And);
    t.addSubTerm(lhs);
    t.addSubTerm(rhs);
    return t;
}

inline Term operator ||(const Term& lhs, const Term& rhs) {
    Term t(Term::Or);
    t.addSubTerm(lhs);
    t.addSubTerm(rhs);
    return t;
}

inline Term operator !(const Term& rhs) {
    Term t(rhs);
    t.setNegation(!rhs.isNegated());
    return t;
}

inline QDebug operator << (QDebug d, const Term& t) {
    d << t.toVariantMap();
    return d;
}

}
#endif // TERM_H
