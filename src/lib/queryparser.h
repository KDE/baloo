/*
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_QUERYPARSER_H
#define BALOO_QUERYPARSER_H

#include "term.h"

namespace Baloo::QueryParser
{

Baloo::Term parse(const QString &text);

} // namespace Baloo::QueryParser

#endif // BALOO_QUERYPARSER_H
