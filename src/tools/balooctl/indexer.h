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

#ifndef BALOO_INDEXER_H
#define BALOO_INDEXER_H

#include <QString>
#include <QMimeDatabase>
#include <KFileMetaData/ExtractorCollection>

#include "transaction.h"

namespace Baloo {
class Indexer
{
public:
    Indexer(const QString& url, Transaction* tr);

    void index();

private:
    QString m_url;
    QMimeDatabase m_mimeDB;
    KFileMetaData::ExtractorCollection m_extractorCollection;

    Transaction* m_tr;
};
}

#endif //BALOO_INDEXER_H
