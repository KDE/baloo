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

#ifndef BALOO_XATTRDETECTOR_H
#define BALOO_XATTRDETECTOR_H

#include <QObject>

namespace Baloo {

/**
 * \class XattrDetector xattrdetector.h
 *
 * \brief This class is used to detect if a file supports the adding of
 * extended attributes to given a file path.
 *
 * The creation of this object is expensive, so avoid it as much as possible
 *
 * \author Vishesh Handa <me@vhanda.in>
 */
class XattrDetector : public QObject
{
public:
    explicit XattrDetector(QObject* parent = nullptr);
    ~XattrDetector() override;

    /**
     * Checks if the give local url \p path supports
     * saving of extended attributes in file system
     */
    bool isSupported(const QString& path);

private:
    class Private;
    Private* d;
};

}

#endif // BALOO_XATTRDETECTOR_H
