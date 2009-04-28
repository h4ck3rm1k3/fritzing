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

class AddPartWizard;

class SourceOptionsPage : public QWizardPage {
	public:
		SourceOptionsPage(AddPartWizard*);

	protected:
		void addButton(const QString &btnText, const char *method);
		void initializePage();

	protected:
		AddPartWizard* m_parent;

};

//////////////////////////////////////////////////

class PageSourcePage : public QWizardPage {
	public:
		PageSourcePage(AddPartWizard*);
		void setCentralWidget(QWidget *widget);

	protected:
		void initializePage();

	protected:
		AddPartWizard* m_parent;
		QWidget* m_centralWidget;
};

//////////////////////////////////////////////////

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

	public:
		static QList<class ModelPart*> getModelParts(MainWindow *mainWindow, QWidget *parent);

	protected:
		QList<ModelPart*> modelParts();

		QList<ModelPart*> m_modelParts;
		MainWindow *m_mainWindow;

		SourceOptionsPage *m_sourceOptionsPage;
		PageSourcePage *m_partSourcePage;
};

#endif /* ADDPARTWIZARD_H_ */
