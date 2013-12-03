/*
   Copyright (C) 2008-2010 by Sebastian Trueg <trueg at kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _NEPOMUK_KIO_NEPOMUK_SEARCH_H_
#define _NEPOMUK_KIO_NEPOMUK_SEARCH_H_

#include <kio/slavebase.h>

#include "searchfolder.h"

#include <QtCore/QQueue>

namespace Nepomuk2
{
namespace Search
{
class Query;
}

class SearchProtocol : public KIO::SlaveBase
{

public:
    SearchProtocol(const QByteArray& poolSocket, const QByteArray& appSocket);
    virtual ~SearchProtocol();

    /**
     *
     */
    void listDir(const KUrl& url);

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void mimetype(const KUrl& url);

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void stat(const KUrl& url);

private:
    bool ensureNepomukRunning(bool emitError = true);
    void listRoot();
};
}

#endif
