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

#ifndef ADDPARTWIZARD_H_
#define ADDPARTWIZARD_H_

#include <QWizard>
#include "addpartwizardpages.h"

class MainWindow;

class AddPartWizard : public QWizard {
	Q_OBJECT
	public:
		AddPartWizard(MainWindow *mainWindow, QWidget *parent=0);
		virtual ~AddPartWizard();

	public slots:
		void fromPartsEditor();
		void fromAllTheLibrary();
		void fromWebGenerator();
		void fromLocalFolder();

		void loadPart(QString newPartPath);

	public:
		static QList<class ModelPart*> getModelParts(MainWindow *mainWindow, QWidget *parent);

	protected:
		QList<ModelPart*> modelParts();
		QAbstractButton *finishButton();

		QList<ModelPart*> m_modelParts;
		MainWindow *m_mainWindow;


		SourceOptionsPage *m_sourceOptionsPage;

		PartsEditorPage *m_partsEditorPage;
		FileBrowserPage *m_fileBrowserPage;
};

#endif /* ADDPARTWIZARD_H_ */
