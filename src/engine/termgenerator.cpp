/*
 * This file is part of the KDE Baloo project.
 * Copyright (C) 2014-2015 Vishesh Handa <vhanda@kde.org>
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

#include "termgenerator.h"
#include "document.h"

#include <QTextBoundaryFinder>
#include <QStringList>

using namespace Baloo;

TermGenerator::TermGenerator(Document* doc)
    : m_doc(doc)
    , m_position(1)
{
}

void TermGenerator::indexText(const QString& text, int wdfInc)
{
    indexText(text, QByteArray(), wdfInc);
}

QStringList TermGenerator::termList(const QString& text_)
{
    QString text(text_);
    text.replace('_', ' ');

    int start = 0;
    int end = 0;

    QStringList list;
    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
            start = bf.position();
            continue;
        }
        else if (bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            end = bf.position();

            QString str = text.mid(start, end - start);

            // Remove all accents. It is important to call toLower after normalization,
            // since some exotic unicode symbols can remain uppercase
            const QString denormalized = str.normalized(QString::NormalizationForm_KD).toLower();

            QString cleanString;
            cleanString.reserve(denormalized.size());
            Q_FOREACH (const QChar& ch, denormalized) {
                auto cat = ch.category();
                if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining && cat != QChar::Mark_Enclosing) {
                    cleanString.append(ch);
                }
            }

            str = cleanString.normalized(QString::NormalizationForm_KC);
            if (!str.isEmpty()) {
                list << str;
            }
        }
    }

    return list;
}

void TermGenerator::indexText(const QString& text, const QByteArray& prefix, int wdfInc)
{
    QStringList terms = termList(text);
    for (const QString& term : terms) {
        QByteArray arr = term.toUtf8();

        QByteArray finalArr = prefix + arr;
        finalArr = finalArr.mid(0, maxTermSize);

        m_doc->addPositionTerm(finalArr, m_position, wdfInc);
        m_position++;
    }
}

void TermGenerator::indexFileNameText(const QString& text, const QByteArray& prefix, int wdfInc)
{
    QStringList terms = termList(text);
    for (const QString& term : terms) {
        QByteArray arr = term.toUtf8();

        QByteArray finalArr = prefix + arr;
        finalArr = finalArr.mid(0, maxTermSize);

        m_doc->addFileNamePositionTerm(finalArr, m_position, wdfInc);
        m_position++;
    }
}

void TermGenerator::indexFileNameText(const QString& text, int wdfInc)
{
    indexFileNameText(text, QByteArray(), wdfInc);
}

void TermGenerator::indexXattrText(const QString& text, const QByteArray& prefix, int wdfInc)
{
    QStringList terms = termList(text);
    for (const QString& term : terms) {
        QByteArray arr = term.toUtf8();

        QByteArray finalArr = prefix + arr;
        finalArr = finalArr.mid(0, maxTermSize);

        m_doc->addXattrPositionTerm(finalArr, m_position, wdfInc);
        m_position++;
    }

}

int TermGenerator::position() const
{
    return m_position;
}

void TermGenerator::setPosition(int position)
{
    m_position = position;
}

