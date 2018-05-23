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

#ifndef BALOODATAMODEL_H
#define BALOODATAMODEL_H

#include <QAbstractListModel>
#include <QString>

class Query : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)

public:
    explicit Query(QObject *parent = nullptr);
    ~Query();

    void setSearchString(const QString &searchString);
    QString searchString() const;

    void setLimit(const int &limit);
    int limit() const;

Q_SIGNALS:
    void searchStringChanged();
    void limitChanged();

private:
    QString m_searchString;
    int m_limit;

};

class QueryResultsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Query* query READ query WRITE setQuery NOTIFY queryChanged)

public:
    explicit QueryResultsModel(QObject *parent = nullptr);
    ~QueryResultsModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        UrlRole = Qt::UserRole + 1
    };

    void setQuery(Query *query);
    Query* query() const;

Q_SIGNALS:
    void queryChanged();

private Q_SLOTS:
    void populateModel();

private:
    QStringList m_balooEntryList;
    Query *m_query;
};

#endif
