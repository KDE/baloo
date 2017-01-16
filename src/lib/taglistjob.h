/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BALOO_TAGLISTJOB_H
#define BALOO_TAGLISTJOB_H

#include <KJob>
#include "core_export.h"

namespace Baloo {

class BALOO_CORE_EXPORT TagListJob : public KJob
{
    Q_OBJECT
public:
    explicit TagListJob(QObject* parent = nullptr);
    ~TagListJob() Q_DECL_OVERRIDE;

    void start() Q_DECL_OVERRIDE;
    QStringList tags();

private:
    class Private;
    Private* d;
};

}

#endif // BALOO_TAGLISTJOB_H
