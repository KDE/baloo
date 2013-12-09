/* This file is part of the KDE Project
   Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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

#ifndef _BALOO_FILE_KCM_H_
#define _BALOO_FILE_KCM_H_

#include <KCModule>
#include "ui_nepomukconfigwidget.h"
#include "nepomukserverinterface.h"
#include "fileindexerinterface.h"

class QRadioButton;
class QAbstractButton;

namespace Baloo
{

class IndexFolderSelectionDialog;
class ExcludeFilterSelectionDialog;

class ServerConfigModule : public KCModule, private Ui::NepomukConfigWidget
{
    Q_OBJECT

public:
    ServerConfigModule(QWidget* parent, const QVariantList& args);
    ~ServerConfigModule();

public Q_SLOTS:
    void load();
    void save();
    void defaults();

private Q_SLOTS:
    void updateNepomukServerStatus();
    void updateFileIndexerStatus();
    void recreateInterfaces();
    void slotEditIndexFolders();
    void slotAdvancedFileIndexing();
    void slotStatusDetailsClicked();
    void slotFileIndexerSuspendResumeClicked();

    /**
     * This slot is called when any of the "index documents", "index whatever" checkboxes
     * status is chanaged
     */
    void slotCheckBoxesChanged();

private:
    void setFileIndexerStatusText(const QString& text, bool elide);
    void updateFileIndexerSuspendResumeButtonText(bool isSuspended);

    void syncCheckBoxesFromMimetypes(const QStringList& mimetypes);
    QStringList mimetypesFromCheckboxes();

    org::kde::NepomukServer* m_serverInterface;
    org::kde::nepomuk::FileIndexer* m_fileIndexerInterface;

    IndexFolderSelectionDialog* m_indexFolderSelectionDialog;
    ExcludeFilterSelectionDialog* m_excludeFilterSelectionDialog;

    bool m_failedToInitialize;

    bool m_checkboxesChanged;
};
}

#endif
