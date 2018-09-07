/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Ashish Bansal <bansal.ashish096@gmail.com>
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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "database.h"

#include <QString>

namespace Baloo {

    /*
     * If BALOO_DB_PATH environment variable is set, then it returns value of that variable.
     * Otherwise returns the default database path.
     */
    BALOO_ENGINE_EXPORT QString fileIndexDbPath();

    /*
     * lmdb doesn't support opening database twice at the same time in the single process
     * because if we open database twice at the same time and closes one of them, then it
     * would invalidate the handles of both the instances and may lead to crash or some
     * other undesirable behaviour. So, keeping one global database would solve this problem
     * and improve the performance too.
     */
    BALOO_ENGINE_EXPORT Database* globalDatabaseInstance();
}

#endif // GLOBAL_H
