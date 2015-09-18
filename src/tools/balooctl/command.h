/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#ifndef BALOO_COMMAND_H
#define BALOO_COMMAND_H

#include <QString>
#include <QCommandLineParser>

namespace Baloo {

/**
 * An Abstract class from which all other balooctl commands can inherit from
 */
class Command
{
public:
    virtual ~Command();

    virtual QString command() = 0;
    virtual QString description() = 0;

    virtual int exec(const QCommandLineParser& parser) = 0;

};

}

#endif // BALOO_COMMAND_H
