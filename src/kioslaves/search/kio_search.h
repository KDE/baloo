/*
  This file is part of the KDE Baloo Project
  SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

  SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_KIO_SEARCH_H_
#define BALOO_KIO_SEARCH_H_

#include <kio/slavebase.h>

namespace Baloo
{

class SearchProtocol : public KIO::SlaveBase
{

public:
    SearchProtocol(const QByteArray& poolSocket, const QByteArray& appSocket);
    ~SearchProtocol() override;

    /**
     *
     */
    void listDir(const QUrl& url) override;

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void mimetype(const QUrl& url) override;

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void stat(const QUrl& url) override;
};
}

#endif
