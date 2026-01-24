/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_ORPOSTINGITERATOR_H
#define BALOO_ORPOSTINGITERATOR_H

#include "postingiterator.h"

#include <memory>
#include <vector>

namespace Baloo {

class BALOO_ENGINE_EXPORT OrPostingIterator : public PostingIterator
{
public:
    explicit OrPostingIterator(std::vector<std::unique_ptr<PostingIterator>> &&iterators);
    ~OrPostingIterator() override;

    quint64 next() override;
    quint64 docId() const override;
    quint64 skipTo(quint64 docId) override;

private:
    std::vector<std::unique_ptr<PostingIterator>> m_iterators;
    quint64 m_docId;
    quint64 m_nextId;
};
}

#endif // BALOO_ORPOSTINGITERATOR_H
