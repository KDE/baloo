/* This file is part of the KDE Project
   Copyright (c) 2007-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2013 Vishesh Handa <me@vhanda.in>

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
#include "nepomukserverinterface.h"
#include "servicecontrol.h"
#include "fileexcludefilters.h"
#include "fileindexerinterface.h"
#include "indexfolderselectiondialog.h"
#include "excludefilterselectiondialog.h"
#include "detailswidget.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>
#include <KMessageBox>
#include <KUrlLabel>
#include <KProcess>
#include <KStandardDirs>
#include <KCalendarSystem>
#include <KDirWatch>
#include <KDebug>

#include <QRadioButton>
#include <QInputDialog>
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
      m_serverInterface(0),
      m_fileIndexerInterface(0),
      m_failedToInitialize(false),
      m_checkboxesChanged(false)
{
    KAboutData* about = new KAboutData(
        "kcm_baloofile", "kcm_baloofile", ki18n("Desktop Search Configuration Module"),
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
    watcher->addWatchedService(QLatin1String("org.kde.nepomuk.services.nepomukfileindexer"));
    watcher->addWatchedService(QLatin1String("org.kde.NepomukServer"));
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);

    connect(watcher, SIGNAL(serviceRegistered(const QString&)),
            this, SLOT(recreateInterfaces()));
    connect(watcher, SIGNAL(serviceUnregistered(const QString&)),
            this, SLOT(recreateInterfaces()));

    recreateInterfaces();

    connect(m_checkEnableFileIndexer, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_checkEnableNepomuk, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_checkEnableEmailIndexer, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_comboRemovableMediaHandling, SIGNAL(activated(int)),
            this, SLOT(changed()));

    connect(m_buttonCustomizeIndexFolders, SIGNAL(clicked()),
            this, SLOT(slotEditIndexFolders()));
    connect(m_buttonAdvancedFileIndexing, SIGNAL(clicked()),
            this, SLOT(slotAdvancedFileIndexing()));
    connect(m_fileIndexerSuspendResumeButtom, SIGNAL(clicked(bool)),
            this, SLOT(slotFileIndexerSuspendResumeClicked()));
    connect(m_emailIndexerSuspendResumeButtom, SIGNAL(clicked(bool)),
            this, SLOT(slotEmailIndexerSuspendResumeClicked()));
    connect(m_buttonDetails, SIGNAL(leftClickedUrl()),
            this, SLOT(slotStatusDetailsClicked()));

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

    // args[0] can be the page index allowing to open the config with a specific page
    if (args.count() > 0 && args[0].toInt() < m_mainTabWidget->count()) {
        m_mainTabWidget->setCurrentIndex(args[0].toInt());
    }
}


ServerConfigModule::~ServerConfigModule()
{
    delete m_fileIndexerInterface;
    delete m_serverInterface;
}


void ServerConfigModule::load()
{
    // 1. basic setup
    KConfig config("nepomukserverrc");
    m_checkEnableNepomuk->setChecked(config.group("Basic Settings").readEntry("Start Nepomuk", true));
    m_checkEnableFileIndexer->setChecked(config.group("Service-nepomukfileindexer").readEntry("autostart", true));

    const QString akonadiCtlExe = KStandardDirs::findExe("akonadictl");
    m_emailIndexingBox->setVisible(!akonadiCtlExe.isEmpty());
    KConfig akonadiConfig("akonadi_nepomuk_feederrc");
    m_checkEnableEmailIndexer->setChecked(akonadiConfig.group("akonadi_nepomuk_email_feeder").readEntry("Enabled", true));

    // 2. file indexer settings
    KConfig fileIndexerConfig("nepomukstrigirc");
    KConfigGroup group = fileIndexerConfig.group("General");
    m_indexFolderSelectionDialog->setIndexHiddenFolders(group.readEntry("index hidden folders", false));

    QStringList includeFolders = group.readPathEntry("folders", defaultFolders());
    QStringList excludeFolders = group.readPathEntry("exclude folders", QStringList());

    /*
    QScopedPointer<RemovableMediaCache> rmc(new Nepomuk2::RemovableMediaCache(this));
    QList< const RemovableMediaCache::Entry* > allMedia = rmc->allMedia();
    Q_FOREACH (const RemovableMediaCache::Entry * entry, allMedia) {
        QByteArray groupName("Device-" + entry->url().toUtf8());
        if (!fileIndexerConfig.hasGroup(groupName))
            continue;

        KConfigGroup grp = fileIndexerConfig.group(groupName);

        QString mountPath = grp.readEntry("mount path", QString());
        if (mountPath.isEmpty())
            continue;

        QStringList includes = grp.readPathEntry("folders", defaultFolders());
        Q_FOREACH (const QString & path, includes) {
            if (path == QLatin1String("/"))
                includeFolders << mountPath;
            else
                includeFolders << mountPath + path;
        }

        QStringList excludes = grp.readPathEntry("exclude folders", QStringList());
        Q_FOREACH (const QString & path, excludes) {
            if (path == QLatin1String("/"))
                excludeFolders << mountPath;
            else
                excludeFolders << mountPath + path;
        }
    }
    */

    m_indexFolderSelectionDialog->setFolders(includeFolders, excludeFolders);

    m_excludeFilterSelectionDialog->setExcludeFilters(fileIndexerConfig.group("General").readEntry("exclude filters", Baloo::defaultExcludeFilterList()));

    // MimeTypes
    QStringList mimetypes = fileIndexerConfig.group("General").readEntry("exclude mimetypes", defaultExcludeMimetypes());
    m_excludeFilterSelectionDialog->setExcludeMimeTypes(mimetypes);
    syncCheckBoxesFromMimetypes(mimetypes);

    const bool indexNewlyMounted = fileIndexerConfig.group("RemovableMedia").readEntry("index newly mounted", false);
    const bool askIndividually = fileIndexerConfig.group("RemovableMedia").readEntry("ask user", false);
    // combobox items: 0 - ignore, 1 - index all, 2 - ask user
    m_comboRemovableMediaHandling->setCurrentIndex(int(indexNewlyMounted) + int(askIndividually));

    groupBox->setEnabled(m_checkEnableNepomuk->isChecked());

    recreateInterfaces();
    updateFileIndexerStatus();
    updateNepomukServerStatus();

    // 7. all values loaded -> no changes
    m_checkboxesChanged = false;
    Q_EMIT changed(false);
}


void ServerConfigModule::save()
{
    QStringList includeFolders = m_indexFolderSelectionDialog->includeFolders();
    QStringList excludeFolders = m_indexFolderSelectionDialog->excludeFolders();

    // 1. change the settings (in case the server is not running)
    KConfig config("nepomukserverrc");
    config.group("Basic Settings").writeEntry("Start Nepomuk", m_checkEnableNepomuk->isChecked());
    config.group("Service-nepomukfileindexer").writeEntry("autostart", m_checkEnableFileIndexer->isChecked());

    KConfig akonadiConfig("akonadi_nepomuk_feederrc");
    akonadiConfig.group("akonadi_nepomuk_email_feeder").writeEntry("Enabled", m_checkEnableEmailIndexer->isChecked());
    akonadiConfig.sync();
    QDBusInterface akonadiIface("org.freedesktop.Akonadi.Agent.akonadi_nepomuk_email_feeder", "/", "org.freedesktop.Akonadi.Agent.Control");
    akonadiIface.asyncCall("reconfigure");

    // 2. update file indexer config
    KConfig fileIndexerConfig("nepomukstrigirc");

    // 2.1 Update all the RemovableMedia paths
    /*
    QScopedPointer<RemovableMediaCache> rmc(new Nepomuk2::RemovableMediaCache(this));
    QList< const RemovableMediaCache::Entry* > allMedia = rmc->allMedia();
    Q_FOREACH (const RemovableMediaCache::Entry * entry, allMedia) {
        QByteArray groupName("Device-" + entry->url().toUtf8());
        KConfigGroup group = fileIndexerConfig.group(groupName);

        QString mountPath = entry->mountPath();
        if (mountPath.isEmpty())
            continue;

        group.writeEntry("mount path", mountPath);

        QStringList includes;
        QMutableListIterator<QString> it(includeFolders);
        while (it.hasNext()) {
            QString fullPath = it.next();
            if (fullPath.startsWith(mountPath)) {
                QString path = fullPath.mid(mountPath.length());
                if (!path.isEmpty())
                    includes << path;
                else
                    includes << QLatin1String("/");
                it.remove();
            }
        }

        QStringList excludes;
        QMutableListIterator<QString> iter(excludeFolders);
        while (iter.hasNext()) {
            QString fullPath = iter.next();
            if (fullPath.startsWith(mountPath)) {
                QString path = fullPath.mid(mountPath.length());
                if (!path.isEmpty())
                    excludes << path;
                else
                    excludes << QLatin1String("/");
                iter.remove();
            }
        }

        if (includes.isEmpty() && excludes.isEmpty())
            excludes << QString("/");

        group.writePathEntry("folders", includes);
        group.writePathEntry("exclude folders", excludes);
    }
    */

    // 2.2 Update normals paths
    fileIndexerConfig.group("General").writePathEntry("folders", includeFolders);
    fileIndexerConfig.group("General").writePathEntry("exclude folders", excludeFolders);

    // 2.3 Other stuff
    fileIndexerConfig.group("General").writeEntry("index hidden folders", m_indexFolderSelectionDialog->indexHiddenFolders());
    fileIndexerConfig.group("General").writeEntry("exclude filters", m_excludeFilterSelectionDialog->excludeFilters());

    QStringList excludeMimetypes = m_excludeFilterSelectionDialog->excludeMimeTypes();
    if (m_checkboxesChanged) {
        excludeMimetypes = mimetypesFromCheckboxes();
        m_checkboxesChanged = false;
    }

    fileIndexerConfig.group("General").writeEntry("exclude mimetypes", excludeMimetypes);

    // combobox items: 0 - ignore, 1 - index all, 2 - ask user
    fileIndexerConfig.group("RemovableMedia").writeEntry("index newly mounted", m_comboRemovableMediaHandling->currentIndex() > 0);
    fileIndexerConfig.group("RemovableMedia").writeEntry("ask user", m_comboRemovableMediaHandling->currentIndex() == 2);

    // 4. update the current state of the nepomuk server
    if (m_serverInterface->isValid()) {
        m_serverInterface->enableNepomuk(m_checkEnableNepomuk->isChecked());
        m_serverInterface->enableFileIndexer(m_checkEnableFileIndexer->isChecked());
    } else if (m_checkEnableNepomuk->isChecked()) {
        if (!QProcess::startDetached(QLatin1String("nepomukserver"))) {
            KMessageBox::error(this,
                               i18n("Failed to start the desktop search service (Nepomuk). The settings have been saved "
                                    "and will be used the next time the server is started."),
                               i18n("Desktop search service not running"));
        }
    }


    // 5. update state
    recreateInterfaces();
    updateFileIndexerStatus();
    updateNepomukServerStatus();


    // 6. all values saved -> no changes
    m_checkboxesChanged = false;
    Q_EMIT changed(false);
}


void ServerConfigModule::defaults()
{
    m_checkEnableFileIndexer->setChecked(true);
    m_checkEnableNepomuk->setChecked(true);
    m_checkEnableEmailIndexer->setChecked(true);
    m_indexFolderSelectionDialog->setIndexHiddenFolders(false);
    m_indexFolderSelectionDialog->setFolders(defaultFolders(), QStringList());
    m_excludeFilterSelectionDialog->setExcludeFilters(Baloo::defaultExcludeFilterList());
}


void ServerConfigModule::updateNepomukServerStatus()
{
    if (m_serverInterface &&
            m_serverInterface->isNepomukEnabled()) {
        m_labelNepomukStatus->setText(i18nc("@info:status", "Desktop search services are active"));
    } else {
        m_labelNepomukStatus->setText(i18nc("@info:status", "Desktop search services are disabled"));
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
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.nepomuk.services.nepomukfileindexer")) {
        if (org::kde::nepomuk::ServiceControl("org.kde.nepomuk.services.nepomukfileindexer", "/servicecontrol",
                                              QDBusConnection::sessionBus()).isInitialized()) {
            QString status = m_fileIndexerInterface->userStatusString();
            if (status.isEmpty()) {
                setFileIndexerStatusText(i18nc("@info:status %1 is an error message returned by a dbus interface.",
                                               "Failed to contact File Indexer service (%1)",
                                               m_fileIndexerInterface->lastError().message()), false);
            } else {
                m_failedToInitialize = false;
                setFileIndexerStatusText(status, true);
                updateFileIndexerSuspendResumeButtonText(m_fileIndexerInterface->isSuspended());
            }
        } else {
            m_failedToInitialize = true;
            setFileIndexerStatusText(i18nc("@info:status", "File indexing service failed to initialize, "
                                           "most likely due to an installation problem."), false);
        }
    } else if (!m_failedToInitialize) {
        setFileIndexerStatusText(i18nc("@info:status", "File indexing service not running."), false);
    }
}

void ServerConfigModule::updateFileIndexerSuspendResumeButtonText(bool isSuspended)
{
    if (isSuspended) {
        m_fileIndexerSuspendResumeButtom->setText(i18nc("Resumes the Nepomuk file indexing service.", "Resume"));
        m_fileIndexerSuspendResumeButtom->setIcon(KIcon("media-playback-start"));
    } else {
        m_fileIndexerSuspendResumeButtom->setText(i18nc("Suspends the Nepomuk file indexing service.", "Suspend"));
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
    delete m_serverInterface;

    m_fileIndexerInterface = new org::kde::nepomuk::FileIndexer("org.kde.nepomuk.services.nepomukfileindexer", "/nepomukfileindexer", QDBusConnection::sessionBus());
    m_serverInterface = new org::kde::NepomukServer("org.kde.NepomukServer", "/nepomukserver", QDBusConnection::sessionBus());

    connect(m_fileIndexerInterface, SIGNAL(statusChanged()),
            this, SLOT(updateFileIndexerStatus()));
}


void ServerConfigModule::slotStatusDetailsClicked()
{
    DetailsWidget dialog(this);
    dialog.exec();
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
