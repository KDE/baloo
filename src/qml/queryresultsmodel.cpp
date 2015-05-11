/*
 *   Copyright 2014 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "queryresultsmodel.h"
#include "query.h"

#include <QMimeDatabase>
#include <QUrl>

Query::Query(QObject *parent)
    : QObject(parent)
    , m_limit(0)
{
}

Query::~Query()
{
}

void Query::setSearchString(const QString &searchString)
{
    if (m_searchString == searchString) {
        return;
    }

    m_searchString = searchString;
    Q_EMIT searchStringChanged();
}

QString Query::searchString() const
{
    return m_searchString;
}

void Query::setLimit(const int &limit)
{
    if (m_limit == limit) {
        return;
    }

    m_limit = limit;
    Q_EMIT limitChanged();
}

int Query::limit() const
{
    return m_limit;
}

QueryResultsModel::QueryResultsModel(QObject *parent)
    : QAbstractListModel(parent),
      m_query(new Query(this))
{
    connect(m_query, &Query::searchStringChanged, this, &QueryResultsModel::populateModel);
    connect(m_query, &Query::limitChanged, this, &QueryResultsModel::populateModel);
}

QueryResultsModel::~QueryResultsModel()
{
}

QHash<int, QByteArray> QueryResultsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[UrlRole] = "url";

    return roleNames;
}

QVariant QueryResultsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        const QUrl url = QUrl::fromLocalFile(m_balooEntryList.at(index.row()));
        return url.fileName();
    }
    case Qt::DecorationRole: {
        QString localUrl = m_balooEntryList.at(index.row());
        return QMimeDatabase().mimeTypeForFile(localUrl).iconName();
    }
    case UrlRole:
        return m_balooEntryList.at(index.row());
    default:
        return QVariant();
    }
}

int QueryResultsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_balooEntryList.count();
}

void QueryResultsModel::setQuery(Query *query)
{
    if (m_query == query) {
        return;
    }

    delete m_query;
    m_query = query;
    m_query->setParent(this);
    Q_EMIT queryChanged();
}

Query* QueryResultsModel::query() const
{
    return m_query;
}

void QueryResultsModel::populateModel()
{
    Baloo::Query query;
    query.setSearchString(m_query->searchString());
    query.setLimit(m_query->limit());
    Baloo::ResultIterator it = query.exec();

    beginResetModel();
    m_balooEntryList.clear();
    while (it.next()) {
        m_balooEntryList << it.filePath();
    }
    endResetModel();
}

