/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2025 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_TERMGENERATOR_P_H
#define BALOO_TERMGENERATOR_P_H

namespace Baloo
{
namespace detail
{

bool verifySurrogates(QStringView text)
{
    if (auto size = text.size(); size == 1) {
        return !text.back().isSurrogate();
    } else if (size == 0) {
        return true;
    }

    for (qsizetype i = 0; i < text.size() - 1; i++) {
        const QChar &c = text.at(i);
        if (!c.isSurrogate()) {
            continue;
        }
        if (c.isLowSurrogate()) {
            return false;
        } else if (!text.at(i + 1).isLowSurrogate()) {
            return false;
        } else {
            i++;
        }
    }
    if (!text.back().isSurrogate()) {
        return true;
    } else if (text.back().isHighSurrogate()) {
        return false;
    } else {
        auto back2 = text.at(text.size() - 2);
        return back2.isHighSurrogate();
    }
}

} // namespace detail
} // namespace Baloo

#endif // BALOO_TERMGENERATOR_P_H
