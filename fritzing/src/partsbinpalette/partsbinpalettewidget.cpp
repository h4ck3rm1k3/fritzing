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

$Revision$:
$Author$:
$Date$

********************************************************************/


#include <QUndoStack>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QInputDialog>
#include <QWidgetAction>

#include "partsbinpalettewidget.h"
#include "partsbincommands.h"
#include "partsbiniconview.h"
#include "partsbinlistview.h"
#include "simpleeditablelabelwidget.h"
#include "binmanager/binmanager.h"
#include "../fritzingwindow.h"
#include "../utils/misc.h"
#include "../debugdialog.h"
#include "../dockmanager.h"
#include "../htmlinfoview.h"
#include "../utils/fileprogressdialog.h"


PartsBinPaletteWidget::PartsBinPaletteWidget(ReferenceModel *refModel, HtmlInfoView *infoView, WaitPushUndoStack *undoStack, BinManager* manager) :
	QFrame(manager)
{
	setAcceptDrops(true);

	m_tabWidget = NULL;
	m_manager = manager;

	m_refModel = refModel;
	m_canDeleteModel = false;
	m_orderHasChanged = false;

	Q_UNUSED(undoStack);

	m_undoStack = new WaitPushUndoStack(this);PartsBinPaletteWidget::
	connect(m_undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(undoStackCleanChanged(bool)) );

	setupButtons();
	setupFooter();

	m_iconView = new PartsBinIconView(m_refModel, this, m_binContextMenu, m_partContextMenu);
	m_iconView->setInfoView(infoView);

	m_listView = new PartsBinListView(m_refModel, this, m_binContextMenu, m_partContextMenu);
	m_listView->setInfoView(infoView);

	setObjectName("partsBinContainer");
	toIconView();

	m_defaultSaveFolder = getUserDataStorePath("bins");
	m_untitledFileName = tr("Untitled Bin");

	connect(m_listView, SIGNAL(currentRowChanged(int)), m_iconView, SLOT(setSelected(int)));
	connect(m_iconView, SIGNAL(selectionChanged(int)), m_listView, SLOT(setSelected(int)));

	connect(m_listView, SIGNAL(currentRowChanged(int)), this, SLOT(updateMenus()));
	connect(m_iconView, SIGNAL(selectionChanged(int)), this, SLOT(updateMenus()));

	//connect(m_listView, SIGNAL(clicked()), this, SLOT(updateButtonStates()));
	//connect(m_iconView, SIGNAL(clicked()(int)), this, SLOT(updateButtonStates()));

	connect(m_listView, SIGNAL(informItemMoved(int,int)), m_iconView, SLOT(itemMoved(int,int)));
	connect(m_iconView, SIGNAL(informItemMoved(int,int)), m_listView, SLOT(itemMoved(int,int)));
	connect(m_listView, SIGNAL(informItemMoved(int,int)), this, SLOT(itemMoved()));
	connect(m_iconView, SIGNAL(informItemMoved(int,int)), this, SLOT(itemMoved()));

	m_addPartToMeAction = new QAction(m_title,this);
	connect(m_addPartToMeAction, SIGNAL(triggered()),this, SLOT(addSketchPartToMe()));

	installEventFilter(this);
}

PartsBinPaletteWidget::~PartsBinPaletteWidget() {
	if (m_canDeleteModel && m_model != NULL) {
		delete m_model;
		m_model = NULL;
	}
}

QSize PartsBinPaletteWidget::sizeHint() const {
	return QSize(DockManager::DockDefaultWidth, DockManager::PartsBinDefaultHeight);
}

QString PartsBinPaletteWidget::title() const {
	return m_title;
}

void PartsBinPaletteWidget::setTitle(const QString &title) {
	if(m_title != title) {
		m_title = title;
		m_addPartToMeAction->setText(title);
		m_manager->updateTitle(this, title);
	}
}

void PartsBinPaletteWidget::setTabWidget(StackTabWidget *tabWidget) {
	m_tabWidget = tabWidget;
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
	rightLayout->addWidget(m_binMenuButton);
	rightLayout->addWidget(m_partMenuButton);

	QHBoxLayout *footerLayout = new QHBoxLayout(m_footer);
	footerLayout->setMargin(2);
	footerLayout->setSpacing(0);
	footerLayout->addWidget(leftButtons);
	footerLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	footerLayout->addWidget(rightButtons);
}

void PartsBinPaletteWidget::setView(PartsBinView *view) {
	m_currentView = view;
	if(m_currentView == m_iconView) {
		m_iconView->show();
		m_showIconViewButton->setEnabledIcon();
		m_listView->hide();
		m_showListViewButton->setDisabledIcon();
	} else if(m_currentView == m_listView) {
		m_listView->show();
		m_showListViewButton->setEnabledIcon();
		m_iconView->hide();
		m_showIconViewButton->setDisabledIcon();
	}

	delete layout();
	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->setMargin(3);
	lo->setSpacing(0);
	lo->addWidget(dynamic_cast<QWidget*>(m_currentView));
	lo->addWidget(m_footer);
}

void PartsBinPaletteWidget::toIconView() {
	setView(m_iconView);
}

void PartsBinPaletteWidget::toListView() {
	disconnect(m_listView, SIGNAL(currentRowChanged(int)), m_iconView, SLOT(setSelected(int)));
	setView(m_listView);
	connect(m_listView, SIGNAL(currentRowChanged(int)), m_iconView, SLOT(setSelected(int)));
}

void PartsBinPaletteWidget::saveAsAux(const QString &filename) {
	FileProgressDialog progress("Saving...", 0, this);

	QString oldFilename = m_fileName;
	m_fileName = filename;
	QString title = this->title();
	if(!title.isNull() && !title.isEmpty()) {
		m_model->root()->modelPartShared()->setTitle(title);
	}

	if(m_orderHasChanged) {
		m_model->setOrdererChildren(m_iconView->orderedChildren());
	}
	m_model->save(filename, false);
	if(m_orderHasChanged) {
		m_model->setOrdererChildren(QList<QObject*>());
		m_orderHasChanged = false;
	}

	m_undoStack->setClean();

	saveAsLastBin();
	if(oldFilename != m_fileName) {
		emit fileNameUpdated(this,m_fileName,oldFilename);
	}
	emit saved(hasAlienParts());
}

void PartsBinPaletteWidget::loadFromModel(PaletteModel *model) {
	m_iconView->loadFromModel(model);
	m_listView->setPaletteModel(model);
	afterModelSetted(model);
	m_canDeleteModel = false;				// FApplication is holding this model, so don't delete it
	m_fileName = model->loadedFrom();
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
}

void PartsBinPaletteWidget::grabTitle(PaletteModel *model) {
	m_title = model->root()->modelPartShared()->title();
	m_addPartToMeAction->setText(m_title);
}

void PartsBinPaletteWidget::addPart(ModelPart *modelPart, int position) {
	ModelPart *mp = m_model->addModelPart(m_model->root(),modelPart);

	m_iconView->addPart(mp, position);
	m_listView->addPart(mp, position);

	if(modelPart->isAlien()) {
		m_alienParts << mp->moduleID();
	}
}


void PartsBinPaletteWidget::setupButtons() {
	m_showIconViewButton = new ImageButton("IconView",this);
	m_showIconViewButton->setToolTip(tr("Show as icons"));
	connect(m_showIconViewButton,SIGNAL(clicked()),this,SLOT(toIconView()));

	m_showListViewButton = new ImageButton("ListView",this);
	m_showListViewButton->setToolTip(tr("Show as list"));
	connect(m_showListViewButton,SIGNAL(clicked()),this,SLOT(toListView()));

	createBinMenu();
	createPartMenu();
	createContextMenus();
}

QToolButton* PartsBinPaletteWidget::newToolButton(const QString& btnObjName, const QString& imgPath, const QString &text) {
	QToolButton *toolBtn = new QToolButton(this);
	toolBtn->setObjectName(btnObjName);
	if(text != ___emptyString___) {
		toolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		toolBtn->setText(text);
	}
	toolBtn->setPopupMode(QToolButton::InstantPopup);
	if(imgPath != ___emptyString___) {
		toolBtn->setIcon(QIcon(imgPath));
	}
	toolBtn->setArrowType(Qt::NoArrow);
	return toolBtn;
}

QAction* PartsBinPaletteWidget::newTitleAction(const QString &text) {
	QWidgetAction *act = new QWidgetAction(this);
	QWidget* w = new QLabel(text,this);
	w->setObjectName("titleAction");
	act->setDefaultWidget(w);
	act->setDisabled(true);
	return act;
}

void PartsBinPaletteWidget::createBinMenu() {
	m_binMenuButton = newToolButton("partsBinBinMenu");

	m_newBinAction = new QAction(tr("New..."), this);
	m_openBinAction = new QAction(tr("Open..."),this);
	m_openCoreBinAction = new QAction(tr("Open Core"),this);
	m_closeBinAction = new QAction(tr("Close"),this);
	m_saveAction = new QAction(tr("Save"),this);
	m_saveAsAction = new QAction(tr("Save As..."),this);
	m_saveAsBundledAction = new QAction(tr("Export..."),this);
	m_renameAction = new QAction(tr("Rename..."),this);

	connect(m_newBinAction, SIGNAL(triggered()),this, SLOT(newBin()));
	connect(m_openBinAction, SIGNAL(triggered()),this, SLOT(openNewBin()));
	connect(m_openCoreBinAction, SIGNAL(triggered()),this, SLOT(openCoreBin()));
	connect(m_closeBinAction, SIGNAL(triggered()),this, SLOT(closeBin()));
	connect(m_saveAction, SIGNAL(triggered()),this, SLOT(save()));
	connect(m_saveAsAction, SIGNAL(triggered()),this, SLOT(saveAs()));
	connect(m_saveAsBundledAction, SIGNAL(triggered()),this, SLOT(saveBundledBin()));
	connect(m_renameAction, SIGNAL(triggered()),this, SLOT(rename()));

	QMenu *menu = new QMenu(this);
	menu->addAction(newTitleAction(tr("Bin")));
	menu->addAction(m_newBinAction);
	menu->addAction(m_openBinAction);
	menu->addAction(m_openCoreBinAction);
	menu->addSeparator();
	menu->addAction(m_closeBinAction);
	menu->addAction(m_saveAction);
	menu->addAction(m_saveAsAction);
	menu->addAction(m_saveAsBundledAction);
	menu->addAction(m_renameAction);
	m_binMenuButton->setMenu(menu);
}

void PartsBinPaletteWidget::createPartMenu() {
	m_partMenuButton = newToolButton("partsBinPartMenu");

	m_newPartAction = new QAction(tr("New..."), this);
	m_importPartAction = new QAction(tr("Import..."),this);
	m_editPartAction = new QAction(tr("Edit..."),this);
	m_exportPartAction = new QAction(tr("Export..."),this);
	m_removePartAction = new QAction(tr("Remove"),this);

	connect(m_newPartAction, SIGNAL(triggered()),this, SLOT(newPart()));
	connect(m_importPartAction, SIGNAL(triggered()),this, SLOT(importPart()));
	connect(m_editPartAction, SIGNAL(triggered()),this, SLOT(editSelected()));
	connect(m_exportPartAction, SIGNAL(triggered()),this, SLOT(exportSelected()));
	connect(m_removePartAction, SIGNAL(triggered()),this, SLOT(removeSelected()));

	QMenu *menu = new QMenu(this);
	connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
	menu->addAction(newTitleAction(tr("Part")));
	menu->addAction(m_newPartAction);
	menu->addAction(m_importPartAction);
	menu->addSeparator();
	menu->addAction(m_editPartAction);
	menu->addAction(m_exportPartAction);
	menu->addAction(m_removePartAction);
	m_partMenuButton->setMenu(menu);
}

void PartsBinPaletteWidget::createContextMenus() {
	m_binContextMenu = new QMenu(this);
	m_binContextMenu->addAction(m_closeBinAction);
	m_binContextMenu->addAction(m_saveAction);
	m_binContextMenu->addAction(m_saveAsAction);
	m_binContextMenu->addAction(m_renameAction);

	m_partContextMenu = new QMenu(this);
	connect(m_partContextMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
	m_partContextMenu->addAction(m_editPartAction);
	m_partContextMenu->addAction(m_exportPartAction);
	m_partContextMenu->addAction(m_removePartAction);
}

bool PartsBinPaletteWidget::removeSelected() {
	if(selected()) {
		QString modId = selected()->moduleID();
		removePartCommand(modId);
		return true;
	} else return false;
}

bool PartsBinPaletteWidget::save() {
	bool result = true;
	if (FritzingWindow::isEmptyFileName(m_fileName,m_untitledFileName) || currentBinIsCore()) {
		result = saveAs();
	} else {
		saveAsAux(m_fileName);
	}
	if(result) m_manager->setDirtyTab(this,false);
	return result;
}

bool PartsBinPaletteWidget::saveAs() {
	QString fileExt;
    QString fileName = QFileDialog::getSaveFileName(
		this,
		tr("Specify a file name"),
		(m_fileName.isNull() || m_fileName.isEmpty() || /* it's a resource */ m_fileName.startsWith(":"))?
				m_defaultSaveFolder+"/"+title()+FritzingBinExtension:
				m_fileName,
		tr("Fritzing Bin (*%1)").arg(FritzingBinExtension),
		&fileExt
	  );

    if (fileName.isEmpty()) return false; // Cancel pressed

    if(!FritzingWindow::alreadyHasExtension(fileName, FritzingBinExtension)) {
		fileName += FritzingBinExtension;
	}
    saveAsAux(fileName);
    return true;
}

void PartsBinPaletteWidget::saveBundledBin() {
	bool wasModified = m_isDirty;
	m_manager->mainWindow()->saveBundledNonAtomicEntity(
		m_fileName, FritzingBundledBinExtension, this,
		m_model->root()->getAllNonCoreParts()
	);
	setDirty(wasModified);
	saveAsLastBin();
}

void PartsBinPaletteWidget::loadBundledAux(QDir &unzipDir, QList<ModelPart*> mps) {
	QStringList namefilters;
	namefilters << "*"+FritzingBinExtension;

	this->load(unzipDir.entryInfoList(namefilters)[0].filePath());
	foreach(ModelPart* mp, mps) {
		if(mp->isAlien()) { // double check
			m_alienParts << mp->moduleID();
		}
	}
	m_fileName = ___emptyString___;
}


bool PartsBinPaletteWidget::open(QString fileName) {
	QFile file(fileName);
	if (!file.exists()) {
       QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot find file %1.")
                             .arg(fileName));
		return false;
	}

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    file.close();

    if(fileName.endsWith(FritzingBinExtension)) {
    	load(fileName);
    	saveAsLastBin();
    	m_isDirty = false;
    } else if(fileName.endsWith(FritzingBundledBinExtension)) {
    	m_manager->mainWindow()->loadBundledNonAtomicEntity(fileName,this,false);
    }

    return true;
}

void PartsBinPaletteWidget::openCore() {
	load(FritzingWindow::CoreBinLocation);
}

void PartsBinPaletteWidget::load(const QString &filename) {
	// TODO deleting this local palette reference model deletes modelPartShared held by the palette bin modelParts
	//PaletteModel * paletteReferenceModel = new PaletteModel(true, true);

	PaletteModel * oldModel = (m_canDeleteModel) ? m_model : NULL;
	PaletteModel * paletteBinModel = new PaletteModel(true, false);
	if (!paletteBinModel->load(filename, m_refModel)) {		// paletteReferenceModel
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Friting cannot load the parts bin"));
		return;
	}
	setPaletteModel(paletteBinModel,true);
	m_canDeleteModel = true;					// since we just created this model, we can delete it later
	if (oldModel) {
		delete oldModel;
	}
	//delete paletteReferenceModel;
}

void PartsBinPaletteWidget::undoStackCleanChanged(bool isClean) {
	if(!isClean && currentBinIsCore()) {
		m_fileName = QString::null;
	}
	setWindowModified(!isClean);
	m_manager->setDirtyTab(this,isClean);
}

bool PartsBinPaletteWidget::currentBinIsCore() {
	return m_fileName == FritzingWindow::CoreBinLocation;
}


bool PartsBinPaletteWidget::beforeClosing() {
	bool retval;
	if (this->isWindowModified()) {
		QMessageBox::StandardButton reply;
		QMessageBox messageBox(
				tr("Save \"%1\"").arg(QFileInfo(m_fileName).baseName()),
				tr("Do you want to save the changes you made in the bin \"%1\"?")
					.arg(title()),
				QMessageBox::Warning,
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default,
				this, Qt::Sheet);

		messageBox.setButtonText(QMessageBox::Yes, tr("Save"));
		messageBox.setButtonText(QMessageBox::No, tr("Don't Save"));
		messageBox.button(QMessageBox::No)->setShortcut(tr("Ctrl+D"));
		messageBox.setInformativeText(tr("Your changes will be lost if you don't save them."));

		reply = (QMessageBox::StandardButton)messageBox.exec();

     	if (reply == QMessageBox::Yes) {
     		retval = save();
    	} else if (reply == QMessageBox::No) {
    		retval = true;
        } else {
         	retval = false;
        }
	} else {
		retval = true;
	}
	return retval;
}

void PartsBinPaletteWidget::saveAsLastBin() {
	QSettings settings;
	settings.setValue("lastBin",m_fileName);
}

void PartsBinPaletteWidget::closeEvent(QCloseEvent* event) {
	saveAsLastBin();
	QFrame::closeEvent(event);
}

void PartsBinPaletteWidget::mousePressEvent(QMouseEvent* event) {
	emit focused(this);
	QFrame::mousePressEvent(event);
}

ModelPart * PartsBinPaletteWidget::selected() {
	return m_currentView->selected();
}

bool PartsBinPaletteWidget::contains(const QString &moduleID) {
	return m_iconView->contains(moduleID);
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
	/*bool updating = alreadyIn(moduleID);

	QString partTitle = m_refModel->partTitle(moduleID);
	if(partTitle == ___emptyString___) partTitle = moduleID;

	QString undoStackMsg;

	if(!updating) {
		undoStackMsg = tr("\"%1\" added to bin").arg(partTitle);
	} else {
		undoStackMsg = tr("\"%1\" updated in bin").arg(partTitle);
	}
	QUndoCommand *parentCmd = new QUndoCommand(undoStackMsg);

	int index = m_listView->position(moduleID);
	new PartsBinAddCommand(this, moduleID, index, parentCmd);
	m_undoStack->push(parentCmd);*/

	QMessageBox::StandardButton answer = QMessageBox::question(
		this,
		tr("Add to bin"),
		tr("Do you really want to add the selected part to the bin?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::Yes
	);
	// TODO: make button texts translatable
	if(answer == QMessageBox::Yes) {
		int index = m_listView->position(moduleID);
		m_undoStack->push(new QUndoCommand("Parts bin: part added"));
		addPart(moduleID, index);
	}
}

void PartsBinPaletteWidget::removePartCommand(const QString& moduleID) {
	/*QString partTitle = m_refModel->partTitle(moduleID);
	if(partTitle == ___emptyString___) partTitle = moduleID;

	QUndoCommand *parentCmd = new QUndoCommand(tr("\"%1\" removed from the bin").arg(partTitle));

	int index = m_listView->position(moduleID);
	new PartsBinRemoveCommand(this, moduleID, index, parentCmd);
	m_undoStack->push(parentCmd);*/

	QMessageBox::StandardButton answer = QMessageBox::question(
		this,
		tr("Remove from bin"),
		tr("Do you really want to remove the selected part from the bin?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::Yes
	);
	// TODO: make button texts translatable
	if(answer == QMessageBox::Yes) {
		m_undoStack->push(new QUndoCommand("Parts bin: part removed"));
		removePart(moduleID);
	}

	setDirty();
}

void PartsBinPaletteWidget::newBin() {
	m_manager->newBinIn(m_tabWidget);
}

void PartsBinPaletteWidget::openNewBin(const QString &filename) {
	m_manager->openBinIn(m_tabWidget,filename);
}

void PartsBinPaletteWidget::openCoreBin() {
	m_manager->openCoreBinIn(m_tabWidget);
}

void PartsBinPaletteWidget::closeBin() {
	m_manager->closeBinIn(m_tabWidget);
}


void PartsBinPaletteWidget::rename() {
	bool ok;
	QString newTitle = QInputDialog::getText(
		this,
		tr("Rename bin"),
		tr("Please choose a name for the bin:"),
		QLineEdit::Normal,
		m_title,
		&ok
	);
	if(ok) {
		setTitle(newTitle);
	}
}

void PartsBinPaletteWidget::newPart() {
	m_manager->newPartTo(this);
}

void PartsBinPaletteWidget::importPart() {
	m_manager->importPartTo(this);
}

void PartsBinPaletteWidget::editSelected() {
	m_manager->editSelectedPartFrom(this);
}

void PartsBinPaletteWidget::exportSelected() {
	ModelPart *mp = selected();
	if(mp) {
		emit savePartAsBundled(mp->moduleID());
	}
}

void PartsBinPaletteWidget::itemMoved() {
	m_orderHasChanged = true;
	m_manager->setDirtyTab(this);
}

void PartsBinPaletteWidget::setDirty(bool dirty) {
	m_manager->setDirtyTab(this,dirty);
	m_isDirty = dirty;
}

const QString &PartsBinPaletteWidget::fileName() {
	return m_fileName;
}

void PartsBinPaletteWidget::updateMenus() {
	ModelPart *mp = selected();
	bool enabled = mp != NULL;
	m_editPartAction->setEnabled(enabled);
	m_exportPartAction->setEnabled(enabled && !mp->isCore());
	m_removePartAction->setEnabled(enabled);
}

bool PartsBinPaletteWidget::eventFilter(QObject *obj, QEvent *event) {
	if (obj == this) {
		if (event->type() == QEvent::MouseButtonPress ||
			event->type() == QEvent::GraphicsSceneDragMove ||
			event->type() == QEvent::GraphicsSceneDrop ||
			event->type() == QEvent::GraphicsSceneMousePress
		) {
			emit focused(this);
		}
	}
	return QFrame::eventFilter(obj, event);
}

PartsBinView *PartsBinPaletteWidget::currentView() {
	return m_currentView;
}

bool PartsBinPaletteWidget::isOverFooter(QDropEvent* event) {
	QRect mappedFooterRect(
		m_footer->pos(),
		m_footer->rect().size()
	);
	return mappedFooterRect.contains(event->pos());
}

void PartsBinPaletteWidget::dragEnterEvent(QDragEnterEvent *event) {
	if(BinManager::isTabReorderingEvent(event)) {
		event->acceptProposedAction();
	}
	QFrame::dragEnterEvent(event);
}

void PartsBinPaletteWidget::dragLeaveEvent(QDragLeaveEvent *event) {
	emit draggingCloseToSeparator((QWidget*)m_tabWidget,false);
	QFrame::dragLeaveEvent(event);
}

void PartsBinPaletteWidget::dragMoveEvent(QDragMoveEvent *event) {
	if(isOverFooter(event) && BinManager::isTabReorderingEvent(event)) {
		event->acceptProposedAction();
		emit draggingCloseToSeparator((QWidget*)m_tabWidget,true);
	}
	QFrame::dragMoveEvent(event);
}

void PartsBinPaletteWidget::dropEvent(QDropEvent *event) {
	if(isOverFooter(event) && BinManager::isTabReorderingEvent(event)) {
		emit dropToSeparator((QWidget*)m_tabWidget);
	}
	QFrame::dropEvent(event);
}


QAction *PartsBinPaletteWidget::addPartToMeAction() {
	return m_addPartToMeAction;
}

void PartsBinPaletteWidget::addSketchPartToMe() {
	QString moduleID = m_manager->getSelectedModuleIDFromSketch();
	bool wasAlreadyIn = contains(moduleID);
	addPart(moduleID);
	if(!wasAlreadyIn) {
		setDirty();
	}
}
