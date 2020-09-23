/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "termgenerator.h"

#include <QTextBoundaryFinder>

using namespace Baloo;

namespace {

QString normalizeTerm(const QString &str)
{
    // Remove all accents. It is important to call toLower after normalization,
    // since some exotic unicode symbols can remain uppercase
    const QString denormalized = str.normalized(QString::NormalizationForm_KD).toLower();

    // Remove any leading and trailing space or punctuation
    auto isPunctOrSpace = [] (QChar c) {
        return c.isPunct() || c.isSpace();
    };

    auto begin = denormalized.begin();
    auto end = denormalized.end();
    while (begin < end && isPunctOrSpace(*std::prev(end))) {
        --end;
    }
    while (begin < end && isPunctOrSpace(*begin)) {
        ++begin;
    }

    QString cleanString;
    cleanString.reserve(std::distance(begin, end));
    for (; begin != end; ++begin) {
        if (!begin->isMark()) {
            cleanString.append(*begin);
        }
    }

    return cleanString.normalized(QString::NormalizationForm_KC);
}

void appendTerm(QByteArrayList &list, const QString &term)
{
    if (!term.isEmpty()) {
        // Truncate the string to avoid arbitrarily long terms
        list << term.leftRef(TermGenerator::maxTermSize).toUtf8();
    }
}

}

TermGenerator::TermGenerator(Document& doc)
    : m_doc(doc)
    , m_position(1)
{
}

void TermGenerator::indexText(const QString& text)
{
    indexText(text, QByteArray());
}

QByteArrayList TermGenerator::termList(const QString& text_)
{
    QString text(text_);
    text.replace(QLatin1Char('_'), QLatin1Char(' '));

    int start = 0;

    QByteArrayList list;
    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        // We cannot use only text between StartOfItem and EndOfItem pairs.
        // Most of CJK characters are neither StartOfItem or EndOfItem. And it is possible to have
        // valid characters between an EndOfItem and next StartOfItem. So we just use StartOfItem
        // and EndOfItem as a term boundary hint here.
        if (bf.boundaryReasons() & (QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem)) {
            int end = bf.position();
            const QString term = normalizeTerm(text.mid(start, end - start));
            appendTerm(list, term);
            start = end;
        }
    }
    if (start < text.size()) {
        const QString term = normalizeTerm(text.mid(start));
        appendTerm(list, term);
    }
    return list;
}

void TermGenerator::indexText(const QString& text, const QByteArray& prefix)
{
    const QByteArrayList terms = termList(text);
    if (terms.size() == 1) {
        QByteArray finalArr = prefix + terms[0];
        m_doc.addTerm(finalArr);
        return;
    }
    for (const QByteArray& term : terms) {
        QByteArray finalArr = prefix + term;

        m_doc.addPositionTerm(finalArr, m_position);
        m_position++;
    }
    m_position++;
}

void TermGenerator::indexFileNameText(const QString& text)
{
    const QByteArray prefix = QByteArrayLiteral("F");
    const QByteArrayList terms = termList(text);
    if (terms.size() == 1) {
        QByteArray finalArr = prefix + terms[0];
        m_doc.addFileNameTerm(finalArr);
        return;
    }
    for (const QByteArray& term : terms) {
        QByteArray finalArr = prefix + term;

        m_doc.addFileNamePositionTerm(finalArr, m_position);
        m_position++;
    }
    m_position++;
}

void TermGenerator::indexXattrText(const QString& text, const QByteArray& prefix)
{
    const QByteArrayList terms = termList(text);
    if (terms.size() == 1) {
        QByteArray finalArr = prefix + terms[0];
        m_doc.addXattrTerm(finalArr);
        return;
    }
    for (const QByteArray& term : terms) {
        QByteArray finalArr = prefix + term;

        m_doc.addXattrPositionTerm(finalArr, m_position);
        m_position++;
    }
    m_position++;
}

int TermGenerator::position() const
{
    return m_position;
}

void TermGenerator::setPosition(int position)
{
    m_position = position;
}

