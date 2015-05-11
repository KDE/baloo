/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BASICINDEXINGJOB_H
#define BASICINDEXINGJOB_H

#include <KFileMetaData/Types>
#include "document.h"

namespace Baloo {

class BasicIndexingJob
{
public:
    BasicIndexingJob(const QString& filePath, const QString& mimetype,
                     bool onlyBasicIndexing);
    ~BasicIndexingJob();

    bool index();

    Document document() { return m_doc; }

    /**
     * Adds the data for all the extended attributes of \p url
     * in the document \p doc
     *
     * \return Returns true if the \p doc was modified
     */
    static bool indexXAttr(const QString& url, Document& doc);

private:
    static QVector<KFileMetaData::Type::Type> typesForMimeType(const QString& mimeType);

    QString m_filePath;
    QString m_mimetype;
    bool m_onlyBasicIndexing;

    Document m_doc;

    friend class BasicIndexingJobTest;
};

}

#endif // BASICINDEXINGJOB_H
