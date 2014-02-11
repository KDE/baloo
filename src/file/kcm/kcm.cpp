/* This file is part of the KDE Project
   Copyright (c) 2007-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>

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

#include "kcm.h"
#include "../fileexcludefilters.h"
#include "indexfolderselectiondialog.h"
#include "folderselectionwidget.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>
#include <KDirWatch>
#include <KDebug>
#include <KStandardDirs>

#include <QPushButton>
#include <QDir>
#include <QProcess>

K_PLUGIN_FACTORY(BalooConfigModuleFactory, registerPlugin<Baloo::ServerConfigModule>();)
K_EXPORT_PLUGIN(BalooConfigModuleFactory("kcm_baloofile", "kcm_baloofile"))


namespace
{
QStringList defaultFolders()
{
    return QStringList() << QDir::homePath();
}

}

using namespace Baloo;

ServerConfigModule::ServerConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(BalooConfigModuleFactory::componentData(), parent, args)
    , m_failedToInitialize(false)
{
    KAboutData* about = new KAboutData(
        "kcm_baloofile", "kcm_baloofile", ki18n("Configure Desktop Search"),
        KDE_VERSION_STRING, KLocalizedString(), KAboutData::License_GPL,
        ki18n("Copyright 2007-2010 Sebastian Trüg"));
    about->addAuthor(ki18n("Sebastian Trüg"), KLocalizedString(), "trueg@kde.org");
    about->addAuthor(ki18n("Vishesh Handa"), KLocalizedString(), "vhanda@kde.org");
    setAboutData(about);
    setButtons(Help | Apply | Default);

    setupUi(this);

    m_indexFolderSelectionDialog = new IndexFolderSelectionDialog(this);

    connect(m_checkEnableFileIndexer, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_checkEnabled, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_checkEnabled, SIGNAL(toggled(bool)),
            this, SLOT(updateEnabledItems()));

    connect(m_buttonCustomizeIndexFolders, SIGNAL(clicked()),
            this, SLOT(slotEditIndexFolders()));

    connect(m_checkboxSourceCode, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
}


ServerConfigModule::~ServerConfigModule()
{
}


void ServerConfigModule::load()
{
    // Basic setup
    KConfig config("baloofilerc");
    KConfigGroup basicSettings = config.group("Basic Settings");
    m_checkEnabled->setChecked(basicSettings.readEntry("Enabled", true));
    m_checkEnableFileIndexer->setChecked(basicSettings.readEntry("Indexing-Enabled", true));

    // File indexer settings
    KConfigGroup group = config.group("General");
    m_indexFolderSelectionDialog->setIndexHiddenFolders(group.readEntry("index hidden folders", false));

    QStringList includeFolders = group.readPathEntry("folders", defaultFolders());
    QStringList excludeFolders = group.readPathEntry("exclude folders", QStringList());

    m_indexFolderSelectionDialog->setFolders(includeFolders, excludeFolders);

    // MimeTypes
    QStringList mimetypes = config.group("General").readEntry("exclude mimetypes", defaultExcludeMimetypes());

    m_oldExcludeMimetypes = mimetypes;
    m_oldExcludeFolders = excludeFolders;
    m_oldIncludeFolders = includeFolders;

    // All values loaded -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::save()
{
    QStringList includeFolders = m_indexFolderSelectionDialog->includeFolders();
    QStringList excludeFolders = m_indexFolderSelectionDialog->excludeFolders();

    // Change the settings
    KConfig config("baloofilerc");
    KConfigGroup basicSettings = config.group("Basic Settings");
    basicSettings.writeEntry("Enabled", m_checkEnabled->isChecked());
    basicSettings.writeEntry("Indexing-Enabled", m_checkEnableFileIndexer->isChecked());

    // 2.2 Update normals paths
    config.group("General").writePathEntry("folders", includeFolders);
    config.group("General").writePathEntry("exclude folders", excludeFolders);

    // 2.3 Other stuff
    config.group("General").writeEntry("index hidden folders", m_indexFolderSelectionDialog->indexHiddenFolders());

    QStringList excludeMimetypes;
    if (m_checkboxSourceCode->isChecked())
        excludeMimetypes = sourceCodeMimeTypes();

    config.group("General").writeEntry("exclude mimetypes", excludeMimetypes);

    // Start Baloo
    if (m_checkEnabled) {
        const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file"));
        QProcess::startDetached(exe);
    }

    // Start cleaner
    bool cleaningRequired = false;
    if (includeFolders != m_oldIncludeFolders)
        cleaningRequired = true;
    else if (excludeFolders != m_oldExcludeFolders)
        cleaningRequired = true;
    else if (excludeMimetypes != m_oldExcludeMimetypes)
        cleaningRequired = true;

    if (cleaningRequired) {
        const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file_cleaner"));
        QProcess::startDetached(exe);
    }

    // all values saved -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::defaults()
{
    m_checkEnableFileIndexer->setChecked(true);
    m_checkEnabled->setChecked(true);
    m_indexFolderSelectionDialog->setIndexHiddenFolders(false);
    m_indexFolderSelectionDialog->setFolders(defaultFolders(), QStringList());
}


void ServerConfigModule::updateEnabledItems()
{
    bool checked = m_checkEnabled->isChecked();
    m_checkEnableFileIndexer->setEnabled(checked);
}


void ServerConfigModule::slotEditIndexFolders()
{
    FolderSelectionWidget* widget = new FolderSelectionWidget(this);
    widget->setMinimumWidth(600);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addWidget(widget);
    hLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    widget->show();
    verticalLayout_2->addItem(hLayout);

    QStringList exclude;
    exclude << "/home/vishesh/kde" << "/home/vishesh/kde5";

    widget->setFolders(m_oldIncludeFolders, exclude);

    const QStringList oldIncludeFolders = m_indexFolderSelectionDialog->includeFolders();
    const QStringList oldExcludeFolders = m_indexFolderSelectionDialog->excludeFolders();
    const bool oldIndexHidden = m_indexFolderSelectionDialog->indexHiddenFolders();

    if (m_indexFolderSelectionDialog->exec()) {
        changed();
    } else {
        // revert to previous settings
        m_indexFolderSelectionDialog->setFolders(oldIncludeFolders, oldExcludeFolders);
        m_indexFolderSelectionDialog->setIndexHiddenFolders(oldIndexHidden);
    }
}

#include "kcm.moc"
