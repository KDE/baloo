/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "xapiantermgenerator.h"

#include <QTextBoundaryFinder>
#include <QDebug>

using namespace Baloo;

XapianTermGenerator::XapianTermGenerator(Xapian::Document* doc)
    : m_doc(doc)
    , m_position(1)
{
    if (doc) {
        m_termGen.set_document(*doc);
    }
}

void XapianTermGenerator::indexText(const QString& text)
{
    indexText(text, QString());
}

void XapianTermGenerator::setDocument(Xapian::Document* doc)
{
    m_doc = doc;
}


void XapianTermGenerator::indexText(const QString& text, const QString& prefix, int wdfInc)
{
    const QByteArray par = prefix.toUtf8();
    //const QByteArray ta = text.toUtf8();
    //m_termGen.index_text(ta.constData(), wdfInc, par.constData());

    int start = 0;
    int end = 0;

    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
            start = bf.position();
            continue;
        }
        else if (bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            end = bf.position();

            QString str = text.mid(start, end - start);

            // Get the string ready for saving
            str = str.toLower();

            // Remove all accents
            const QString denormalized = str.normalized(QString::NormalizationForm_KD);
            QString cleanString;
            Q_FOREACH (const QChar& ch, denormalized) {
                auto cat = ch.category();
                if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining && cat != QChar::Mark_Enclosing) {
                    cleanString.append(ch);
                }
            }

            str = cleanString.normalized(QString::NormalizationForm_KC);
            Q_FOREACH (const QString& term, str.split(QLatin1Char('_'), QString::SkipEmptyParts)) {
                QByteArray arr = term.toUtf8();

                QByteArray finalArr = par + arr;
                std::string stdString(finalArr.constData(), finalArr.size());
                m_doc->add_posting(stdString, m_position, wdfInc);

                m_position++;
            }
        }
    }
}

int XapianTermGenerator::position() const
{
    return m_position;
}

void XapianTermGenerator::setPosition(int position)
{
    m_position = position;
}

