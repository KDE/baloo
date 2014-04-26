/*
 * Copyright (C) 2014  David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "taglistjobtest.h"
#include "taglistjob.h"

#include "qtest_kde.h"

using namespace Baloo;

void TagListJobTest::testExec()
{
    // A very basic test to ensure that autodeletion works, while debugging a gwenview issue.
    QPointer<Baloo::TagListJob> job = new Baloo::TagListJob();
    job->exec();
    qApp->sendPostedEvents(job, QEvent::DeferredDelete);
    QVERIFY(job == 0);
}

QTEST_KDEMAIN_CORE(TagListJobTest)
