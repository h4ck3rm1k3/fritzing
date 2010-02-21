/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "alignsettingsdialog.h"

#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSettings>

AlignSettingsDialog::AlignSettingsDialog(const QString & viewName, qreal defaultSize, QWidget *parent) : QDialog(parent) 
{
	m_defaultSize = defaultSize;
	m_viewName = viewName;
	this->setWindowTitle(QObject::tr("Grid Size"));

	QVBoxLayout * vLayout = new QVBoxLayout();

	QLabel * explain = new QLabel(tr("Set the grid size for %1 only.").arg(viewName));
	vLayout->addWidget(explain);

	QGroupBox * groupBox = new QGroupBox(this);

	QHBoxLayout * hLayout = new QHBoxLayout();

	QLabel * label = new QLabel(tr("Grid Size:"));
	hLayout->addWidget(label);

	m_lineEdit = new QLineEdit();
	m_lineEdit->setFixedWidth(45);

	m_validator = new QDoubleValidator(m_lineEdit);
	m_validator->setRange(0.001, 1.0, 3);
	m_validator->setNotation(QDoubleValidator::StandardNotation);
	m_lineEdit->setValidator(m_validator);

	hLayout->addWidget(m_lineEdit);

	m_inButton = new QRadioButton(tr("in"), this); 
	connect(m_inButton, SIGNAL(clicked(bool)), this, SLOT(units(bool)));
	hLayout->addWidget(m_inButton);

	m_mmButton = new QRadioButton(tr("mm"), this); 
	connect(m_mmButton, SIGNAL(clicked(bool)), this, SLOT(units(bool)));
	hLayout->addWidget(m_mmButton);

	groupBox->setLayout(hLayout);

	vLayout->addWidget(groupBox);
	vLayout->addSpacing(5);

	QPushButton * pushButton = new QPushButton(this);
	pushButton->setText(tr("Restore Default"));
	pushButton->setMaximumWidth(115);
	connect(pushButton, SIGNAL(clicked()), this, SLOT(restoreDefault()));
	vLayout->addWidget(pushButton);
	vLayout->addSpacing(10);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptAnd()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);

	QSettings settings;
	QString szString = settings.value(QString("%1GridSize").arg(m_viewName), "").toString();
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

}

AlignSettingsDialog::~AlignSettingsDialog() {
}

void AlignSettingsDialog::units(bool checked) {
	QString units;
	if (sender() == m_inButton) {
		units = (checked) ? "in" : "mm";
	}
	else {
		units = (checked) ? "mm" : "in";
	}
	
	if (units.startsWith("mm")) {
		m_validator->setTop(25.4);
		m_lineEdit->setText(QString::number(m_lineEdit->text().toDouble() * 25.4));
	}
	else {
		m_validator->setTop(1.0);
		m_lineEdit->setText(QString::number(m_lineEdit->text().toDouble() / 25.4));
	}
}

void AlignSettingsDialog::acceptAnd() {
	QString units = (m_inButton->isChecked() ? "in" : "mm");
	QSettings settings;
	settings.setValue(QString("%1GridSize").arg(m_viewName), m_lineEdit->text() + units);
	accept();
}

void AlignSettingsDialog::restoreDefault() {
	m_inButton->setChecked(true);
	m_mmButton->setChecked(false);
	m_lineEdit->setText(QString::number(m_defaultSize));
}
