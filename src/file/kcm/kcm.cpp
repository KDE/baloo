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
#include "excludefilterselectiondialog.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>
#include <KDirWatch>
#include <KDebug>

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
    : KCModule(BalooConfigModuleFactory::componentData(), parent, args),
      m_fileIndexerInterface(0),
      m_failedToInitialize(false),
      m_checkboxesChanged(false)
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
    m_excludeFilterSelectionDialog = new ExcludeFilterSelectionDialog(this);

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

    connect(m_buttonCustomizeIndexFolders, SIGNAL(clicked()),
            this, SLOT(slotEditIndexFolders()));
    connect(m_buttonAdvancedFileIndexing, SIGNAL(clicked()),
            this, SLOT(slotAdvancedFileIndexing()));
    connect(m_fileIndexerSuspendResumeButtom, SIGNAL(clicked(bool)),
            this, SLOT(slotFileIndexerSuspendResumeClicked()));

    connect(m_checkboxAudio, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckBoxesChanged()));
    connect(m_checkboxImage, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckBoxesChanged()));
    connect(m_checkboxVideo, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckBoxesChanged()));
    connect(m_checkboxDocuments, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckBoxesChanged()));
    connect(m_checkboxSourceCode, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckBoxesChanged()));
}


ServerConfigModule::~ServerConfigModule()
{
    delete m_fileIndexerInterface;
}


void ServerConfigModule::load()
{
    // 1. basic setup
    KConfig config("baloofilerc");
    KConfigGroup basicSettings = config.group("Basic Settings");
    m_checkEnabled->setChecked(basicSettings.readEntry("Enabled", true));
    m_checkEnableFileIndexer->setChecked(basicSettings.readEntry("Indexing-Enabled", true));

    // 2. file indexer settings
    KConfigGroup group = config.group("General");
    m_indexFolderSelectionDialog->setIndexHiddenFolders(group.readEntry("index hidden folders", false));

    QStringList includeFolders = group.readPathEntry("folders", defaultFolders());
    QStringList excludeFolders = group.readPathEntry("exclude folders", QStringList());

    m_indexFolderSelectionDialog->setFolders(includeFolders, excludeFolders);

    m_excludeFilterSelectionDialog->setExcludeFilters(config.group("General").readEntry("exclude filters", Baloo::defaultExcludeFilterList()));

    // MimeTypes
    QStringList mimetypes = config.group("General").readEntry("exclude mimetypes", defaultExcludeMimetypes());
    m_excludeFilterSelectionDialog->setExcludeMimeTypes(mimetypes);
    syncCheckBoxesFromMimetypes(mimetypes);

    groupBox->setEnabled(m_checkEnabled->isChecked());

    recreateInterfaces();
    updateFileIndexerStatus();
    updateFileServerStatus();

    // 7. all values loaded -> no changes
    m_checkboxesChanged = false;
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
    config.group("General").writeEntry("exclude filters", m_excludeFilterSelectionDialog->excludeFilters());

    QStringList excludeMimetypes = m_excludeFilterSelectionDialog->excludeMimeTypes();
    if (m_checkboxesChanged) {
        excludeMimetypes = mimetypesFromCheckboxes();
        m_checkboxesChanged = false;
    }

    config.group("General").writeEntry("exclude mimetypes", excludeMimetypes);

    // TODO: Update the baloo_file process or just restart it!

    // 5. update state
    recreateInterfaces();
    updateFileIndexerStatus();
    updateFileServerStatus();


    // 6. all values saved -> no changes
    m_checkboxesChanged = false;
    Q_EMIT changed(false);
}


void ServerConfigModule::defaults()
{
    m_checkEnableFileIndexer->setChecked(true);
    m_checkEnabled->setChecked(true);
    m_indexFolderSelectionDialog->setIndexHiddenFolders(false);
    m_indexFolderSelectionDialog->setFolders(defaultFolders(), QStringList());
    m_excludeFilterSelectionDialog->setExcludeFilters(Baloo::defaultExcludeFilterList());
}


void ServerConfigModule::updateFileServerStatus()
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.baloo.file")) {
        m_labelStatus->setText(i18nc("@info:status", "File Metadata services are active"));
    } else {
        m_labelStatus->setText(i18nc("@info:status", "File Metadata services are disabled"));
    }
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

void ServerConfigModule::slotAdvancedFileIndexing()
{
    const QStringList oldExcludeFilters = m_excludeFilterSelectionDialog->excludeFilters();
    QStringList oldExcludeMimeTypes = m_excludeFilterSelectionDialog->excludeMimeTypes();

    if (m_checkboxesChanged) {
        oldExcludeMimeTypes = mimetypesFromCheckboxes();
        m_excludeFilterSelectionDialog->setExcludeMimeTypes(oldExcludeMimeTypes);
        m_checkboxesChanged = false;
    }

    if (m_excludeFilterSelectionDialog->exec()) {
        changed();

        QStringList mimetypes = m_excludeFilterSelectionDialog->excludeMimeTypes();
        syncCheckBoxesFromMimetypes(mimetypes);
    } else {
        m_excludeFilterSelectionDialog->setExcludeFilters(oldExcludeFilters);
        m_excludeFilterSelectionDialog->setExcludeMimeTypes(oldExcludeMimeTypes);
    }
}


namespace
{
bool containsRegex(const QStringList& list, const QString& regex)
{
    QRegExp exp(regex, Qt::CaseInsensitive, QRegExp::Wildcard);
    Q_FOREACH (const QString & string, list) {
        if (string.contains(exp))
            return true;;
    }
    return false;
}

void syncCheckBox(const QStringList& mimetypes, const QString& type, QCheckBox* checkbox)
{
    if (containsRegex(mimetypes, type)) {
        if (mimetypes.contains(type))
            checkbox->setChecked(false);
        else
            checkbox->setCheckState(Qt::PartiallyChecked);
    } else {
        checkbox->setChecked(true);
    }
}

void syncCheckBox(const QStringList& mimetypes, const QStringList& types, QCheckBox* checkbox)
{
    bool containsAll = true;
    bool containsAny = false;

    Q_FOREACH (const QString & type, types) {
        if (mimetypes.contains(type)) {
            containsAny = true;
        } else {
            containsAll = false;
        }
    }

    if (containsAll)
        checkbox->setCheckState(Qt::Unchecked);
    else if (containsAny)
        checkbox->setCheckState(Qt::PartiallyChecked);
    else
        checkbox->setCheckState(Qt::Checked);
}

}

void ServerConfigModule::syncCheckBoxesFromMimetypes(const QStringList& mimetypes)
{
    syncCheckBox(mimetypes, QLatin1String("image/*"), m_checkboxImage);
    syncCheckBox(mimetypes, QLatin1String("audio/*"), m_checkboxAudio);
    syncCheckBox(mimetypes, QLatin1String("video/*"), m_checkboxVideo);

    syncCheckBox(mimetypes, documentMimetypes(), m_checkboxDocuments);
    syncCheckBox(mimetypes, sourceCodeMimeTypes(), m_checkboxSourceCode);
    m_checkboxesChanged = false;
}

QStringList ServerConfigModule::mimetypesFromCheckboxes()
{
    QStringList types;
    if (!m_checkboxAudio->isChecked())
        types << QLatin1String("audio/*");
    if (!m_checkboxImage->isChecked())
        types << QLatin1String("image/*");
    if (!m_checkboxVideo->isChecked())
        types << QLatin1String("video/*");
    if (!m_checkboxDocuments->isChecked())
        types << documentMimetypes();
    if (!m_checkboxSourceCode->isChecked())
        types << sourceCodeMimeTypes();

    return types;
}

void ServerConfigModule::slotCheckBoxesChanged()
{
    m_checkboxesChanged = true;;
    changed(true);
}

#include "kcm.moc"
