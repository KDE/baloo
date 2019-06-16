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

#ifndef BALOO_TERMGENERATOR_H
#define BALOO_TERMGENERATOR_H

#include <QByteArray>
#include <QString>
#include "engine_export.h"
#include "document.h"

namespace Baloo {

class BALOO_ENGINE_EXPORT TermGenerator
{
public:
    explicit TermGenerator(Document& doc);

    void setDocument(Document& doc) {
        m_doc = doc;
    }

    void indexText(const QString& text);
    void indexText(const QString& text, const QByteArray& prefix);

    void indexXattrText(const QString& text, const QByteArray& prefix);
    void indexFileNameText(const QString& text);
    void indexFileNameText(const QString& text, const QByteArray& prefix);

    void setPosition(int position);
    int position() const;

    static QByteArrayList termList(const QString& text);

    // Trim all terms to this size
    const static int maxTermSize = 25;
private:
    Document& m_doc;
    int m_position;
};
}

#endif // BALOO_TERMGENERATOR_H
