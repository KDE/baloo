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

#ifndef BALOO_PENDINGFILE_H
#define BALOO_PENDINGFILE_H

#include <QString>
#include <QObject>

namespace Baloo {

/**
 * Represents a file which needs to be indexed.
 */
class PendingFile
{
public:
    explicit PendingFile(const QString& path = QString());

    QString path() const;
    void setPath(const QString& path);

    void setAttributeChanged() { m_attributesChanged = true; }
    void setClosedOnWrite() { m_closedOnWrite = true; }
    void setModified() { m_modified = true; }
    void setCreated() { m_created = true; }
    void setDeleted() { m_deleted = true; }

    bool shouldIndexContents() const;
    bool shouldIndexXAttrOnly() const;
    bool shouldRemoveIndex() const;

    bool operator == (const PendingFile& rhs) const {
        return m_path == rhs.m_path;
    }

    /**
     * Takes a PendingFile \p file and merges its flags into
     * the current PendingFile
     */
    void merge(const PendingFile& file);

private:
    QString m_path;

    bool m_created : 1;
    bool m_closedOnWrite : 1;
    bool m_attributesChanged : 1;
    bool m_deleted : 1;
    bool m_modified : 1;

    void printFlags() const;
};

}

Q_DECLARE_METATYPE(Baloo::PendingFile);
Q_DECLARE_TYPEINFO(Baloo::PendingFile, Q_MOVABLE_TYPE);

#endif // BALOO_PENDINGFILE_H
