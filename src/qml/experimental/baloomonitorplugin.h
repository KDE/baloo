/*
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOOMONITORPLUGIN_H
#define BALOOMONITORPLUGIN_H

#include <QQmlExtensionPlugin>

class BalooMonitorPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif
