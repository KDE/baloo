/*
  Copyright (C) 2007-2011 Sebastian Trueg <trueg@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "util.h"

#include <QtCore/QUrl>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QUuid>
#include <QtCore/QScopedPointer>
#include <QtCore/QDebug>

#include <KJob>
#include <KDebug>
#include <KGlobal>
#include <KComponentData>

using namespace Baloo;

KJob* clearIndexedData(const QUrl& url)
{
    return 0;//clearIndexedData(QList<QUrl>() << url);
}

KJob* clearIndexedData(const QList<QUrl>& urls)
{
    return 0;
    /*
    if (urls.isEmpty())
        return 0;

    //kDebug() << urls;

    //
    // New way of storing File Indexing Data
    // The Datamanagement API will automatically find the resource corresponding to that url
    //
    KComponentData component = KGlobal::mainComponent();
    if (component.componentName() != QLatin1String("nepomukindexer")) {
        component = KComponentData(QByteArray("nepomukindexer"),
                                   QByteArray(), KComponentData::SkipMainComponentRegistration);
    }
    return removeDataByApplication(urls, RemoveSubResoures, component);
    */
}

//
// We don't really care if the indexing level is in the incorrect graph
//
void updateIndexingLevel(const QUrl& uri, int level)
{
    /*
    QString uriN3 = Soprano::Node::resourceToN3(uri);

    QString query = QString::fromLatin1("select ?g ?l where { graph ?g { %1 kext:indexingLevel ?l . } }")
                    .arg(uriN3);
    Soprano::Model* model = ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparqlNoInference);

    QUrl graph;
    Soprano::Node prevLevel;
    if (it.next()) {
        graph = it[0].uri();
        prevLevel = it[1];
        it.close();
    }

    if (!graph.isEmpty()) {
        QString graphN3 = Soprano::Node::resourceToN3(graph);
        QString removeCommand = QString::fromLatin1("sparql delete { graph %1 { %2 kext:indexingLevel %3 . } }")
                                .arg(graphN3, uriN3, prevLevel.toN3());
        model->executeQuery(removeCommand, Soprano::Query::QueryLanguageUser, QLatin1String("sql"));

        QString insertCommand = QString::fromLatin1("sparql insert { graph %1 { %2 kext:indexingLevel %3 . } }")
                                .arg(graphN3, uriN3, Soprano::Node::literalToN3(level));
        model->executeQuery(insertCommand, Soprano::Query::QueryLanguageUser, QLatin1String("sql"));
    }
    // Practically, this should never happen, but still
    else {
        QScopedPointer<KJob> job(setProperty(QList<QUrl>() << uri, KExt::indexingLevel(),
                                 QVariantList() << QVariant(level)));
        job->setAutoDelete(false);
        job->exec();
    }
    */
}
