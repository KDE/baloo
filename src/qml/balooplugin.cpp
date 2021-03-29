/*
    SPDX-FileCopyrightText: 2014 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "balooplugin.h"


#include "queryresultsmodel.h"

void BalooPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);
}

void BalooPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArrayLiteral("org.kde.baloo"));

    qmlRegisterType<QueryResultsModel>(uri, 0, 1, "QueryResultsModel");
    qmlRegisterType<Query>(uri, 0, 1, "Query");
}

