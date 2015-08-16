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

#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QTextStream>

#include "baloo_interface.h"

namespace org {
    namespace kde {
        typedef OrgKdeBalooInterface balooInterface;
    }
}

namespace Baloo {
class Monitor : public QObject
{
    Q_OBJECT
public:
    Monitor(QObject* parent = 0);

private Q_SLOTS:
    void newFile(const QString& url);

private:
    QTextStream m_out;
    org::kde::balooInterface* m_interface;

};
}
#endif // MONITOR_H
