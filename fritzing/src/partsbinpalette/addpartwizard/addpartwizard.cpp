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

#include "addpartwizard.h"
#include "../../modelpart.h"
#include "../../mainwindow.h"


SourceOptionsPage::SourceOptionsPage(AddPartWizard *parent) : QWizardPage() {
	m_parent = parent;

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(1);
	layout->setMargin(1);

	addButton( tr("Create a new part"), SLOT(fromPartsEditor()) );
	addButton( tr("Browse all the existing parts"), SLOT(fromAllTheLibrary()) );
	addButton( tr("Generate new part"), SLOT(fromWebGenerator()) );
	addButton( tr("Import part from local folder"), SLOT(fromLocalFolder()) );

	layout->addSpacing(3);
}

void SourceOptionsPage::initializePage() {
	QList<QWizard::WizardButton> btnlayout;
	btnlayout << QWizard::Stretch << QWizard::BackButton << QWizard::CancelButton;
	m_parent->setButtonLayout(btnlayout);
}

void SourceOptionsPage::addButton(const QString &btnText, const char *method) {
	QPushButton *btn = new QPushButton(btnText, this);
	connect(btn, SIGNAL(clicked()),	m_parent, method);
	layout()->addWidget(btn);
}

//////////////////////////////////////////////////

PageSourcePage::PageSourcePage(AddPartWizard* parent) : QWizardPage() {
	m_parent = parent;
	m_centralWidget = NULL;
	new QVBoxLayout(this);
	layout()->setMargin(0);
	layout()->setSpacing(0);
}

void PageSourcePage::setCentralWidget(QWidget *widget) {
	if(m_centralWidget) {
		layout()->removeWidget(m_centralWidget);
		delete m_centralWidget;
	}
	m_centralWidget = widget;
	layout()->addWidget(m_centralWidget);
}

void PageSourcePage::initializePage() {
	QList<QWizard::WizardButton> btnlayout;
	btnlayout << QWizard::Stretch << QWizard::BackButton
			  << QWizard::FinishButton << QWizard::CancelButton;
	m_parent->setButtonLayout(btnlayout);
}

//////////////////////////////////////////////////

AddPartWizard::AddPartWizard(MainWindow *mainWindow, QWidget *parent) : QWizard(parent) {
	m_mainWindow = mainWindow;

	m_sourceOptionsPage = new SourceOptionsPage(this);
	addPage(m_sourceOptionsPage);

	m_partSourcePage = new PageSourcePage(this);
	addPage(m_partSourcePage);
}

AddPartWizard::~AddPartWizard() {

}


QList<ModelPart*> AddPartWizard::getModelParts(MainWindow *mainWindow, QWidget *parent) {
	AddPartWizard dialog(mainWindow, parent);
	int result = dialog.exec();
	if(result == QDialog::Accepted) {
		return dialog.modelParts();
	} else {
		return QList<ModelPart*>();
	}
}

QList<ModelPart*> AddPartWizard::modelParts() {
	return m_modelParts;
}


void AddPartWizard::fromPartsEditor() {
	m_partSourcePage->setCentralWidget(
		m_mainWindow->getPartsEditor(NULL, this, false)
	);
	next();
}

void AddPartWizard::fromAllTheLibrary() {

}

void AddPartWizard::fromWebGenerator() {

}

void AddPartWizard::fromLocalFolder() {

}
