/* This file is part of the KDE Project
   Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _BALOO_INDEX_FOLDER_SELECTION_DIALOG_H_
#define _BALOO_INDEX_FOLDER_SELECTION_DIALOG_H_

#include <KDialog>
#include "ui_indexfolderselectionwidget.h"

class FolderSelectionModel;

namespace Baloo
{
class IndexFolderSelectionDialog : public KDialog, public Ui_IndexFolderSelectionWidget
{
    Q_OBJECT

public:
    IndexFolderSelectionDialog(QWidget* parent = 0);
    ~IndexFolderSelectionDialog();

    void setFolders(const QStringList& includeDirs, const QStringList& exclude);
    void setIndexHiddenFolders(bool enable);

    QStringList includeFolders() const;
    QStringList excludeFolders() const;
    bool indexHiddenFolders() const;

private:
    FolderSelectionModel* m_folderModel;
};
}

#endif
