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

#include "termgenerator.h"

using namespace Baloo;

TermGenerator::TermGenerator(Xapian::Document* doc)
    : m_doc(doc)
{
    if (doc) {
        m_termGen.set_document(*doc);
    }
}

void TermGenerator::indexText(const QString& text)
{
    indexText(text, QString());
}

void TermGenerator::indexText(const QString& text, const QString& prefix, int wdfInc)
{
    const QByteArray tarr = text.toUtf8();
    const QByteArray par = prefix.toUtf8();
    m_termGen.index_text(tarr.constData(), wdfInc, par.constData());
}
