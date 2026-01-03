/*
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILEINDEXRESULT_H
#define BALOO_FILEINDEXRESULT_H

#include <QObject>
#include <QtTypes>

namespace Baloo::IndexResult
{
enum Changed : qint32 {
    Unchanged,
    Updated,
};

enum FileStatus : qint32 {
    Successful = 0x000,

    IgnoredFilename = 0x100,
    IgnoredMimetype = 0x110,
    IgnoredMimetypeUnsupported = 0x115,
    IgnoredTooLarge = 0x120,

    ErrorFileNotFound = 0x400,
    ErrorExtractionFailed = 0x410,
};

} // namespace Baloo::IndexResult

Q_DECLARE_METATYPE(Baloo::IndexResult::FileStatus)
Q_DECLARE_METATYPE(Baloo::IndexResult::Changed)

#endif // BALOO_FILEINDEXRESULT_H
