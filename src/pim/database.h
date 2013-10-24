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

#include <KTempDir>
#include <QHash>

class Database
{
public:
    Database();
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
     * efficiency
     */
    void commit();

    /**
     * Sets the value of this \p key to exactly that of \p value
     */
    void set(const QByteArray& key, const QByteArray& value);
    void set(const QByteArray& key, const QString& value) {
        set(key, value.toUtf8());
    }

    /**
     * Sets the value of key \p key, to \p value after running the
     * value through a token generator. This means superflous details
     * such as punctuations and capitalization will be lost
     */
    void setText(const QByteArray& key, const QByteArray& value);
    void setText(const QByteArray& key, const QString& value) {
        setText(key, value.toUtf8());
    }

    void append(const QByteArray& key, const QByteArray& value);
    void append(const QByteArray& key, const QString& value) {
        append(key, value.toUtf8());
    }

    void appendText(const QByteArray& key, const QByteArray& value);
    void appendText(const QByteArray& key, const QString& value) {
        appendText(key, value.toUtf8());
    }

    /**
     * Only appends the boolean value if \p value is true. Otherwise
     * it is ignored.
     */
    // FIXME: This means we cannot do negation queries on booleans
    void appendBool(const QByteArray& dbKey, const QByteArray& key, bool value);

private:
    Xapian::WritableDatabase* fetchDb(const QByteArray& key);
    Xapian::Document* fetchDoc(Xapian::WritableDatabase* db);

    /// Maps keys -> database
    QHash<QByteArray, Xapian::WritableDatabase*> m_databases;
    QHash<Xapian::WritableDatabase*, Xapian::Document*> m_documents;

    QByteArray m_pathDir;
    uint m_docId;
};

#endif // DATABASE_H
