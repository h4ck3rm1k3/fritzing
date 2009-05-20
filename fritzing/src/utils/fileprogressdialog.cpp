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

#include "fileprogressdialog.h"
#include "../debugdialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLocale>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QCloseEvent>

/////////////////////////////////////

FileProgressDialog::FileProgressDialog(QWidget *parent) : QDialog(parent) 
{
	Qt::WindowFlags flags = windowFlags();
	flags ^= Qt::WindowCloseButtonHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	this->setWindowTitle(QObject::tr("File Progress..."));

	QVBoxLayout * vLayout = new QVBoxLayout(this);

	m_message = new QLabel(this);
	m_message->setMinimumWidth(300);
	vLayout->addWidget(m_message);

	m_progressBar = new QProgressBar(this);
	vLayout->addWidget(m_progressBar);

    //QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
	//buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	//buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
    //connect(buttonBox, SIGNAL(rejected()), this, SLOT(sendCancel()));
	//vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);
}

FileProgressDialog::~FileProgressDialog() {
}

void FileProgressDialog::setMinimum(int minimum) {
	m_progressBar->setMinimum(minimum);
}

void FileProgressDialog::setMaximum(int maximum) {
	m_progressBar->setMaximum(maximum);
}

void FileProgressDialog::setValue(int value) {
	m_progressBar->setValue(value);
}

void FileProgressDialog::sendCancel() {
	emit cancel();
}

void FileProgressDialog::closeEvent(QCloseEvent *event)
{
	event->ignore();
}

void FileProgressDialog::setMessage(const QString & message) {
	m_message->setText(message);
}


