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
#include "../items/hole.h"
#include "../fsvgrenderer.h"
#include "../sketch/pcbsketchwidget.h"
#include "../utils/textutils.h"
#include "../utils/graphicsutils.h"


const QString AutorouterSettingsDialog::AutorouteTraceWidth = "autorouteTraceWidth";

AutorouterSettingsDialog::AutorouterSettingsDialog(QWidget *parent) : QDialog(parent) 
{
	QSettings settings;
	m_traceWidth = settings.value(AutorouteTraceWidth, "0").toInt();
	if (m_traceWidth == 0) {
		m_traceWidth = GraphicsUtils::pixels2mils(Wire::STANDARD_TRACE_WIDTH, FSvgRenderer::printerScale());
		settings.setValue(AutorouteTraceWidth, m_traceWidth);
	}

	Hole::initHoleSettings(m_holeSettings);
	m_holeSettings.ringThicknessRange = Via::ringThicknessRange;
	m_holeSettings.holeDiameterRange = Via::holeDiameterRange;

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

	m_customGroupBox = new QGroupBox("Via size", this);
	QVBoxLayout * customLayout = new QVBoxLayout();
	QWidget * customWidget = Hole::createHoleSettings(m_customGroupBox, m_holeSettings, true, "");

	connect(m_holeSettings.sizesComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeHoleSize(const QString &)));
	connect(m_holeSettings.unitsComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeUnits(const QString &)));
	connect(m_holeSettings.diameterEdit, SIGNAL(editingFinished()), this, SLOT(changeDiameter()));
	connect(m_holeSettings.thicknessEdit, SIGNAL(editingFinished()), this, SLOT(changeThickness()));

	enableCustom(initRadios());
	customLayout->addWidget(customWidget);
	m_customGroupBox->setLayout(customLayout);

	customFrameLayout->addWidget(m_customGroupBox);
	customFrame->setLayout(customFrameLayout);

	gLayout->addWidget(customFrame);

	groupBox->setLayout(gLayout);
	vLayout->addWidget(groupBox);
	vLayout->addSpacing(5);

	groupBox = new QGroupBox(tr("Trace width"), this);

	gLayout = new QVBoxLayout();
	QComboBox * comboBox = TraceWire::createWidthComboBox(m_traceWidth, groupBox);
	connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(widthEntry(const QString &)));

	gLayout->addWidget(comboBox);
	groupBox->setLayout(gLayout);

	vLayout->addWidget(groupBox);

	vLayout->addSpacerItem(new QSpacerItem (1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
	vLayout->addSpacing(10);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptAnd()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);
}

AutorouterSettingsDialog::~AutorouterSettingsDialog() {
}

void AutorouterSettingsDialog::production(bool checked) {
	Q_UNUSED(checked);

	QString units;
	if (sender() == m_homebrewButton) {
		enableCustom(false);
		changeHoleSize(sender()->property("holesize").toString() + "," + sender()->property("ringthickness").toString());
	}
	else if (sender() == m_professionalButton) {
		enableCustom(false);
		changeHoleSize(sender()->property("holesize").toString() + "," + sender()->property("ringthickness").toString());
	}
	else if (sender() == m_customButton) {
		enableCustom(true);
	}	
}

void AutorouterSettingsDialog::acceptAnd() {
	QSettings settings;
	settings.setValue(Hole::AutorouteViaHoleSize, m_holeSettings.holeDiameter);
	settings.setValue(Hole::AutorouteViaRingThickness, m_holeSettings.ringThickness);
	settings.setValue(AutorouteTraceWidth, m_traceWidth);
	
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
	m_customGroupBox->setVisible(enable);
}

bool AutorouterSettingsDialog::initRadios() 
{
	bool custom = true;
	foreach (QString name, Hole::HoleSizes.keys()) {
		QStringList values = Hole::HoleSizes.value(name).split(",");
		QString ringThickness = values[1];
		QString holeSize = values[0];
		if (!name.isEmpty() && !ringThickness.isEmpty() && !holeSize.isEmpty()) {
			QRadioButton * button = NULL;
			if (name.contains("homebrew", Qt::CaseInsensitive)) button = m_homebrewButton;
			else if (name.contains("professional", Qt::CaseInsensitive)) button = m_professionalButton;
			if (button) {
				button->setProperty("ringthickness", ringThickness);
				button->setProperty("holesize", holeSize);
				if (ringThickness.compare(m_holeSettings.ringThickness, Qt::CaseInsensitive) == 0 && holeSize.compare(m_holeSettings.holeDiameter, Qt::CaseInsensitive) == 0) {
					button->setChecked(true);
					custom = false;
				}
			}
		}
	}	

	m_customButton->setChecked(custom);

	return custom;
}

void AutorouterSettingsDialog::widthEntry(const QString & text) {
	int w = TraceWire::widthEntry(text, sender());
	if (w == 0) return;

	m_traceWidth = w;
}

void AutorouterSettingsDialog::changeHoleSize(const QString & newSize) {
	QString s = newSize;
	Hole::setHoleSize(s, false, m_holeSettings);
}

void AutorouterSettingsDialog::changeUnits(const QString & units) 
{
	QString newVal = Hole::changeUnits(units, m_holeSettings);
}

void AutorouterSettingsDialog::changeDiameter() 
{
	if (Hole::changeDiameter(m_holeSettings, sender())) {
		QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
		changeHoleSize(edit->text() + m_holeSettings.unitsComboBox->currentText() + "," + m_holeSettings.ringThickness);
	}
}

void AutorouterSettingsDialog::changeThickness() 
{
	if (Hole::changeThickness(m_holeSettings, sender())) {
		QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
		changeHoleSize(m_holeSettings.holeDiameter + "," + edit->text() + m_holeSettings.unitsComboBox->currentText());
	}	
}
