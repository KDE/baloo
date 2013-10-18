/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <xapian.h>

#include <QObject>
#include <KTempDir>

#include <QHash>

class Database : public QObject
{
public:
    explicit Database(QObject* parent = 0);
    ~Database();

    /**
     * Set the folder where the database files should be stored
     */
    void setPath(const QString& path);
    QString path() const;

    /**
     * Begin indexing a document. The id must > 0
     */
    void beginDocument(uint id);
    void endDocument();

    /**
     * Commit all the pending documents to the database. Avoid
     * calling this function too frequently in order to increase
     * efficiently
     */
    void commit();

    void insert(const QByteArray& key, const QByteArray& value);
    void insertText(const QString& text);

    /**
     * Inserts they key in the document only if the \p value is true.
     * Otherwise it is ignored
     */
    void insertBool(const QByteArray& key, bool value);

private:
    /// Maps keys -> database
    QHash<QByteArray, Xapian::WritableDatabase*> m_databases;

    Xapian::WritableDatabase* m_plainTextDb;
    Xapian::Document m_plainTextDoc;
    Xapian::SimpleStopper m_stopper;

    QByteArray m_pathDir;
    uint m_docId;
};

#endif // DATABASE_H
