/* This file is part of the Baloo query parser
   Copyright (c) 2013 Denis Steckelmacher <steckdenis@yahoo.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2.1 as published by the Free Software Foundation,
   or any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "pass_typehints.h"
#include "utils.h"

#include "term.h"

#include <klocalizedstring.h>

PassTypeHints::PassTypeHints()
{
    registerHints(QLatin1String("File"),
        i18nc("List of words representing a file", "file files"));
    registerHints(QLatin1String("Image"),
        i18nc("List of words representing an image", "image images picture pictures photo photos"));
    registerHints(QLatin1String("Video"),
        i18nc("List of words representing a video", "video videos movie movies film films"));
    registerHints(QLatin1String("Audio"),
        i18nc("List of words representing an audio file", "music musics"));
    registerHints(QLatin1String("Document"),
        i18nc("List of words representing a document", "document documents"));
    registerHints(QLatin1String("Email"),
        i18nc("List of words representing an email", "mail mails email emails e-mail e-mails message messages"));
    registerHints(QLatin1String("Archive"),
        i18nc("List of words representing an archive", "archive archives tarball tarballs zip"));
    registerHints(QLatin1String("Folder"),
        i18nc("List of words representing a folder", "folder folders directory directories"));
    registerHints(QLatin1String("Contact"),
        i18nc("List of words representing a contact", "contact contacts person people"));
    registerHints(QLatin1String("Note"),
        i18nc("List of words representing a note", "note notes"));
}

void PassTypeHints::registerHints(const QString &type, const QString &hints)
{
    Q_FOREACH(const QString &hint, hints.split(QLatin1Char(' '))) {
        type_hints.insert(hint, type);
    }
}

QList<Baloo::Term> PassTypeHints::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    const QString value = stringValueIfLiteral(match.at(0)).toLower();

    if (value.isNull()) {
        return rs;
    }

    if (type_hints.contains(value)) {
        rs.append(Baloo::Term(
            QLatin1String("_k_typehint"),
            type_hints.value(value),
            Baloo::Term::Equal
        ));
    }

    return rs;
}
