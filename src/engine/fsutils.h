/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Vishesh Handa <me@vhanda.in>
 * Copyright (C) 2010  Tobias Koenig <tokoe@kde.org>
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

#ifndef BALOO_ENGINE_FSUTILS_H
#define BALOO_ENGINE_FSUTILS_H

#include <QString>

namespace Baloo {
namespace FSUtils {

/**
 * Disables filesystem copy-on-write feature on given file or directory.
 * Only implemented on Linux and does nothing on other platforms.
 */
void disableCoW(const QString &path);

}
}
#endif
