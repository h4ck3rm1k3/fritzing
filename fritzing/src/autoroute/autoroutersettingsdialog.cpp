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


///////////////////////////////////////

// todo: 
//	save and reload as settings
//	enable/disable custom on radio presses
//	change wording on custom via
//	actually modify the autorouter
//	enable single vs. double-sided settings


///////////////////////////////////////

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
#include "../sketch/pcbsketchwidget.h"


AutorouterSettingsDialog::AutorouterSettingsDialog(QWidget *parent) : QDialog(parent) 
{
	PCBSketchWidget::getDefaultViaSize(m_holeSettings.ringThickness, m_holeSettings.holeDiameter);

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

	QFrame * customFrame = new QFrame(this);
	QHBoxLayout * customFrameLayout = new QHBoxLayout(this);
	customFrameLayout->addSpacing(10);

	QGroupBox * customGroupBox = new QGroupBox("Via size", this);
	QVBoxLayout * customLayout = new QVBoxLayout();
	m_holeSettings.ringThicknessRange = Via::ringThicknessRange;
	m_holeSettings.holeDiameterRange = Via::holeDiameterRange;
	QWidget * customWidget = Hole::createHoleSettings(customGroupBox, m_holeSettings, true, "");
	enableCustom(initRadios());
	customLayout->addWidget(customWidget);
	customGroupBox->setLayout(customLayout);



	customFrameLayout->addWidget(customGroupBox);
	customFrame->setLayout(customFrameLayout);

	gLayout->addWidget(customFrame);

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
		enableCustom(false);
	}
	else if (sender() == m_professionalButton) {
		enableCustom(false);
	}
	else if (sender() == m_customButton) {
		enableCustom(true);
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

void AutorouterSettingsDialog::enableCustom(bool enable) 
{
	m_holeSettings.diameterEdit->setEnabled(enable);
	m_holeSettings.thicknessEdit->setEnabled(enable);
	m_holeSettings.unitsComboBox->setEnabled(enable);
	m_holeSettings.sizesComboBox->setEnabled(enable);
}

bool AutorouterSettingsDialog::initRadios() 
{

	QFile file(":/resources/vias.xml");

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("failed loading properties %1 line:%2 col:%3").arg(errorStr).arg(errorLine).arg(errorColumn));
		return false;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) return false;
	if (root.tagName() != "vias") return false;

	QDomElement viaElement = root.firstChildElement("via");
	if (viaElement.isNull()) return false;

	bool custom = true;

	while (!viaElement.isNull()) {
		QString name = viaElement.attribute("name");
		QString ringThickness = viaElement.attribute("ringthickness");
		QString holeSize = viaElement.attribute("holesize");
		if (!name.isEmpty() && !ringThickness.isEmpty() && !holeSize.isEmpty()) {
			QRadioButton * button = NULL;
			if (name.compare("homebrew", Qt::CaseInsensitive) == 0) button = m_homebrewButton;
			else if (name.compare("professional", Qt::CaseInsensitive) == 0) button = m_professionalButton;
			if (button) {
				button->setProperty("ringthickness", ringThickness);
				button->setProperty("holesize", holeSize);
				if (ringThickness.compare(m_holeSettings.ringThickness, Qt::CaseInsensitive) == 0 && holeSize.compare(m_holeSettings.holeDiameter, Qt::CaseInsensitive) == 0) {
					button->setChecked(true);
					custom = false;
				}
			}
		}

		viaElement = viaElement.nextSiblingElement("via");
	}	

	m_customButton->setChecked(custom);

	return custom;
}
