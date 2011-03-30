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

$Revision: 4183 $:
$Author: cohen@irascible.com $:
$Date: 2010-05-06 22:30:19 +0200 (Thu, 06 May 2010) $

********************************************************************/

#include "autoroutersettingsdialog.h"

#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QComboBox>

#include "../items/tracewire.h"


AutorouterSettingsDialog::AutorouterSettingsDialog(QWidget *parent) : QDialog(parent) 
{
	this->setWindowTitle(QObject::tr("Auorouter Settings"));

	QVBoxLayout * vLayout = new QVBoxLayout();

	QGroupBox * groupBox = new QGroupBox(tr("Production type"), this);

	QVBoxLayout * gLayout = new QVBoxLayout();

	m_homebrewButton = new QRadioButton(tr("homebrew"), this); 
	connect(m_homebrewButton, SIGNAL(clicked(bool)), this, SLOT(production(bool)));
	gLayout->addWidget(m_homebrewButton);

	m_professionalButton = new QRadioButton(tr("professional"), this); 
	connect(m_professionalButton, SIGNAL(clicked(bool)), this, SLOT(production(bool)));
	gLayout->addWidget(m_professionalButton);

	m_customButton = new QRadioButton(tr("custom"), this); 
	connect(m_customButton, SIGNAL(clicked(bool)), this, SLOT(production(bool)));
	gLayout->addWidget(m_customButton);

	groupBox->setLayout(gLayout);
	vLayout->addWidget(groupBox);
	vLayout->addSpacing(5);

	groupBox = new QGroupBox(tr("Trace width"), this);

	gLayout = new QVBoxLayout();
	QComboBox * comboBox = TraceWire::createWidthComboBox(Wire::STANDARD_TRACE_WIDTH, groupBox);

	connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(widthEntry(const QString &)));

	gLayout->addWidget(comboBox);
	groupBox->setLayout(gLayout);

	vLayout->addWidget(groupBox);
	vLayout->addSpacing(10);


    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptAnd()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);

	QSettings settings;

	/*
	QString szString = settings.value(QString("Autorouter"), "").toString();
	if (szString.isEmpty()) {
		m_inButton->setChecked(true);
		m_lineEdit->setText(QString::number(m_defaultSize));
	}
	else {
		if (szString.endsWith("mm")) {
			m_mmButton->setChecked(true);
			m_validator->setTop(25.4);
		}
		else {
			m_inButton->setChecked(true);
		}
		szString.chop(2);
		m_lineEdit->setText(szString);
	}
	*/
}

AutorouterSettingsDialog::~AutorouterSettingsDialog() {
}

void AutorouterSettingsDialog::production(bool checked) {
	QString units;
	if (sender() == m_homebrewButton) {
	}
	else if (sender() == m_professionalButton) {
	}
	else if (sender() == m_customButton) {
	}
	
}

void AutorouterSettingsDialog::acceptAnd() {
	//QSettings settings;
	//settings.setValue(QString("%1GridSize").arg(m_viewName), m_lineEdit->text() + units);
	accept();
}

void AutorouterSettingsDialog::restoreDefault() {
	//m_inButton->setChecked(true);
	//m_mmButton->setChecked(false);
}
