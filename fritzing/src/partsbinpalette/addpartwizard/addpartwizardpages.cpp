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

#include <QFileDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "addpartwizardpages.h"
#include "addpartwizard.h"
#include "../../partseditor/partseditormainwindow.h"
#include "../../debugdialog.h"

AbstractAddPartWizardPage::AbstractAddPartWizardPage(AddPartWizard* parent)
	: QWizardPage()
{
	m_parent = parent;
	m_layout = new QVBoxLayout(this);
}

//////////////////////////////////////////////////
AbstractAddPartSourceWizardPage::AbstractAddPartSourceWizardPage(AddPartWizard* parent)
	: AbstractAddPartWizardPage(parent)
{
	setFinalPage(true);
	m_centralWidget = NULL;
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
}

void AbstractAddPartSourceWizardPage::initializePage() {
	QList<QWizard::WizardButton> btnlayout;
	btnlayout << QWizard::Stretch << QWizard::BackButton
			  << QWizard::FinishButton << QWizard::CancelButton;
	m_parent->setButtonLayout(btnlayout);
	m_layout->addWidget(m_centralWidget);
	m_parent->resize(m_centralWidget->size());
}

//////////////////////////////////////////////////

SourceOptionsPage::SourceOptionsPage(AddPartWizard *parent) : AbstractAddPartWizardPage(parent) {
	m_layout->setSpacing(1);
	m_layout->setMargin(1);

	addButton( QObject::tr("Create a new part"), SLOT(fromPartsEditor()) );
	//addButton( QObject::tr("Browse all the existing parts"), SLOT(fromAllTheLibrary()) );
	//addButton( QObject::tr("Generate new part"), SLOT(fromWebGenerator()) );
	addButton( QObject::tr("Import part from local folder"), SLOT(fromLocalFolder()) );

	m_layout->addSpacing(3);
}

void SourceOptionsPage::initializePage() {
	QList<QWizard::WizardButton> btnlayout;
	btnlayout << QWizard::Stretch << QWizard::BackButton << QWizard::CancelButton;
	m_parent->setButtonLayout(btnlayout);
	m_parent->resize(200,100);
}

void SourceOptionsPage::addButton(const QString &btnText, const char *method) {
	QPushButton *btn = new QPushButton(btnText, this);
	connect(btn, SIGNAL(clicked()),	m_parent, method);
	layout()->addWidget(btn);
}

//////////////////////////////////////////////////

PartsEditorPage::PartsEditorPage(AddPartWizard* parent, PartsEditorMainWindow * partsEditor)
	: AbstractAddPartSourceWizardPage(parent)
{
	m_partsEditor = partsEditor;
	connect(
		m_parent, SIGNAL(accepted()),
		this, SLOT(setModelPart())
	);
}


void PartsEditorPage::initializePage() {
	if(!m_centralWidget) {
		m_centralWidget = m_partsEditor->centralWidget();
	}
	AbstractAddPartSourceWizardPage::initializePage();
}

bool PartsEditorPage::validatePage() {
	return m_partsEditor->validateMinRequirements();
}

void PartsEditorPage::setModelPart() {
	m_partsEditor->save();
	m_parent->loadPart(m_partsEditor->fileName());
}

//////////////////////////////////////////////////

FileBrowserPage::FileBrowserPage(AddPartWizard* parent)
	: AbstractAddPartSourceWizardPage(parent)
{
	m_fileDialog = NULL;
}

FileBrowserPage::~FileBrowserPage() {
	if(m_fileDialog) {
		m_fileDialog->setLayout(m_centralWidget->layout());
	}
}


void FileBrowserPage::initializePage() {
	if(!m_fileDialog) {
		m_fileDialog = new QFileDialog(
			this,
			QObject::tr("Select a part to import"),
			"",
			QObject::tr("External Part (*%1)").arg(FritzingBundledPartExtension)
		);

		connect(
			m_parent, SIGNAL(accepted()),
			this, SLOT(setModelPart())
		);

		removeButtonsFrom(m_fileDialog);
		m_centralWidget = new QWidget(this);
		m_centralWidget->setLayout(m_fileDialog->layout());
	}
	AbstractAddPartSourceWizardPage::initializePage();
}

#define FILE_NEEDED_MSG QMessageBox::information(this, tr("File needed"), tr("Please, select a file to import"));

bool FileBrowserPage::validatePage() {
	QStringList selFiles = m_fileDialog->selectedFiles();
	if(selFiles.isEmpty()) {
		FILE_NEEDED_MSG
		return false;
	} else {
		if(selFiles.count() == 1) {
			if(m_fileDialog->directory().path() != selFiles[0]) {
				return true;
			} else {
				FILE_NEEDED_MSG
				return false;
			}
		} else {
			QMessageBox::information(this, tr("File needed"), tr("Please, select just one file to import"));
			return false;
		}
	}

}

void FileBrowserPage::removeButtonsFrom(QFileDialog* dlg) {
	QDialogButtonBox* toHide = NULL;
	foreach(QObject *w, dlg->children()) {
		QDialogButtonBox *bb = dynamic_cast<QDialogButtonBox*>(w);
		if(bb) {
			toHide = bb;
			break;
		}
	}
	if(toHide) {
		toHide->hide();
	}
}

void FileBrowserPage::setModelPart() {
	m_parent->loadBundledPart(m_fileDialog->selectedFiles()[0]);
}

//////////////////////////////////////////////////
