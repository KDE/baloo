/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_SEARCHSTORE_H
#define BALOO_SEARCHSTORE_H

#include "result_p.h"

namespace Baloo {

class Query;

namespace SearchStore
{
ResultList exec(const Query &query);
};

}

#endif // BALOO_SEARCHSTORE_H
