/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "removabledeviceindexnotification.h"
//#include "fileindexerinterface.h"

#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>
#include <KIcon>
#include <QDebug>
#include <KToolInvocation>

#include <Solid/StorageAccess>

RemovableDeviceIndexNotification::RemovableDeviceIndexNotification(const Baloo::RemovableMediaCache::Entry* medium,
        QObject* parent)
    : KNotification(QLatin1String("nepomuk_new_removable_device"),
                    KNotification::Persistent,
                    parent),
    m_medium(medium)
{
    setTitle(i18nc("@title", "New removable device detected"));
    setText(i18nc("@info", "Do you want files on removable device <resource>%1</resource> to be indexed for fast desktop searches?", m_medium->device().description()));
    setPixmap(KIcon(QLatin1String("nepomuk")).pixmap(32, 32));

    setActions(QStringList()
               << i18nc("@action", "Index files")
               << i18nc("@action", "Ignore device")
               << i18nc("@action", "Configure"));
    connect(this, SIGNAL(activated(uint)), this, SLOT(slotActionActivated(uint)));

    // as soon as the device is unmounted this notification becomes pointless
    if (const Solid::StorageAccess* storage = m_medium->device().as<Solid::StorageAccess>()) {
        connect(storage, SIGNAL(accessibilityChanged(bool,QString)), SLOT(close()));
    }
}

void RemovableDeviceIndexNotification::slotActionDoIndexActivated()
{
    KConfig fileIndexerConfig("nepomukstrigirc");
    KConfigGroup group = fileIndexerConfig.group(QByteArray("Device-") + m_medium->url().toUtf8());
    group.writeEntry("mount path", m_medium->mountPath());
    group.writePathEntry("folders", QStringList() << QLatin1String("/"));

    // We don't need to inform the fileindexer. The configuration file would have
    // changed and the file indexer will update itself
    close();
}

void RemovableDeviceIndexNotification::slotActionDoNotIndexActivated()
{
    KConfig fileIndexerConfig("nepomukstrigirc");
    KConfigGroup group = fileIndexerConfig.group(QByteArray("Device-") + m_medium->url().toUtf8());
    group.writeEntry("mount path", m_medium->mountPath());
    group.writePathEntry("exclude folders", QStringList() << QLatin1String("/"));

    close();
}

void RemovableDeviceIndexNotification::slotActionConfigureActivated()
{
    QStringList args;
    args << "kcm_nepomuk" << "--args" << "1";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void RemovableDeviceIndexNotification::slotActionActivated(uint action)
{
    qDebug() << action;
    switch (action) {
    case 1:
        slotActionDoIndexActivated();
        break;
    case 2:
        slotActionDoNotIndexActivated();
        break;
    case 3:
        slotActionConfigureActivated();
        break;
    }
}

#include "removabledeviceindexnotification.moc"
