/*
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "baloomonitorplugin.h"
#include "monitor.h"
#include "indexerstate.h"

#include <QtQml>

void BalooMonitorPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArrayLiteral("org.kde.baloo.experimental"));

    qmlRegisterType<Baloo::Monitor>(uri, 0, 1, "Monitor");
    qmlRegisterUncreatableMetaObject(Baloo::staticMetaObject, uri, 0, 1, "Global", QStringLiteral("Error: only enums"));
}
