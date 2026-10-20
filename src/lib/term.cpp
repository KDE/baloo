/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "term.h"

#include <QDateTime>

namespace Baloo {

class Baloo::Term::Private {
public:
    Operation m_op = None;
    Comparator m_comp = Auto;

    QString m_property;
    QVariant m_value;

    QList<Term> m_subTerms;
};

Term::Term()
    : d(new Private)
{
}

Term::Term(const Term& t)
    : d(new Private(*t.d))
{
}

Term::Term(const QString& property, const QVariant& value, Term::Comparator c)
    : d(new Private)
{
    d->m_property = property;
    d->m_value = value;

    if (c == Auto) {
        if (value.typeId() == QMetaType::QString) {
            d->m_comp = Contains;
        } else if (value.typeId() == QMetaType::QDateTime) {
            d->m_comp = Contains;
        } else {
            d->m_comp = Equal;
        }
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

Term::Term(const Term& lhs, Term::Operation op, const Term& rhs)
    : d(new Private)
{
    d->m_op = op;
    if (lhs.isEmpty()) {
	*d = *(rhs.d);
	return;
    }
    if (rhs.isEmpty()) {
	*d = *(lhs.d);
	return;
    }

    if (lhs.operation() == op) {
        d->m_subTerms << lhs.subTerms();
    } else {
        d->m_subTerms << lhs;
    }

    if (rhs.operation() == op) {
        d->m_subTerms << rhs.subTerms();
    } else {
        d->m_subTerms << rhs;
    }
}

Term::~Term() = default;

bool Term::isValid() const
{
    // Terms with an operator but no subterms are still valid
    if (d->m_op != Term::None) {
        return true;
    }

    if (d->m_comp == Term::Auto) {
        return false;
    }

    return true;
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
    if (!d->m_subTerms.isEmpty()) {
        return d->m_subTerms.first();
    }

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

bool Term::empty() const
{
    return isEmpty();
}

bool Term::isEmpty() const
{
    return d->m_property.isEmpty() && d->m_value.isNull() && d->m_subTerms.isEmpty();
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

bool Term::operator==(const Term& rhs) const
{
    if (d->m_op != rhs.d->m_op || d->m_comp != rhs.d->m_comp //
        || d->m_property != rhs.d->m_property || d->m_value != rhs.d->m_value) {
        return false;
    }

    if (d->m_subTerms.size() != rhs.d->m_subTerms.size()) {
        return false;
    }

    if (d->m_subTerms.isEmpty()) {
        return true;
    }

    for (const Term& t : std::as_const(d->m_subTerms)) {
        if (!rhs.d->m_subTerms.contains(t)) {
            return false;
        }
    }

    return true;
}

Term& Term::operator=(const Term& rhs)
{
    *d = *rhs.d;
    return *this;
}

char *toString(const Term& term)
{
    QString buffer;
    QDebug stream(&buffer);
    stream << term;
    return qstrdup(buffer.toUtf8().constData());
}

} // namespace Baloo

namespace {
    QString comparatorToString(Baloo::Term::Comparator c) {
        switch (c) {
        case Baloo::Term::Auto:
            return QStringLiteral("Auto");
        case Baloo::Term::Equal:
            return QStringLiteral("=");
        case Baloo::Term::Contains:
            return QStringLiteral(":");
        case Baloo::Term::Less:
            return QStringLiteral("<");
        case Baloo::Term::LessEqual:
            return QStringLiteral("<=");
        case Baloo::Term::Greater:
            return QStringLiteral(">");
        case Baloo::Term::GreaterEqual:
            return QStringLiteral(">=");
        }

        return QString();
    }

    QString operationToString(Baloo::Term::Operation op) {
        switch (op) {
        case Baloo::Term::None:
            return QStringLiteral("NONE");
        case Baloo::Term::And:
            return QStringLiteral("AND");
        case Baloo::Term::Or:
            return QStringLiteral("OR");
        }

        return QString();
    }
}

QDebug operator <<(QDebug d, const Baloo::Term& t)
{
    QDebugStateSaver saver(d);
    d.noquote().nospace();
    if (t.subTerms().isEmpty()) {
        if (!t.property().isEmpty()) {
            d << t.property();
        }
	d << comparatorToString(t.comparator());
    if (t.value().typeId() == QMetaType::QString) {
            d << QLatin1Char('"') << t.value().toString() << QLatin1Char('"');
    } else {
            d << t.value().typeName() << QLatin1Char('(')
              << t.value().toString() << QLatin1Char(')');
    }
    }
    else {
        d << "[" << operationToString(t.operation());
        const auto subTerms = t.subTerms();
        for (const Baloo::Term& term : subTerms) {
            d << QLatin1Char(' ') << term;
        }
        d << "]";
    }
    return d;
}
