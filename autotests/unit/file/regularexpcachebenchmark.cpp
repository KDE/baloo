/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "regexpcache.h"
#include "fileexcludefilters.h"

#include <QTest>
#include <QDir>
#include <QDirIterator>

class RegularExpCacheBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void RegularExpCacheBenchmark::test()
{
    RegExpCache regex;
    regex.rebuildCacheFromFilterList(Baloo::defaultExcludeFilterList());

    QBENCHMARK {
        QDirIterator iter(QDir::home(), QDirIterator::Subdirectories);
        int amt = 5000;
        while (iter.hasNext()) {
            if (!amt) {
                break;
            }

            amt--;
            iter.next();
            regex.exactMatch(iter.fileName());
        }

    }
}

QTEST_GUILESS_MAIN(RegularExpCacheBenchmark)

#include "regularexpcachebenchmark.moc"
