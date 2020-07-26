/*
    This file is part of the Baloo KDE project.
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <heidelbach@web.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef INDEXERCONFIGTEST_H
#define INDEXERCONFIGTEST_H

#include <QObject>
// #include <QTemporaryDir>
class QTemporaryDir;

namespace Baloo {
namespace Test {
class FileIndexerConfigTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void testShouldFolderBeIndexed();
    void testShouldFolderBeIndexed_data();

    void testShouldFolderBeIndexedHidden();
    void testShouldFolderBeIndexedHidden_data();

    void testShouldBeIndexed();
    void testShouldBeIndexed_data();

    void testShouldBeIndexedHidden();
    void testShouldBeIndexedHidden_data();

    void testShouldExcludeFilterOnFolders();
    void testShouldExcludeFilterOnFolders_data();

private:
    QTemporaryDir* m_mainDir;
    QString m_dirPrefix;
};

}
}
#endif // INDEXERCONFIGTEST_H
