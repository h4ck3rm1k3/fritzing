/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

#include <QListWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileInfoList>
#include <QXmlStreamReader>
#include <QLabel>

#include "recoverydialog.h"
#include "../utils/folderutils.h"
#include "../debugdialog.h"

RecoveryDialog::RecoveryDialog(QFileInfoList fileInfoList, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags)
{
    m_recoveryList = new QListWidget(this);
    m_recoveryList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QListWidgetItem *item;
    foreach (QFileInfo fileInfo, fileInfoList) {
        item = new QListWidgetItem;
		QString originalName = getOriginalName(fileInfo.absoluteFilePath());
		item->setData(Qt::UserRole, fileInfo.absoluteFilePath());
        if (!originalName.isEmpty()) {
            item->setText(originalName);
        }
        else {
            item->setText(fileInfo.fileName());
        }
		DebugDialog::debug(QString("Displaying recoveryDialog text %1 and data %2").arg(item->text()).arg(item->data(Qt::UserRole).value<QString>()));
        m_recoveryList->addItem(item);
    }

    QVBoxLayout *layout = new QVBoxLayout();

	QLabel * label = new QLabel;
	label->setText(tr("Select any files you want to recover from the list below"));

	layout->addWidget(label);
    layout->addWidget(m_recoveryList);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    layout->addLayout(buttonLayout);

    QPushButton *recover = new QPushButton(tr("&Recover"));
    recover->setDefault(true);
	recover->setMaximumWidth(100);
    connect(recover, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *ignore = new QPushButton(tr("&Ignore"));
	ignore->setMaximumWidth(100);
    connect(ignore, SIGNAL(clicked()), this, SLOT(reject()));

	buttonLayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding));
    buttonLayout->addWidget(recover);
	buttonLayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding));
    buttonLayout->addWidget(ignore);
	buttonLayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding));

    setLayout(layout);
}

 QList<QListWidgetItem*> RecoveryDialog::getSelectedFiles() {
    return m_recoveryList->selectedItems();
}

 QString RecoveryDialog::getOriginalName(const QString & path) 
 {
    QString originalName;
    QFile file(path);
	if (!file.open(QFile::ReadOnly)) {
		// TODO: not sure how else to handle this...
		DebugDialog::debug(QString("unable to open recovery file %1").arg(path));
		return originalName;
	}

    QXmlStreamReader xml(&file);
    xml.setNamespaceProcessing(false);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();
            if (name == "originalFileName") {
                originalName = xml.readElementText();
                break;
            }
        }
    }
    file.close();
	return originalName;
 }
