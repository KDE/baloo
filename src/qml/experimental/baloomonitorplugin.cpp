/*
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "baloomonitorplugin.h"
#include "monitor.h"

#include <QtQml>

void BalooMonitorPlugin::registerTypes(const char* uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.baloo.experimental"));

    qmlRegisterType<Baloo::Monitor>(uri, 0, 1, "Monitor");
}
