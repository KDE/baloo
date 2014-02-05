/*
 * <one line to give the library's name and an idea of what it does.>
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

#ifndef BALOO_FILECUSTOMMETADATA_H
#define BALOO_FILECUSTOMMETADATA_H

#include <QString>
#include "file_export.h"

namespace Baloo {

/**
 * Save the property identified by key \p key and value \p value
 * as extra metadata for the file with url \p url
 *
 * This metadata is stored in the extended attributes of the file
 * when possible, otherwise they are stored in a separate database
 * common for all files.
 *
 * This operation is synchronous and blocking.
 *
 * @param url The local url of the file
 * @param key The property identifier. Will be converted to utf8
 * @param value The value of the property. Will also be stored in utf8
 */
BALOO_FILE_EXPORT void setCustomFileMetaData(const QString& url, const QString& key,
                                             const QString& value);

/**
 * Fetch the extra metadata property identified by key \p key for the
 * file with url \p url.
 *
 * This metadata is either fetched from the extended attributes of the
 * file or from a common database for all files
 *
 * This operation is synchronous and blocking
 */
BALOO_FILE_EXPORT QString customFileMetaData(const QString& url,
                                             const QString& key);

}

#endif // BALOO_FILECUSTOMMETADATA_H
