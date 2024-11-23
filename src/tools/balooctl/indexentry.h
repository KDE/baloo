/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_INDEXENTRY_H
#define BALOO_INDEXENTRY_H

#include <QTextStream>

#include "database.h"
#include "transaction.h"

namespace Baloo
{

class IndexEntry : public Transaction
{
public:
    IndexEntry(const Database &db, QTextStream &out);
    IndexEntry(Database *db, QTextStream &out);
    bool indexFileNow(const QString &fileName);

private:
    QTextStream &m_out;
};
}

#endif // BALOO_INDEXENTRY_H
