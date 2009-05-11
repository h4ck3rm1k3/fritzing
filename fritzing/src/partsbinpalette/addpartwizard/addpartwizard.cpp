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

#include "addpartwizard.h"
#include "addpartwizardpages.h"
#include "../../modelpart.h"
#include "../../mainwindow.h"


AddPartWizard::AddPartWizard(MainWindow *mainWindow, QWidget *parent) : QWizard(parent) {
	m_mainWindow = mainWindow;

	m_sourceOptionsPage = new SourceOptionsPage(this);
	addPage(m_sourceOptionsPage);

	m_partsEditorPage = NULL;
	m_fileBrowserPage = NULL;

	setSizeGripEnabled(true);
	//setWizardStyle(ModernStyle);
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

QAbstractButton *AddPartWizard::finishButton() {
	return button(FinishButton);
}


void AddPartWizard::fromPartsEditor() {
	removePage(1);
	if(!m_partsEditorPage) {
		m_partsEditorPage = new PartsEditorPage(this, m_mainWindow->getPartsEditor(NULL,this,false));
	}
	addPage(m_partsEditorPage);
	next();
}

void AddPartWizard::fromAllTheLibrary() {

}

void AddPartWizard::fromWebGenerator() {

}

void AddPartWizard::fromLocalFolder() {
	removePage(1);

	if(m_fileBrowserPage) {
		m_fileBrowserPage->setParent(NULL);
		m_fileBrowserPage->hide();
		//delete m_fileBrowserPage;
	}

	m_fileBrowserPage = new FileBrowserPage(this);

	addPage(m_fileBrowserPage);
	next();
}

void AddPartWizard::loadPart(QString newPartPath) {
	m_modelParts.clear();
	ModelPart *mp = m_mainWindow->loadPartFromFile(newPartPath);
	if(mp) {
		m_modelParts << mp;
	}
}

void AddPartWizard::loadBundledPart(QString newPartPath) {
	m_modelParts.clear();
	ModelPart *mp = m_mainWindow->loadBundledPart(newPartPath,false);
	if(mp) {
		m_modelParts << mp;
	}
}
