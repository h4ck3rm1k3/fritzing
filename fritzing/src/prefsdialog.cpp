/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 1923 $:
$Author: cohen@irascible.com $:
$Date: 2008-12-20 03:07:49 +0100 (Sat, 20 Dec 2008) $

********************************************************************/

#include "prefsdialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

TranslatorListModel::TranslatorListModel(QFileInfoList &, QObject* parent)
: QAbstractListModel(parent) 
{
}

QVariant TranslatorListModel::data ( const QModelIndex & index, int role) const 
{
	return "";
	
}

int TranslatorListModel::rowCount ( const QModelIndex & parent) const 
{
	return 1;
}

/////////////////////////////////////

PrefsDialog::PrefsDialog(QFileInfoList & list, QWidget *parent)
	: QDialog(parent)
{
	this->setWindowTitle(QObject::tr("Preferences"));
	
	QGridLayout * gridLayout = new QGridLayout(this);
	
	QLabel * languageLabel = new QLabel(this);
	languageLabel->setText(QObject::tr("Choose your preferred language"));
	gridLayout->addWidget(languageLabel, 0, 0);
	
	gridLayout->setColumnMinimumWidth(1, 10);
	
	QComboBox* comboBox = new QComboBox(this);
	TranslatorListModel * tlm = new TranslatorListModel(list);
	comboBox->setModel(tlm);
	gridLayout->addWidget(comboBox, 0, 2);	
	
	gridLayout->setRowMinimumHeight(1, 20);
	
	QLabel * textLabel = new QLabel(this);
	textLabel->setText(QObject::tr("This will soon provide the ability to set some preferences. "
							  "such as your default sketch folder, your fritzing.org login name, etc.\n"
							  "Please stay tuned."));	
	gridLayout->addWidget(textLabel, 2, 0, 1, 3);

	gridLayout->setRowMinimumHeight(3, 20);

	QPushButton * ok = new QPushButton(QObject::tr("OK"), this);
	gridLayout->addWidget(ok, 4, 0);	
	connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
	
	QPushButton * cancel = new QPushButton(QObject::tr("Cancel"), this);
	gridLayout->addWidget(cancel, 4, 3);
	connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	
}

PrefsDialog::~PrefsDialog()
{
}


