/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "query.h"
#include "term.h"

#include <QDateTime>

// Dolphin Use cases
void testSearchAllFiles() {
    // All Files
    {
        Query q;
        q.addType("File");
    }

    // All Audio Files
    {
        Query q;
        q.addType("File/Audio");
    }

    // All Video Files
    {
        Query q;
        q.addType("File/Video");
    }

    // Any file with string
    {
        Query q;
        q.addType("File");
        q.setSearchString("Quick brown fox");
    }

    // Any file with rating < 5
    {
        Query q;
        q.addType("File");
        q.addTerm(Term("rating", 5, Term::L));
    }

    // Any file with Tag x
    {
        Query q;
        q.addType("File");
        q.addTerm(Term("tag", "TagX"));
    }
}

void testSearchTimeline() {
    // Range
    {
        Query q;
        q.addType("File");
        q.addTerm(Term("modified", QDate(2012, 02), QDate(2012, 04)));
    }

    // Files modified in March
    {
        Query q;
        q.addType("File");
        q.addTerm(Term("modified", QDate(2012, 02)));
    }

    // Files Modified before Feb
    {
        Query q;
        q.addType("File");
        q.addTerm(Term("modified", QDate(2012, 02), Term::L));
    }
}

void testFileSearches() {
    // Music Files with artist x
    {
        Query q;
        q.addType("File/Audio");
        q.addTerm(Term("artist", QLatin1String("x")));
    }

    // Music files with word x
    {
        Query q;
        q.addType("File/Audio");
        q.setSearchString("x");
    }

    // Music from Coldplay and not album "X&Y"
    {
        // The Term::Equal can be skiped
        Query q;
        q.addType("File/Audio");
        q.addTerm(Term("artist", QLatin1String("Coldplay"), Term::Equal));
        q.addTerm(!Term("album", QLatin1String("X&Y"), Term::Equal));
    }
}

void testEmailSearches() {
    // Emails have the following properties
    // - Subject
    // - From
    // - To
    // - CC
    // - Message Status
    // - Reply To
    // - Organization
    // - List ID
    // - date

    // Simple email search
    {
        Query q;
        q.addType("Email");
        q.setSearchString("brown fox");
    }

    // Email search with a specific subject
    {
        Query q;
        q.addType("Email");
        q.addTerm(Term("subject", "x"));
    }

    // Email with an attachment?
    {
        Query q;
        q.addType("Email");
        q.addTerm(Term("attachment", true));
    }

    // An Audio file which is an Email Attachment?
    // FIXME!!
    // FIXME: Should an Email class exist?
    {
        Query q;
        q.addType("File");
        q.addRelation(EmailAttachmentRelation(Email("akondi:?item=5")));
    }

    // Email from a or b
    {
        Term t(Term::Or);
        t.addSubTerm(Term("from", "a@x.com"));
        t.addSubTerm(Term("from", "b@y.com"));

        Query q(t);
        q.addType("Email");
    }

    // Email from a or b
    {
        Term a("from", "a@x.com");
        Term b("from", "b@y.com");

        Query q(a || b);
        q.addType("Email");
    }
}
