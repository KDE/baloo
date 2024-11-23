/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_CLEARENTRY_H
#define BALOO_CLEARENTRY_H

#include <QTextStream>

#include "database.h"
#include "transaction.h"

namespace Baloo
{

class ClearEntry : public Transaction
{
public:
    ClearEntry(const Database &db, QTextStream &out);
    ClearEntry(Database *db, QTextStream &out);
    bool clearEntryNow(const QString &fileName);

private:
    QTextStream &m_out;
};
}

#endif // BALOO_CLEARENTRY_H
