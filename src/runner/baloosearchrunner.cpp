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

#include <QIcon>
#include <QDir>
#include <KRun>
#include <KRunner/QueryMatch>
#include <KLocalizedString>
#include <QLinkedList>

#include "query.h"
#include "result.h"

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


void SearchRunner::match(Plasma::RunnerContext& context, const QString& type,
                         const QString& category)
{
    if (!context.isValid())
        return;

    Baloo::Query query;
    query.setSearchString(context.query());
    query.setType(type);
    query.setLimit(10);

    QLinkedList<Plasma::QueryMatch> mailMatches;
    Baloo::ResultIterator it = query.exec();

    while (context.isValid() && it.next()) {
        Plasma::QueryMatch match(this);
        match.setIcon(QIcon::fromTheme(it.icon()));
        match.setId(it.id());
        match.setText(it.text());
        match.setData(it.url());
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setMatchCategory(category);

        QString url = it.url().toLocalFile();
        if (url.startsWith(QDir::homePath())) {
            url.replace(0, QDir::homePath().length(), QLatin1String("~"));
        }
        match.setSubtext(url);

        context.addMatch(match);
    }
}

void SearchRunner::match(Plasma::RunnerContext& context)
{
    match(context, QLatin1String("File/Audio"), QLatin1String("Audio"));
    match(context, QLatin1String("File/Image"), QLatin1String("Image"));
    match(context, QLatin1String("File/Document"), QLatin1String("Document"));
    match(context, QLatin1String("File/Video"), QLatin1String("Video"));
    match(context, QLatin1String("File/Folder"), QLatin1String("Folder"));
    match(context, QLatin1String("Email"), QLatin1String("Email"));
}

void SearchRunner::run(const Plasma::RunnerContext&, const Plasma::QueryMatch& match)
{
    const QUrl url = match.data().toUrl();
    new KRun(url, 0);
}

K_EXPORT_PLASMA_RUNNER(baloosearchrunner, SearchRunner)

#include "baloosearchrunner.moc"
