/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "termgenerator.h"
#include "termgenerator_p.h"

#include <QTextBoundaryFinder>

using namespace Baloo;

namespace {

QString normalizeTerm(const QString &str, QString &buffer)
{
    // Remove all accents. It is important to call toLower after normalization,
    // since some exotic unicode symbols can remain uppercase
    const QString denormalized = str.normalized(QString::NormalizationForm_KD).toLower();

    buffer.resize(0);
    for (const auto& c : denormalized) {
        if (!c.isMark()) {
            buffer.append(c);
        }
    }

    return buffer.normalized(QString::NormalizationForm_KC);
}

void appendTerm(QByteArrayList &list, const QString &term)
{
    if (term.isEmpty()) {
        return;
    }
    // Truncate the string to avoid arbitrarily long terms
    auto utf8 = QStringView(term).left(TermGenerator::maxTermSize).toUtf8();
    if (!utf8.isEmpty()) {
        list.append(utf8);
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
    const QByteArrayList terms = termList(text);
    for (const QByteArray &term : terms) {
        m_doc.addPositionTerm(term, m_position);
        m_position++;
    }
}

QByteArrayList TermGenerator::termList(const QString &text)
{
    if (!Baloo::detail::verifySurrogates(text)) {
        return {};
    }

    qsizetype start = 0;
    auto nextUnderscore = text.indexOf(u'_');
    if (nextUnderscore == -1) {
        nextUnderscore = text.size();
    }

    auto isSkipChar = [] (const QChar& c) {
        return c.isPunct() || c.isMark() || c.isSpace() || (!c.isPrint() && !c.isSurrogate());
    };

    QString normalizationBuffer;
    normalizationBuffer.reserve(20);

    QByteArrayList list;
    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (auto end = bf.toNextBoundary(); end >= 0; end = bf.toNextBoundary()) {
        while (start < end && isSkipChar(text.at(start))) {
            start++;
        }
        if (end == start) {
            continue;
        }

        // Typically we commit a term when we have an EndOfItem, starting
        // from the last StartOfItem, everything between last EndOfItem and
        // StartOfItem is just whitespace and punctuation. Unfortunately,
        // most CJK characters do not trigger a StartOfItem and thus no
        // EndOfItem, so everything in front of a StartOfItem has to be
        // committed as well
        bool commit = bf.boundaryReasons() & (QTextBoundaryFinder::EndOfItem | QTextBoundaryFinder::StartOfItem);

        if (nextUnderscore < end) {
            if (nextUnderscore < start) {
                nextUnderscore = start;
            }
            end = nextUnderscore;
            bf.setPosition(end);
            nextUnderscore = text.indexOf(u'_', nextUnderscore + 1);
            if (nextUnderscore < 0) {
                nextUnderscore = text.size();
            }
            commit = true;
        }

        // Also commit term if end-of-text is reached or when we find
        // any punctuation
        if (!commit && (end == text.size() || isSkipChar(text.at(end)))) {
            commit = true;
        }

        if (commit) {
            const QString term = normalizeTerm(text.mid(start, end - start), normalizationBuffer);
            appendTerm(list, term);
            start = end;
        }
    }
    return list;
}

void TermGenerator::indexProperty(const QString &text, const QByteArray &prefix)
{
    const QByteArrayList terms = termList(text);
    for (const QByteArray &term : terms) {
        QByteArray finalArr = prefix + term;
        m_doc.addTerm(finalArr);
    }
}

void TermGenerator::indexFileNameText(const QString& text)
{
    const QByteArray prefix = QByteArrayLiteral("F");
    const QByteArrayList terms = termList(text);
    for (const QByteArray& term : terms) {
        QByteArray finalArr = prefix + term;
        m_doc.addFileNameTerm(finalArr);
    }
}

void TermGenerator::indexMimetype(const QString &text)
{
    const QByteArray prefix = QByteArrayLiteral("M");
    const QByteArrayList terms = termList(text);
    for (const QByteArray &term : terms) {
        QByteArray finalArr = prefix + term;
        m_doc.addTerm(finalArr);
    }
    m_doc.setMimetype(text);
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
