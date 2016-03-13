/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
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

#include "indexer.h"
#include "basicindexingjob.h"
#include "database.h"
#include "./extractor/result.h"

#include <KFileMetaData/Extractor>
#include <KFileMetaData/PropertyInfo>

using namespace Baloo;

Indexer::Indexer(const QString& url, Transaction* tr)
    : m_url(url)
    , m_tr(tr)
{
}

void Indexer::index()
{
    const QString mimetype = m_mimeDB.mimeTypeForFile(m_url).name();
    BasicIndexingJob basicIJ(m_url, mimetype, BasicIndexingJob::NoLevel);
    basicIJ.index();
    Baloo::Document doc = basicIJ.document();

    Result result(m_url, mimetype, KFileMetaData::ExtractionResult::ExtractEverything);
    result.setDocument(doc);

    const QList<KFileMetaData::Extractor*> exList = m_extractorCollection.fetchExtractors(mimetype);

    for (KFileMetaData::Extractor* ex : exList) {
        ex->extract(&result);
    }

    result.finish();
    if (m_tr->hasDocument(doc.id())) {
        m_tr->replaceDocument(doc, DocumentTerms | DocumentData);
    } else {
        m_tr->addDocument(result.document());
    }
}
