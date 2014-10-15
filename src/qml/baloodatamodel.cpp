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

#include "baloodatamodel.h"
#include "query.h"

#include <QMimeDatabase>

Query::Query(QObject *parent)
    : QObject(parent)
{
}

Query::~Query()
{
}

void Query::setType(const QString &type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    Q_EMIT typeChanged();
}

QString Query::type() const
{
    return m_type;
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

BalooDataModel::BalooDataModel(QObject *parent)
    : QAbstractListModel(parent),
      m_query(new Query(this))
{
    qRegisterMetaType<Baloo::ResultIterator>("Baloo::ResultIterator");
    connect(m_query, &Query::typeChanged, this, &BalooDataModel::populateModel);
    connect(m_query, &Query::limitChanged, this, &BalooDataModel::populateModel);
}

BalooDataModel::~BalooDataModel()
{
}

QHash<int, QByteArray> BalooDataModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[IdRole] = "id";
    roleNames[UrlRole] = "url";

    return roleNames;
}

QVariant BalooDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_balooEntryList.at(index.row()).url().fileName();
    case Qt::DecorationRole: {
        QString localUrl = m_balooEntryList.at(index.row()).url().toLocalFile();
        return QMimeDatabase().mimeTypeForFile(localUrl).iconName();
    }
    case IdRole:
        return m_balooEntryList.at(index.row()).id();
    case UrlRole:
        return m_balooEntryList.at(index.row()).url();
    default:
        return QVariant();
    }
}

int BalooDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_balooEntryList.count();
}

void BalooDataModel::setQuery(Query *query)
{
    if (m_query == query) {
        return;
    }

    delete m_query;
    m_query = query;
    m_query->setParent(this);
    Q_EMIT queryChanged();
}

Query* BalooDataModel::query() const
{
    return m_query;
}

void BalooDataModel::populateModel()
{
    Baloo::Query query;
    query.setType(m_query->type());
    query.setLimit(m_query->limit());
    Baloo::ResultIterator it = query.exec();

    beginResetModel();
    m_balooEntryList.clear();
    while (it.next()) {
        Baloo::Result res = it.result();
        m_balooEntryList << res;
    }
    endResetModel();
}

