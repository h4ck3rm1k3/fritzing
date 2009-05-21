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

$Revision: 2597 $:
$Author: cohen@irascible.com $:
$Date: 2009-03-10 12:44:55 +0100 (Tue, 10 Mar 2009) $

********************************************************************/

#include "autorouteprogressdialog.h"
#include "../debugdialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLocale>
#include <QDialogButtonBox>
#include <QGroupBox>

/////////////////////////////////////

AutorouteProgressDialog::AutorouteProgressDialog(QWidget *parent) : QDialog(parent) 
{
	Qt::WindowFlags flags = windowFlags();
	flags ^= Qt::WindowCloseButtonHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	this->setWindowTitle(QObject::tr("Autorouting Progress..."));

	QVBoxLayout * vLayout = new QVBoxLayout(this);

	m_progressBar = new QProgressBar(this);
	vLayout->addWidget(m_progressBar);

	QPushButton * button = new QPushButton(tr("Skip current trace"), this);
	connect(button, SIGNAL(clicked()), this, SLOT(sendSkip()));
	vLayout->addWidget(button);

	// TODO: eventually allow zooming and panning during dialog

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Stop Now"));

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(sendCancel()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(sendStop()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);
}

AutorouteProgressDialog::~AutorouteProgressDialog() {
}

void AutorouteProgressDialog::setMinimum(int minimum) {
	m_progressBar->setMinimum(minimum);
}

void AutorouteProgressDialog::setMaximum(int maximum) {
	m_progressBar->setMaximum(maximum);
}

void AutorouteProgressDialog::setValue(int value) {
	m_progressBar->setValue(value);
}

void AutorouteProgressDialog::sendSkip() {
	emit skip();
}

void AutorouteProgressDialog::sendCancel() {
	emit cancel();
}

void AutorouteProgressDialog::sendStop() {
	emit stop();
}

void AutorouteProgressDialog::closeEvent(QCloseEvent *event)
{
	sendCancel();
	QDialog::closeEvent(event);
}

void AutorouteProgressDialog::wheelEvent(QWheelEvent *event)
{
	// pass it to the sketch?
	DebugDialog::debug("got wheel event");
	QDialog::wheelEvent(event);
}

