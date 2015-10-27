/*
 * This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#ifndef BALOO_ENGINE_DOCUMENT_OPERATIONS_H
#define BALOO_ENGINE_DOCUMENT_OPERATIONS_H

#include <QFlag>

namespace Baloo {

enum DocumentOperation {
    DocumentTerms =  1,
    FileNameTerms =  2,
    XAttrTerms    =  4,
    DocumentData  =  8,
    DocumentTime  = 16,
    DocumentUrl   = 32,
    Everything    = DocumentTerms | FileNameTerms | XAttrTerms | DocumentData | DocumentTime | DocumentUrl
};
Q_DECLARE_FLAGS(DocumentOperations, DocumentOperation)

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Baloo::DocumentOperations)

#endif
