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

$Revision$:
$Author$:
$Date$

********************************************************************/


#include <QUndoStack>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

#include "partsbinpalettewidget.h"
#include "partsbincommands.h"
#include "../fritzingwindow.h"
#include "../mainwindow.h"
#include "../misc.h"
#include "../debugdialog.h"

#define TITLE tr("Parts")

PartsBinPaletteWidget::PartsBinPaletteWidget(ReferenceModel *refModel, HtmlInfoView *infoView, WaitPushUndoStack *undoStack, QWidget* parent) :
	FDockWidget(TITLE,parent)
{
	m_refModel = refModel;

	m_undoStack = undoStack;
	connect(m_undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(undoStackCleanChanged(bool)) );

	m_iconView = new PartsBinIconView(this);
	m_iconView->setInfoView(infoView);

	m_listView = new PartsBinListView(this);
	m_listView->setInfoView(infoView);

	m_binTitle = new SimpleEditableLabelWidget(m_undoStack,this);

	m_container = new QFrame(this);
	m_container->setObjectName("partsBinContainer");

	setupPixmaps();
	setupButtons();
	setupFooter();

	toIconView();

	setWidget(m_container);

	m_defaultSaveFolder = getApplicationSubFolderPath("bins");
	m_untitledFileName = "Untitled Bin";

	connect(m_listView, SIGNAL(currentRowChanged(int)), m_iconView, SLOT(setSelected(int)));
	connect(m_iconView, SIGNAL(selectionChanged(int)), m_listView, SLOT(setSelected(int)));
}

PartsBinPaletteWidget::~PartsBinPaletteWidget() {
	delete m_iconViewActive;
	delete m_iconViewInactive;
	delete m_listViewActive;
	delete m_listViewInactive;
	delete m_saveButtonEnabled;
	delete m_saveButtonDisabled;
}

QSize PartsBinPaletteWidget::sizeHint() const {
	return QSize(MainWindow::DockDefaultWidth, MainWindow::PartsBinDefaultHeight);
}

void PartsBinPaletteWidget::setupFooter() {
	m_footer = new QFrame(this);
	m_footer->setObjectName("partsBinFooter");

	QFrame *leftButtons = new QFrame(m_footer);
	QHBoxLayout *leftLayout = new QHBoxLayout(leftButtons);
	leftLayout->setMargin(0);
	leftLayout->setSpacing(3);
	leftLayout->addWidget(m_showIconViewButton);
	leftLayout->addWidget(m_showListViewButton);

	QFrame *rightButtons = new QFrame(m_footer);
	QHBoxLayout *rightLayout = new QHBoxLayout(rightButtons);
	rightLayout->setDirection(QBoxLayout::RightToLeft);
	rightLayout->setMargin(0);
	rightLayout->setSpacing(3);
	rightLayout->addWidget(m_removeSelected);
	rightLayout->addWidget(m_coreBinButton);
	rightLayout->addWidget(m_saveBinButton);
	rightLayout->addWidget(m_openBinButton);

	QHBoxLayout *footerLayout = new QHBoxLayout(m_footer);
	footerLayout->setMargin(2);
	footerLayout->setSpacing(0);
	footerLayout->addWidget(leftButtons);
	footerLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	footerLayout->addWidget(rightButtons);
}

void PartsBinPaletteWidget::setView(PartsBinView *view, QPixmap *showIconPixmap, QPixmap *showListPixmap) {
	m_currentView = view;
	if(m_currentView == m_iconView) {
		m_iconView->show();
		m_listView->hide();
	} else if(m_currentView == m_listView) {
		m_listView->show();
		m_iconView->hide();
	}

	delete m_container->layout();
	QVBoxLayout *lo = new QVBoxLayout(m_container);
	lo->setMargin(3);
	lo->setSpacing(0);
	lo->addWidget(m_binTitle);
	lo->addWidget(dynamic_cast<QWidget*>(m_currentView));
	lo->addWidget(m_footer);

	m_showIconViewButton->setPixmap(*showIconPixmap);
	m_showListViewButton->setPixmap(*showListPixmap);
}

void PartsBinPaletteWidget::toIconView() {
	setView(m_iconView, m_iconViewActive, m_listViewInactive);
}

void PartsBinPaletteWidget::toListView() {
	disconnect(m_listView, SIGNAL(currentRowChanged(int)), m_iconView, SLOT(setSelected(int)));
	setView(m_listView, m_iconViewInactive, m_listViewActive);
	connect(m_listView, SIGNAL(currentRowChanged(int)), m_iconView, SLOT(setSelected(int)));
}

void PartsBinPaletteWidget::saveAsAux(const QString &filename) {
	m_fileName = filename;
	QString title = m_binTitle->text();
	if(!title.isNull() && !title.isEmpty()) {
		m_model->root()->modelPartStuff()->setTitle(title);
	}
	m_model->save(filename);
	m_undoStack->setClean();

	saveAsLastBin();
	emit saved(hasAlienParts());
}

void PartsBinPaletteWidget::loadFromModel(PaletteModel *model) {
	m_iconView->loadFromModel(model);
	m_listView->setPaletteModel(model);
	afterModelSetted(model);
}

void PartsBinPaletteWidget::setPaletteModel(PaletteModel *model, bool clear) {
	m_iconView->setPaletteModel(model, clear);
	m_listView->setPaletteModel(model, clear);
	afterModelSetted(model);
}

void PartsBinPaletteWidget::afterModelSetted(PaletteModel *model) {
	grabTitle(model);
	m_model = model;
	m_undoStack->setClean();
	m_fileName = model->loadedFrom();
	if(currentBinIsCore()) {
		setSaveButtonEnabled(false);
	}
}

void PartsBinPaletteWidget::grabTitle(PaletteModel *model) {
	m_binTitle->setText(model->root()->modelPartStuff()->title(), false);
}

void PartsBinPaletteWidget::addPart(ModelPart *modelPart, int position) {
	ModelPart *mp = m_model->addModelPart(m_model->root(),modelPart);

	m_iconView->addPart(mp, position);
	m_listView->addPart(mp, position);

	if(modelPart->isAlien()) {
		m_alienParts << mp->moduleID();
	}
}

void PartsBinPaletteWidget::setSaveButtonEnabled(bool enabled) {
	m_saveBinButton->setEnabled(enabled);
	if(!enabled) {
		m_saveBinButton->setPixmap(*m_saveButtonDisabled);
	} else {
		m_saveBinButton->setPixmap(*m_saveButtonEnabled);
	}
}

void PartsBinPaletteWidget::setupButtons() {
	m_showIconViewButton = new ImageButton(this);
	m_showIconViewButton->setToolTip(tr("Show as icons"));
	connect(m_showIconViewButton,SIGNAL(clicked()),this,SLOT(toIconView()));

	m_showListViewButton = new ImageButton(this);
	m_showListViewButton->setToolTip(tr("Show as list"));
	connect(m_showListViewButton,SIGNAL(clicked()),this,SLOT(toListView()));

	m_removeSelected = new ImageButton(this);
	m_removeSelected->setPixmap(QPixmap(":/resources/images/icons/partsBinDelete_icon.png"));
	m_removeSelected->setToolTip(tr("Remove selected part"));
	connect(m_removeSelected,SIGNAL(clicked()),this,SLOT(removeSelected()));

	m_openBinButton = new ImageButton(this);
	m_openBinButton->setPixmap(QPixmap(":/resources/images/icons/partsBinOpen_icon.png"));
	m_openBinButton->setToolTip(tr("Open bin"));
	connect(m_openBinButton,SIGNAL(clicked()),this,SLOT(open()));

	m_saveBinButton = new ImageButton(this);
	m_saveBinButton->setPixmap(*m_saveButtonEnabled);
	m_saveBinButton->setToolTip(tr("Save bin"));
	connect(m_saveBinButton,SIGNAL(clicked()),this,SLOT(save()));

	m_coreBinButton = new ImageButton(this);
	m_coreBinButton->setPixmap(QPixmap(":/resources/images/icons/partsBinCore_icon.png"));
	m_coreBinButton->setToolTip(tr("Restore core bin"));
	connect(m_coreBinButton,SIGNAL(clicked()),this,SLOT(openCore()));
}

void PartsBinPaletteWidget::setupPixmaps() {
	m_iconViewActive = new QPixmap(":/resources/images/icons/partsBinIconViewActive_icon.png");
	m_iconViewInactive = new QPixmap(":/resources/images/icons/partsBinIconViewInactive_icon.png");
	m_listViewActive = new QPixmap(":/resources/images/icons/partsBinListViewActive_icon.png");
	m_listViewInactive = new QPixmap(":/resources/images/icons/partsBinListViewInactive_icon.png");
	m_saveButtonEnabled = new QPixmap(":/resources/images/icons/partsBinSaveEnabled_icon.png");
	m_saveButtonDisabled = new QPixmap(":/resources/images/icons/partsBinSaveDisabled_icon.png");
}

bool PartsBinPaletteWidget::removeSelected() {
	if(selected()) {
		QString modId = selected()->moduleID();
		removePartCommand(modId);
		return true;
	} else return false;
}

bool PartsBinPaletteWidget::save() {
	if (FritzingWindow::isEmptyFileName(m_fileName,m_untitledFileName)) {
		return saveAs();
	} else {
		saveAsAux(m_fileName);
		return true;
	}
}

bool PartsBinPaletteWidget::saveAs() {
	QString fileExt;
    QString fileName = QFileDialog::getSaveFileName(
						this,
                        tr("Choose a file name"),
                        (m_fileName.isNull() || m_fileName.isEmpty()) ? m_defaultSaveFolder : m_fileName,
                        tr("Fritzing (*%1)").arg(FritzingWindow::FritzingExtension),
                        &fileExt
                      );

    if (fileName.isEmpty()) return false; // Cancel pressed

    if(!FritzingWindow::alreadyHasExtension(fileName)) {
		fileExt = FritzingWindow::getExtFromFileDialog(fileExt);
		fileName += fileExt;
	}
    saveAsAux(fileName);
    return true;
}

void PartsBinPaletteWidget::open() {
	QString fileName = QFileDialog::getOpenFileName(
			this,
			"Select a Fritzing file to open",
			m_defaultSaveFolder,
			tr("Fritzing (*%1)").arg(FritzingWindow::FritzingExtension) );
	if (fileName.isNull()) return;

	QFile file(fileName);
	if (!file.exists()) {
       QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot find file %1.")
                             .arg(fileName));
		return;
	}

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    file.close();

    load(fileName);
    setSaveButtonEnabled(true);
    saveAsLastBin();
}

void PartsBinPaletteWidget::openCore() {
	load(FritzingWindow::CoreBinLocation);
}

void PartsBinPaletteWidget::load(const QString &filename) {
	PaletteModel * paletteReferenceModel = new PaletteModel(true, true);
	PaletteModel * paletteBinModel = new PaletteModel(true, false);
	if (!paletteBinModel->load(filename, paletteReferenceModel, false)) {
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Friting cannot load the parts bin"));
		return;
	}
	setPaletteModel(paletteBinModel,true);
	delete paletteReferenceModel;
}

void PartsBinPaletteWidget::undoStackCleanChanged(bool isClean) {
	if(!isClean && currentBinIsCore()) {
		m_fileName = QString::null;
		setSaveButtonEnabled(true);
	}
	setWindowModified(!isClean);
	setWindowTitle(TITLE + (!isClean ? " (modified)" : ""));
}

bool PartsBinPaletteWidget::currentBinIsCore() {
	return m_fileName == FritzingWindow::CoreBinLocation;
}


bool PartsBinPaletteWidget::beforeClosing() {
	bool retval;
	if (this->isWindowModified()) {
		QMessageBox::StandardButton reply;
		QMessageBox *messageBox = new QMessageBox(
				tr("Save \"%1\"").arg(QFileInfo(m_fileName).baseName()),
				tr("Do you want to save the changes you made in the document \"%1\"?")
					.arg(QFileInfo(m_fileName).baseName()),
				QMessageBox::Warning,
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default,
				this, Qt::Sheet);

		messageBox->setButtonText(QMessageBox::Yes, tr("Save"));
		messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));
		messageBox->button(QMessageBox::No)->setShortcut(tr("Ctrl+D"));
		messageBox->setInformativeText("Your changes will be lost if you don't save them.");

		reply = (QMessageBox::StandardButton)messageBox->exec();

     	if (reply == QMessageBox::Yes) {
     		retval = save();
    	} else if (reply == QMessageBox::No) {
    		retval = true;
        }
     	else {
         	retval = false;
        }
	} else {
		retval = true;
	}

	saveAsLastBin();
	return retval;
}

void PartsBinPaletteWidget::saveAsLastBin() {
	QSettings settings("Fritzing","Fritzing");
	settings.setValue("lastBin",m_fileName);
}

void PartsBinPaletteWidget::closeEvent(QCloseEvent* event) {
	saveAsLastBin();
	FDockWidget::closeEvent(event);
}

ModelPart * PartsBinPaletteWidget::selected() {
	return m_currentView->selected();
}

bool PartsBinPaletteWidget::alreadyIn(QString moduleID) {
	return m_iconView->alreadyIn(moduleID);
}

bool PartsBinPaletteWidget::hasAlienParts() {
	return m_alienParts.size() > 0;
}

void PartsBinPaletteWidget::addPart(const QString& moduleID, int position) {
	ModelPart *modelPart = m_refModel->retrieveModelPart(moduleID);
	addPart(modelPart, position);
}

void PartsBinPaletteWidget::removePart(const QString& moduleID) {
	m_iconView->removePart(moduleID);
	m_listView->removePart(moduleID);

	// remove the model part from the model last, as this deletes it,
	// and the removePart calls above still need the modelpart
	m_model->removePart(moduleID);

}

void PartsBinPaletteWidget::removeAlienParts() {
	foreach(QString moduleID, m_alienParts) {
		removePart(moduleID);
	}
	m_alienParts.clear();
}

void PartsBinPaletteWidget::setInfoViewOnHover(bool infoViewOnHover) {
	if(m_iconView) m_iconView->setInfoViewOnHover(infoViewOnHover);
	if(m_listView) m_listView->setInfoViewOnHover(infoViewOnHover);
}

void PartsBinPaletteWidget::addPartCommand(const QString& moduleID) {
	bool updating = alreadyIn(moduleID);
	QString undoStackMsg;

	if(!updating) {
		undoStackMsg = tr("Part \"%1\" added to bin").arg(moduleID);
	} else {
		undoStackMsg = tr("Part \"%1\" updated in bin").arg(moduleID);
	}
	QUndoCommand *parentCmd = new QUndoCommand(undoStackMsg);

	int index = m_listView->position(moduleID);
	new PartsBinAddCommand(this, moduleID, index, parentCmd);
	m_undoStack->push(parentCmd);
}

void PartsBinPaletteWidget::removePartCommand(const QString& moduleID) {
	QUndoCommand *parentCmd = new QUndoCommand(tr("Part \"%1\" removed from the bin").arg(moduleID));

	int index = m_listView->position(moduleID);
	new PartsBinRemoveCommand(this, moduleID, index, parentCmd);
	m_undoStack->push(parentCmd);
}
