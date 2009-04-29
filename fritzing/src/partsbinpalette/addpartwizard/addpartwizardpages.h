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


#ifndef ADDPARTWIZARDPAGES_H_
#define ADDPARTWIZARDPAGES_H_

#include <QWizardPage>

class AddPartWizard;

//////////////////////////////////////////////////

class AbstractAddPartWizardPage : public QWizardPage {
	public:
		AbstractAddPartWizardPage(AddPartWizard*);

	protected:
		AddPartWizard* m_parent;
		class QVBoxLayout *m_layout;
};

//////////////////////////////////////////////////

class AbstractAddPartSourceWizardPage : public AbstractAddPartWizardPage {
	public:
		AbstractAddPartSourceWizardPage(AddPartWizard*);

	protected:
		void initializePage();

		QWidget* m_centralWidget;
};

//////////////////////////////////////////////////

class SourceOptionsPage : public AbstractAddPartWizardPage {
	public:
		SourceOptionsPage(AddPartWizard*);

	protected:
		void addButton(const QString &btnText, const char *method);
		void initializePage();
};

//////////////////////////////////////////////////

class PartsEditorPage : public AbstractAddPartSourceWizardPage {
	Q_OBJECT
	public:
		PartsEditorPage(AddPartWizard*, class PartsEditorMainWindow *);

	protected slots:
		void setModelPart();

	protected:
		void initializePage();
		bool validatePage();

		PartsEditorMainWindow *m_partsEditor;
};

//////////////////////////////////////////////////

class FileBrowserPage : public AbstractAddPartSourceWizardPage {
	public:
		FileBrowserPage(AddPartWizard*);
		~FileBrowserPage();

	protected:
		void initializePage();
		void removeButtonsFrom(class QFileDialog* dlg);

		QFileDialog *m_fileDialog;
};

//////////////////////////////////////////////////

#endif /* ADDPARTWIZARDPAGES_H_ */
