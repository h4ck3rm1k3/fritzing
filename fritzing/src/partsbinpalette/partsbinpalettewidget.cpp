/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include "searchlineedit.h"
#include "../utils/misc.h"
#include "../debugdialog.h"
#include "../dockmanager.h"
#include "../infoview/htmlinfoview.h"
#include "../utils/fileprogressdialog.h"
#include "../utils/folderutils.h"

PartsBinPaletteWidget::PartsBinPaletteWidget(ReferenceModel *refModel, HtmlInfoView *infoView, WaitPushUndoStack *undoStack, BinManager* manager) :
	QFrame(manager)
{
	m_searchLineEdit = NULL;
	m_saveQuietly = false;

	setAcceptDrops(true);
	setAllowsChanges(true);

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

	m_defaultSaveFolder = FolderUtils::getUserDataStorePath("bins");
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
    leftLayout->setSpacing(0);
	leftLayout->addWidget(m_showIconViewButton);
	leftLayout->addWidget(m_showListViewButton);

	QFrame *rightButtons = new QFrame(m_footer);
	QHBoxLayout *rightLayout = new QHBoxLayout(rightButtons);
	rightLayout->setDirection(QBoxLayout::RightToLeft);
	rightLayout->setMargin(0);
    rightLayout->setSpacing(0);
	rightLayout->addWidget(m_binMenuButton);
	rightLayout->addWidget(m_partMenuButton);

	QHBoxLayout *footerLayout = new QHBoxLayout(m_footer);
	footerLayout->setSpacing(0);
    footerLayout->setMargin(0);
	footerLayout->addWidget(leftButtons);

    footerLayout->addSpacing(8);
	m_searchLineEdit = new SearchLineEdit(m_footer);

    m_searchLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	connect(m_searchLineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(m_searchLineEdit, SIGNAL(clicked()), this, SLOT(clickedSearch()));
	footerLayout->addWidget(m_searchLineEdit);

    footerLayout->addSpacing(2);

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

bool PartsBinPaletteWidget::saveAsAux(const QString &filename) {
	FileProgressDialog progress("Saving...", 0, this);

	QString oldFilename = m_fileName;
	setFilename(filename);
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

	return true;
}

void PartsBinPaletteWidget::loadFromModel(PaletteModel *model) {
	m_iconView->loadFromModel(model);
	m_listView->setPaletteModel(model);
	afterModelSetted(model);
	m_canDeleteModel = false;				// FApplication is holding this model, so don't delete it
	setFilename(model->loadedFrom());
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
	setFilename(model->loadedFrom());
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

	// TODO: these could probably be static or moved up to the binManager...
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
	createOpenBinMenu();
	m_newBinAction = new QAction(tr("New..."), this);
	m_closeBinAction = new QAction(tr("Close"),this);
	m_saveAction = new QAction(tr("Save"),this);
	m_saveAsAction = new QAction(tr("Save As..."),this);
	m_saveAsBundledAction = new QAction(tr("Export..."),this);
	m_renameAction = new QAction(tr("Rename..."),this);

	connect(m_newBinAction, SIGNAL(triggered()),this, SLOT(newBin()));
	connect(m_closeBinAction, SIGNAL(triggered()),this, SLOT(closeBin()));
	connect(m_saveAction, SIGNAL(triggered()),this, SLOT(save()));
	connect(m_saveAsAction, SIGNAL(triggered()),this, SLOT(saveAs()));
	connect(m_saveAsBundledAction, SIGNAL(triggered()),this, SLOT(saveBundledBin()));
	connect(m_renameAction, SIGNAL(triggered()),this, SLOT(rename()));

	m_fileMenu = new QMenu(tr("Parts Bin"), this);
	m_fileMenu->addAction(newTitleAction(tr("Bin")));
	m_fileMenu->addAction(m_newBinAction);
	m_fileMenu->addMenu(m_openBinMenu);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_closeBinAction);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_saveAsAction);
	m_fileMenu->addAction(m_saveAsBundledAction);
	m_fileMenu->addAction(m_renameAction);
	m_binMenuButton->setMenu(m_fileMenu);
}

void PartsBinPaletteWidget::createOpenBinMenu() {
	m_openBinMenu = new QMenu(tr("Open"),this);

	m_openBinAction = new QAction(tr("From file..."),this);
	m_openCoreBinAction = new QAction(tr("Core"),this);
	m_openAllBinAction = new QAction(tr("All Parts"),this);
	m_openNonCoreBinAction = new QAction(tr("All User Parts"),this);
	m_openContribBinAction = new QAction(tr("Contributed Parts"),this);

	connect(m_openBinAction, SIGNAL(triggered()),this, SLOT(openNewBin()));
	connect(m_openCoreBinAction, SIGNAL(triggered()),this, SLOT(openCoreBin()));
	connect(m_openAllBinAction, SIGNAL(triggered()),this, SLOT(openAllBin()));
	connect(m_openNonCoreBinAction, SIGNAL(triggered()),this, SLOT(openNonCoreBin()));
	connect(m_openContribBinAction, SIGNAL(triggered()),this, SLOT(openContribBin()));

	m_openBinMenu->addAction(m_openCoreBinAction);
	m_openBinMenu->addAction(m_openAllBinAction);
	m_openBinMenu->addSeparator();


	QDir userBinsDir(FolderUtils::getUserDataStorePath("bins"));
	collectBins(userBinsDir);

	QFileInfo fileInfo(BinManager::AllPartsBinLocation);
	QDir dir = fileInfo.absoluteDir();
	dir.cd("more");
	collectBins(dir);

	m_openBinMenu->addAction(m_openNonCoreBinAction);
	m_openBinMenu->addAction(m_openContribBinAction);
	m_openBinMenu->addSeparator();
	m_openBinMenu->addAction(m_openBinAction);

}

void PartsBinPaletteWidget::collectBins(QDir & dir) {
	QHash<QString,QString> binsInfo;

	QStringList filters;
	filters << "*"+FritzingBinExtension;
	QFileInfoList files = dir.entryInfoList(filters);
	foreach(QFileInfo info, files) {
		QString binName = getBinName(info);
		binsInfo[info.filePath()] = binName;
	}

	foreach(QString binFile, binsInfo.keys()) {
		QAction *action = new QAction(binsInfo[binFile],this);
		action->setData(binFile);
		connect(action, SIGNAL(triggered()),this, SLOT(openUserBin()));
		m_openBinMenu->addAction(action);
	}

}

QString PartsBinPaletteWidget::getBinName(const QFileInfo &info) {
	QString binTitle = "";

	// TODO: use xmlStreamReader instead of reading the whole file

	QFile binFile(info.filePath());
	if (binFile.open(QFile::ReadOnly | QFile::Text)) {
		QString content(binFile.readAll());
		QRegExp regexp("<title>(.+)</title>");

		if (regexp.indexIn(content) >= 0) {
			binTitle = regexp.cap(1);
		}
	}

	if(binTitle != ___emptyString___) {
		return binTitle;
	} else {
		return info.fileName();
	}
}

void PartsBinPaletteWidget::openUserBin() {
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		openNewBin(action->data().toString());
	}
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

	m_partMenu = new QMenu(this);
	connect(m_partMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
	m_partMenu->addAction(newTitleAction(tr("Part")));
	m_partMenu->addAction(m_newPartAction);
	m_partMenu->addAction(m_importPartAction);
	m_partMenu->addSeparator();
	m_partMenu->addAction(m_editPartAction);
	m_partMenu->addAction(m_exportPartAction);
	m_partMenu->addAction(m_removePartAction);
	m_partMenuButton->setMenu(m_partMenu);
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
	if (FolderUtils::isEmptyFileName(m_fileName,m_untitledFileName) || currentBinIsCore()) {
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

bool PartsBinPaletteWidget::loadBundledAux(QDir &unzipDir, QList<ModelPart*> mps) {
	QStringList namefilters;
	namefilters << "*"+FritzingBinExtension;

	this->load(unzipDir.entryInfoList(namefilters)[0].filePath());
	foreach(ModelPart* mp, mps) {
		if(mp->isAlien()) { // double check
			m_alienParts << mp->moduleID();
		}
	}
	setFilename(___emptyString___);

	return true;
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
    load(BinManager::CorePartsBinLocation);
}

void PartsBinPaletteWidget::load(const QString &filename) {
	// TODO deleting this local palette reference model deletes modelPartShared held by the palette bin modelParts
	//PaletteModel * paletteReferenceModel = new PaletteModel(true, true);

	PaletteModel * oldModel = (m_canDeleteModel) ? m_model : NULL;
	PaletteModel * paletteBinModel = new PaletteModel(true, false, false);
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
		setFilename(QString::null);
	}
	setWindowModified(!isClean);
	m_manager->setDirtyTab(this,isClean);
}

bool PartsBinPaletteWidget::currentBinIsCore() {
    return m_fileName == BinManager::CorePartsBinLocation;
}


bool PartsBinPaletteWidget::beforeClosing() {
	bool retval;
	if (this->isWindowModified()) {
		QMessageBox::StandardButton reply;
		if (m_saveQuietly) {
			reply = QMessageBox::Save;
		}
		else {
            QMessageBox messageBox(this);
            messageBox.setWindowTitle(tr("Save bin \"%1\"").arg(title()));
            messageBox.setText(tr("Do you want to save the changes you made in the bin \"%1\"?").arg(title()));
            messageBox.setInformativeText(tr("Your changes will be lost if you don't save them."));
            messageBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            messageBox.setDefaultButton(QMessageBox::Save);
            messageBox.setIcon(QMessageBox::Warning);
            messageBox.setWindowModality(Qt::WindowModal);
            messageBox.setButtonText(QMessageBox::Save, tr("Save"));
            messageBox.setButtonText(QMessageBox::Discard, tr("Don't Save"));
            messageBox.button(QMessageBox::Discard)->setShortcut(tr("Ctrl+D"));
            messageBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));

			reply = (QMessageBox::StandardButton)messageBox.exec();
		}

     	if (reply == QMessageBox::Save) {
     		retval = save();
    	} else if (reply == QMessageBox::Discard) {
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


void PartsBinPaletteWidget::removeParts() {
    m_iconView->removeParts();
    m_listView->removeParts();

    // remove the model part from the model last, as this deletes it,
    // and the removePart calls above still need the modelpart
    m_model->removeParts();
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

void PartsBinPaletteWidget::openAllBin() {
	openNewBin(BinManager::AllPartsBinLocation);
}

void PartsBinPaletteWidget::openNonCoreBin() {
	openNewBin(BinManager::NonCorePartsBinLocation);
}

void PartsBinPaletteWidget::openContribBin() {
	openNewBin(BinManager::ContribPartsBinLocation);
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
	bool enabled = (mp != NULL);
	m_editPartAction->setEnabled(enabled);
	m_exportPartAction->setEnabled(enabled && !mp->isCore());
	m_removePartAction->setEnabled(enabled && allowsChanges());
	m_importPartAction->setEnabled(true);
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
	QFrame::dragEnterEvent(event);
}

void PartsBinPaletteWidget::dragLeaveEvent(QDragLeaveEvent *event) {
	QFrame::dragLeaveEvent(event);
}

void PartsBinPaletteWidget::dragMoveEvent(QDragMoveEvent *event) {
	QFrame::dragMoveEvent(event);
}

void PartsBinPaletteWidget::dropEvent(QDropEvent *event) {
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

void PartsBinPaletteWidget::setFilename(const QString &filename) {
	m_fileName = filename;
	bool acceptIt = !currentBinIsCore();
	setAcceptDrops(acceptIt);
	m_iconView->setAcceptDrops(acceptIt);
	m_listView->setAcceptDrops(acceptIt);
}

void PartsBinPaletteWidget::search() {
	SearchLineEdit * edit = qobject_cast<SearchLineEdit *>(sender());
	if (edit == NULL) return;

	QString searchText = edit->text();
	if (searchText.isEmpty()) return;

    m_manager->search(searchText);
}

void PartsBinPaletteWidget::clickedSearch() {
    m_manager->clickedSearch(this);
}

bool PartsBinPaletteWidget::allowsChanges() {
	return m_allowsChanges;
}

void PartsBinPaletteWidget::setAllowsChanges(bool allowsChanges) {
	m_allowsChanges = allowsChanges;
}

void PartsBinPaletteWidget::focusSearch() {
	if (m_searchLineEdit) {
		QTimer::singleShot(20, this, SLOT(focusSearchAfter()));
	}
}

void PartsBinPaletteWidget::focusSearchAfter() {
	//DebugDialog::debug("focus search after");
	if (m_searchLineEdit->decoy()) {
		m_searchLineEdit->setDecoy(false);
	}
	m_searchLineEdit->setFocus(Qt::OtherFocusReason);
}

void PartsBinPaletteWidget::setSaveQuietly(bool saveQuietly) {
	m_saveQuietly = saveQuietly;
}

bool PartsBinPaletteWidget::currentViewIsIconView() {
	if (m_currentView == NULL) return true;

	return (m_currentView == m_iconView);
}

QMenu * PartsBinPaletteWidget::getFileMenu() {
	return m_fileMenu;
}

QMenu * PartsBinPaletteWidget::getPartMenu() {
	return m_partMenu;
}
