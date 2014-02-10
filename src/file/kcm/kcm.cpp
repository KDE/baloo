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
#include "fileindexerinterface.h"
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
#include <QtCore/QDir>
#include <QtDBus/QDBusServiceWatcher>

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
    , m_fileIndexerInterface(0)
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

    QDBusServiceWatcher* watcher = new QDBusServiceWatcher(this);
    watcher->addWatchedService(QLatin1String("org.kde.baloo.file"));
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);

    connect(watcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(recreateInterfaces()));
    connect(watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(recreateInterfaces()));

    recreateInterfaces();

    connect(m_checkEnableFileIndexer, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_checkEnabled, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_checkEnabled, SIGNAL(toggled(bool)),
            this, SLOT(updateEnabledItems()));

    connect(m_buttonCustomizeIndexFolders, SIGNAL(clicked()),
            this, SLOT(slotEditIndexFolders()));
    connect(m_fileIndexerSuspendResumeButtom, SIGNAL(clicked(bool)),
            this, SLOT(slotFileIndexerSuspendResumeClicked()));

    connect(m_checkboxSourceCode, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
}


ServerConfigModule::~ServerConfigModule()
{
    delete m_fileIndexerInterface;
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

    recreateInterfaces();
    updateFileIndexerStatus();

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

    // 5. update state
    recreateInterfaces();
    updateFileIndexerStatus();

    // 6. all values saved -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::defaults()
{
    m_checkEnableFileIndexer->setChecked(true);
    m_checkEnabled->setChecked(true);
    m_indexFolderSelectionDialog->setIndexHiddenFolders(false);
    m_indexFolderSelectionDialog->setFolders(defaultFolders(), QStringList());
}


void ServerConfigModule::setFileIndexerStatusText(const QString& text, bool elide)
{
    m_labelFileIndexerStatus->setWordWrap(!elide);
    m_labelFileIndexerStatus->setTextElideMode(elide ? Qt::ElideMiddle : Qt::ElideNone);
    m_labelFileIndexerStatus->setText(text);
}


void ServerConfigModule::updateFileIndexerStatus()
{
    // FIXME: Make this non-blocking!
    QDBusPendingReply<QString> reply = m_fileIndexerInterface->statusMessage();
    reply.waitForFinished();

    if (reply.isError()) {
        setFileIndexerStatusText(i18nc("@info:status", "File indexing service not running."), false);
        return;
    }

    QString status = reply.value();
    if (status.isEmpty()) {
        setFileIndexerStatusText(i18nc("@info:status %1 is an error message returned by a dbus interface.",
                                        "Failed to contact File Indexer service (%1)",
                                        m_fileIndexerInterface->lastError().message()), false);
    } else {
        m_failedToInitialize = false;
        setFileIndexerStatusText(status, true);
        updateFileIndexerSuspendResumeButtonText(m_fileIndexerInterface->isSuspended());
    }
}

void ServerConfigModule::updateEnabledItems()
{
    bool checked = m_checkEnabled->isChecked();
    m_checkEnableFileIndexer->setEnabled(checked);
    m_fileIndexerSuspendResumeButtom->setEnabled(checked);
    m_labelFileIndexerStatus->setEnabled(checked);
}


void ServerConfigModule::updateFileIndexerSuspendResumeButtonText(bool isSuspended)
{
    if (isSuspended) {
        m_fileIndexerSuspendResumeButtom->setText(i18nc("Resumes the File indexing service.", "Resume"));
        m_fileIndexerSuspendResumeButtom->setIcon(KIcon("media-playback-start"));
    } else {
        m_fileIndexerSuspendResumeButtom->setText(i18nc("Suspends the File indexing service.", "Suspend"));
        m_fileIndexerSuspendResumeButtom->setIcon(KIcon("media-playback-pause"));
    }
}

void ServerConfigModule::slotFileIndexerSuspendResumeClicked()
{
    bool suspended = m_fileIndexerInterface->isSuspended();
    if (!suspended) {
        m_fileIndexerInterface->suspend();
        updateFileIndexerSuspendResumeButtonText(true);
    } else {
        m_fileIndexerInterface->resume();
        updateFileIndexerSuspendResumeButtonText(false);
    }
}


void ServerConfigModule::recreateInterfaces()
{
    delete m_fileIndexerInterface;

    m_fileIndexerInterface = new org::kde::baloo::file("org.kde.baloo.file", "/indexer",
                                                       QDBusConnection::sessionBus());

    connect(m_fileIndexerInterface, SIGNAL(statusChanged()),
            this, SLOT(updateFileIndexerStatus()));
}


void ServerConfigModule::slotEditIndexFolders()
{
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
