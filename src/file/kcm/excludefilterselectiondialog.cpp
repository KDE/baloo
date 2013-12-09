/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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

#include "excludefilterselectiondialog.h"

using namespace Baloo;

ExcludeFilterSelectionDialog::ExcludeFilterSelectionDialog(QWidget* parent): KDialog(parent)
{
    setupUi(mainWidget());
    setCaption(i18nc("@title:window Referring to the folders which will be searched for files to index for desktop search",
                     "Advanced File Filtering"));
}

ExcludeFilterSelectionDialog::~ExcludeFilterSelectionDialog()
{

}

void ExcludeFilterSelectionDialog::setExcludeFilters(const QStringList& filters)
{
    m_editExcludeFilters->setItems(filters);
}

void ExcludeFilterSelectionDialog::setExcludeMimeTypes(const QStringList& mimetypes)
{
    m_editMimeTypeExcludes->setItems(mimetypes);

}

QStringList ExcludeFilterSelectionDialog::excludeFilters()
{
    return m_editExcludeFilters->items();
}

QStringList ExcludeFilterSelectionDialog::excludeMimeTypes()
{
    return m_editMimeTypeExcludes->items();
}
