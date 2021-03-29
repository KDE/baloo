/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_QUERYPARSER_H
#define BALOO_QUERYPARSER_H

#include "engine_export.h"
#include <QByteArray>

class QString;

namespace Baloo {

class EngineQuery;

class BALOO_ENGINE_EXPORT QueryParser
{
public:
    QueryParser();

    EngineQuery parseQuery(const QString& str, const QByteArray& prefix = QByteArray());

    /**
     * Set if each word in the string whose length >= \p size should be treated as
     * a partial word and should be expanded to every possible word.
     *
     * By default this value is 3.
     * Setting this value to 0 disable auto expand.
     */
    void setAutoExapandSize(int size);

private:
    int m_autoExpandSize;
};

}

#endif // BALOO_QUERYPARSER_H
