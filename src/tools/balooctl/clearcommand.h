/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_CLEARCOMMAND_H
#define BALOO_CLEARCOMMAND_H

namespace Baloo
{

class ClearCommand : public Command
{
public:
    QString command() override;
    QString description() override;
    int exec(const QCommandLineParser &parser) override;
};
}

#endif // BALOO_CLEARCOMMAND_H
