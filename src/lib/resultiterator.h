/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_CORE_RESULT_ITERATOR_H
#define BALOO_CORE_RESULT_ITERATOR_H

#include "core_export.h"

#include <QString>

namespace Baloo {

class ResultList;
class ResultIteratorPrivate;

/**
 * @class ResultIterator resultiterator.h <Baloo/ResultIterator>
 */
class BALOO_CORE_EXPORT ResultIterator
{
public:
    ResultIterator(ResultIterator &&rhs);
    ~ResultIterator();

#if BALOO_CORE_BUILD_DEPRECATED_SINCE(5, 55)
    /**
     * @deprecated Since 5.55. Do not use this function, ResultIterator is not copyable, move it if needed
     */
    BALOO_CORE_DEPRECATED_VERSION(5, 55, "Do not use. ResultIterator is not copyable, move it if needed.")
    ResultIterator(const ResultIterator& rhs);
#else
    ResultIterator(const ResultIterator& rhs) = delete;
#endif
    ResultIterator &operator=(const ResultIterator& rhs) = delete;

    bool next();
    QString filePath() const;
    QByteArray documentId() const;

private:
    explicit ResultIterator(ResultList&& res);
    ResultIteratorPrivate* d;

    friend class Query;
};

}
#endif // BALOO_CORE_RESULT_ITERATOR_H
