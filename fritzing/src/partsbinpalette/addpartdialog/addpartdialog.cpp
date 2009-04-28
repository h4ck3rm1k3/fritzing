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

$Revision: 2776 $:
$Author: merunga $:
$Date: 2009-04-02 13:54:08 +0200 (Thu, 02 Apr 2009) $

********************************************************************/

#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include "addpartdialog.h"
#include "../../modelpart.h"

AddPartDialog::AddPartDialog(QWidget *parent) : QDialog(parent) {
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(1);
	layout->setMargin(1);

	addButton(
		tr("Create a new part"),
		SLOT(fromPartsEditor())
	);
	addButton(
		tr("Browse all the existing parts"),
		SLOT(fromAllTheLibrary())
	);
	addButton(
		tr("Generate new part"),
		SLOT(fromWebGenerator())
	);
	addButton(
		tr("Import part from local folder"),
		SLOT(fromLocalFolder())
	);

	layout->addSpacing(3);

	QDialogButtonBox *dlgBtnBox = new QDialogButtonBox(this);
	QPushButton *closeBtn = dlgBtnBox->addButton(QDialogButtonBox::Cancel);
	connect(closeBtn, SIGNAL(clicked()), this, SLOT(reject()));

	layout->addWidget(dlgBtnBox);
}

AddPartDialog::~AddPartDialog() {

}

void AddPartDialog::addButton(const QString &btnText, const char *method) {
	QPushButton *btn = new QPushButton(btnText,this);
	connect(btn, SIGNAL(clicked()),	this, method);
	layout()->addWidget(btn);
}

QList<ModelPart*> AddPartDialog::getModelParts(QWidget *parent) {
	AddPartDialog dialog(parent);
	int result = dialog.exec();
	if(result == QDialog::Accepted) {
		return dialog.modelParts();
	} else {
		return QList<ModelPart*>();
	}
}

QList<ModelPart*> AddPartDialog::modelParts() {
	return m_modelParts;
}


void AddPartDialog::fromPartsEditor() {

}

void AddPartDialog::fromAllTheLibrary() {

}

void AddPartDialog::fromWebGenerator() {

}

void AddPartDialog::fromLocalFolder() {

}
