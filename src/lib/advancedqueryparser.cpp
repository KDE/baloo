/*
 * Copyright (C) 2014-2015  Vishesh Handa <vhanda@kde.org>
 * Copyright (C) 2014  Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "advancedqueryparser.h"

#include <QStringList>
#include <QStack>
#include <QDate>

using namespace Baloo;

AdvancedQueryParser::AdvancedQueryParser()
{
}

static QStringList lex(const QString& text)
{
    QStringList tokenList;
    QString token;
    bool inQuotes = false;

    for (int i = 0, end = text.size(); i != end; ++i) {
        QChar c = text.at(i);

        if (c == QLatin1Char('"')) {
            // Quotes start or end string literals
            inQuotes = !inQuotes;
        } else if (inQuotes) {
            // Don't do any processing in strings
            token.append(c);
        } else if (c.isSpace()) {
            // Spaces end tokens
            if (!token.isEmpty()) {
                tokenList.append(token);
                token.clear();
            }
        } else if (c == '(' || c == ')') {
            // Parentheses end tokens, and are tokens by themselves
            if (!token.isEmpty()) {
                tokenList.append(token);
                token.clear();
            }
            tokenList.append(c);
        } else if (c == '>' || c == '<' || c == ':' || c == '=') {
            // Operators end tokens
            if (!token.isEmpty()) {
                tokenList.append(token);
                token.clear();
            }
            // accept '=' after any of the above
            if (text.at(i + 1) == '=') {
                tokenList.append(text.mid(i, 2));
                i++;
            } else {
                tokenList.append(c);
            }
        } else {
            // Simply extend the current token
            token.append(c);
        }
    }

    if (!token.isEmpty()) {
        tokenList.append(token);
    }

    return tokenList;
}

static void addTermToStack(QStack<Term>& stack, const Term& termInConstruction, Term::Operation op)
{
    Term &tos = stack.top();

    if (tos.isEmpty()) {
        // Empty top of stack, just assign termInConstruction to it
        tos = termInConstruction;
        return;
    }

    tos = Term(tos, op, termInConstruction);
}

static QVariant tokenToVariant(const QString& token)
{
    bool okay = false;
    int intValue = token.toInt(&okay);
    if (okay) {
        return QVariant(intValue);
    }

    QDate date = QDate::fromString(token, Qt::ISODate);
    if (date.isValid() && !date.isNull()) {
        return date;
    }

    return token;
}

Term AdvancedQueryParser::parse(const QString& text)
{
    // The parser does not do any look-ahead but has to store some state
    QStack<Term> stack;
    QStack<Term::Operation> ops;
    Term termInConstruction;
    bool valueExpected = false;

    stack.push(Term());
    ops.push(Term::And);

    // Lex the input string
    QStringList tokens = lex(text);
    for (const QString &token : tokens) {
        // If a key and an operator have been parsed, now is time for a value
        if (valueExpected) {
            // When the parser encounters a literal, it puts it in the value of
            // termInConstruction so that "foo bar baz" is parsed as expected.
            auto property = termInConstruction.value().toString();
            if (property.isEmpty()) {
                qDebug() << "Binary operator without first argument encountered:" << text;
                return Term();
            }
            termInConstruction.setProperty(property);

            QVariant value = tokenToVariant(token);
            if (value.type() != QVariant::String) {
                if (termInConstruction.comparator() == Term::Contains) {
                    termInConstruction.setComparator(Term::Equal);
                }
            }

            termInConstruction.setValue(value);
            valueExpected = false;
            continue;
        }

        // Handle the logic operators
        if (token == QStringLiteral("AND")) {
            if (!termInConstruction.isEmpty()) {
                addTermToStack(stack, termInConstruction, ops.top());
                termInConstruction = Term();
            }
            ops.top() = Term::And;
            continue;
        } else if (token == QStringLiteral("OR")) {
            if (!termInConstruction.isEmpty()) {
                addTermToStack(stack, termInConstruction, ops.top());
                termInConstruction = Term();
            }
            ops.top() = Term::Or;
            continue;
        }

        // Handle the different comparators (and braces)
        Term::Comparator comparator = Term::Auto;

        switch (token.at(0).toLatin1()) {
            case ':':
                comparator = Term::Contains;
                break;
            case '=':
                comparator = Term::Equal;
                break;
            case '<': {
                if (token.size() == 1) {
                    comparator = Term::Less;
                } else if (token[1] == '=') {
                    comparator = Term::LessEqual;
                }
                break;
            }
            case '>': {
                if (token.size() == 1) {
                    comparator = Term::Greater;
                } else if (token[1] == '=') {
                    comparator = Term::GreaterEqual;
                }
                break;
            }
            case '(':
                if (!termInConstruction.isEmpty()) {
                    addTermToStack(stack, termInConstruction, ops.top());
                    ops.top() = Term::And;
                }

                stack.push(Term());
                ops.push(Term::And);
                termInConstruction = Term();

                continue;
            case ')':
                // Prevent a stack underflow if the user writes "a b ))))"
                if (stack.size() > 1) {
                    // Don't forget the term just before the closing brace
                    if (termInConstruction.value().isValid()) {
                        addTermToStack(stack, termInConstruction, ops.top());
                    }

                    // stack.pop() is the term that has just been closed. Append
                    // it to the term just above it.
                    ops.pop();
                    addTermToStack(stack, stack.pop(), ops.top());
                    ops.top() = Term::And;
                    termInConstruction = Term();
                }

                continue;
            default:
                break;
        }

        if (comparator != Term::Auto) {
            // Set the comparator of the term in construction and expect a value
            termInConstruction.setComparator(comparator);
            valueExpected = true;
        } else {
            // A new term will be started, so termInConstruction has to be appended
            // to the top-level subterm list.
            if (!termInConstruction.isEmpty()) {
                addTermToStack(stack, termInConstruction, ops.top());
                ops.top() = Term::And;
            }

            termInConstruction = Term(QString(), token);
        }
    }

    if (termInConstruction.value().isValid()) {
        addTermToStack(stack, termInConstruction, ops.top());
    }

    return stack.top();
}

