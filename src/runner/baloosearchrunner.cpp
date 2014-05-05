/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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


#include "baloosearchrunner.h"

#include <QLinkedList>

#include <KIcon>
#include <KRun>
#include <Plasma/QueryMatch>
#include <QDir>

#include "query.h"

SearchRunner::SearchRunner(QObject* parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
}

SearchRunner::SearchRunner(QObject* parent, const QString& serviceId)
    : Plasma::AbstractRunner(parent, serviceId)
{
}

void SearchRunner::init()
{
    Plasma::RunnerSyntax syntax(":q", i18n("Search through files, emails and contacts"));
}

SearchRunner::~SearchRunner()
{
}

void SearchRunner::match(Plasma::RunnerContext& context)
{
    Baloo::Query query;
    query.setSearchString(context.query());

    query.setType("Email");
    query.setLimit(10);
    QLinkedList<Plasma::QueryMatch> mailMatches;
    Baloo::ResultIterator it = query.exec();

    while (context.isValid() && it.next()) {
        Plasma::QueryMatch match(this);
        match.setIcon(KIcon(it.icon()));
        match.setId(it.id());
        match.setText(it.text());
        match.setData(it.url());
        match.setType(Plasma::QueryMatch::PossibleMatch);

        mailMatches.append(match);
    }

    if (!context.isValid())
        return;

    query.setType("File");
    query.setLimit(10 - qMin(5, mailMatches.count()));
    int matchesAdded = 0;
    it = query.exec();

    while (context.isValid() && it.next()) {
        Plasma::QueryMatch match(this);
        match.setIcon(KIcon(it.icon()));
        match.setId(it.id());
        match.setText(it.text());
        match.setData(it.url());
        match.setType(Plasma::QueryMatch::PossibleMatch);

        QString url = it.url().toLocalFile();
        if (url.startsWith(QDir::homePath())) {
            url.replace(0, QDir::homePath().length(), QLatin1String("~"));
        }
        match.setSubtext(url);

        context.addMatch(context.query(), match);

        ++matchesAdded;
    }

    if (!context.isValid())
        return;

    while (context.isValid() && !mailMatches.isEmpty() && matchesAdded <= 10) {
        context.addMatch(context.query(), mailMatches.takeFirst());

        ++matchesAdded;
    }
}

void SearchRunner::run(const Plasma::RunnerContext&, const Plasma::QueryMatch& match)
{
    const QUrl url = match.data().toUrl();
    new KRun(url, 0);
}

K_EXPORT_PLASMA_RUNNER(baloosearchrunner, SearchRunner)
