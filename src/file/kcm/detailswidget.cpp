/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "detailswidget.h"


#include <KLocalizedString>

#include <QVBoxLayout>

using namespace Baloo;

DetailsWidget::DetailsWidget(QWidget* parent, Qt::WindowFlags flags)
    : KDialog(parent, flags)
{
    // KDialog stuff
    setCaption(i18n("Desktop Search Details"));
    setButtons(KDialog::Close | KDialog::User1);
    setButtonText(KDialog::User1, i18n("Refresh"));

    connect(this, SIGNAL(user1Clicked()), this, SLOT(refresh()));

    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(KIcon("edit-find").pixmap(48, 48));
    iconLabel->setMinimumSize(QSize(48, 48));
    iconLabel->setMaximumSize(QSize(48, 48));

    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(iconLabel);
    verticalLayout->addItem(verticalSpacer);

    QLabel* filesLabel = new QLabel(i18n("Files"));
    QLabel* emailsLabel = new QLabel(i18n("Emails"));

    m_emailCountLabel = new QLabel();
    m_fileCountLabel = new QLabel();

    QLabel* detailsLabel = new QLabel(i18n("Desktop Search Details"));
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    detailsLabel->setFont(font);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget(detailsLabel, 0, 0, 1, 2);
    gridLayout->addItem(verticalLayout, 0, 3, 5, 1);

    // Starting from row 2. Skipping one row to add some space
    gridLayout->addWidget(filesLabel, 2, 0);
    gridLayout->addWidget(m_fileCountLabel, 2, 1);
    gridLayout->addItem(new QSpacerItem(10, 1, QSizePolicy::Expanding), 2, 2);

    gridLayout->addWidget(emailsLabel, 3, 0);
    gridLayout->addWidget(m_emailCountLabel, 3, 1);
    gridLayout->addItem(new QSpacerItem(10, 1, QSizePolicy::Expanding), 3, 2);

    QWidget* mainWidget = new QWidget(this);
    mainWidget->setLayout(gridLayout);
    setMainWidget(mainWidget);

    refresh();
}

DetailsWidget::~DetailsWidget()
{

}

void DetailsWidget::refresh()
{
    m_fileCountLabel->setText(i18n("Calculating"));
    m_emailCountLabel->setText(i18n("Calculating"));

}

void DetailsWidget::slotFileCountFinished()
{
    /*
    int num = query->binding(0).literal().toInt();
    m_fileCountLabel->setText(i18n("%1", num));
    query->close();

    QLatin1String queryStr("select count(distinct ?r) where { ?r a aneo:AkonadiDataObject . }");

    Soprano::Model* model = ResourceManager::instance()->mainModel();
    Soprano::Util::AsyncQuery* q
        = Soprano::Util::AsyncQuery::executeQuery(model, queryStr, Soprano::Query::QueryLanguageSparql);

    connect(q, SIGNAL(nextReady(Soprano::Util::AsyncQuery*)),
            this, SLOT(slotEmailCountFinished(Soprano::Util::AsyncQuery*)));
            */
}

void DetailsWidget::slotEmailCountFinished()
{
    /*
    int num = query->binding(0).literal().toInt();
    m_emailCountLabel->setText(i18n("%1", num));

    query->close();
    */
}

