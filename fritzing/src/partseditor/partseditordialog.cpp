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


#include "partseditordialog.h"
#include "partseditormainwindow.h"
#include "../mainwindow.h"
#include "../modelpart.h"

PartsEditorDialog::PartsEditorDialog(MainWindow *parent, ModelPart* modelPart)
	: QDialog(parent)
{
	m_modelPart = NULL;
	m_mainWindow = parent;
	m_partsEditorWindow = m_mainWindow->getPartsEditor(modelPart,this,true);
	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->setMargin(0);
	lo->setSpacing(0);
	lo->addWidget(m_partsEditorWindow->centralWidget());
	resize(m_partsEditorWindow->size());
	setMinimumSize(m_partsEditorWindow->minimumSize());
}

PartsEditorDialog::~PartsEditorDialog() {

}

void PartsEditorDialog::loadPart(const QString &newPartPath) {
	ModelPart *mp = m_mainWindow->loadPartFromFile(newPartPath);
	if(mp) {
		setWindowTitle(mp->title());
		m_modelPart = mp;
	}
	accept();
}

void PartsEditorDialog::partsEditorClosed(long id) {
	m_mainWindow->partsEditorClosed(id);
	accept();
}

void PartsEditorDialog::reject() {
	m_modelPart = NULL;
	QDialog::reject();
}

ModelPart *PartsEditorDialog::modelPart() {
	return m_modelPart;
}

void PartsEditorDialog::closeEvent(QCloseEvent *event) {
	m_partsEditorWindow->close();
	QDialog::closeEvent(event);
	accept();
}


ModelPart *PartsEditorDialog::newPart(MainWindow *parent) {
	return editPart(parent, NULL);
}

ModelPart *PartsEditorDialog::editPart(MainWindow *parent, ModelPart* mp) {
	PartsEditorDialog *dlg = new PartsEditorDialog(parent, mp);
	dlg->exec();
	ModelPart *retval = dlg->modelPart();
	delete dlg;
	return retval;
}
