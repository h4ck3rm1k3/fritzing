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


#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QtDebug>

#include "binmanager.h"
#include "stacktabwidget.h"
#include "stacktabbar.h"

#include "../partsbinpalettewidget.h"
#include "../../model/modelpart.h"
#include "../../mainwindow.h"
#include "../../model/palettemodel.h"
#include "../../waitpushundostack.h"
#include "../../debugdialog.h"
#include "../../partseditor/partseditormainwindow.h"
#include "../../utils/folderutils.h"
#include "../../referencemodel/referencemodel.h"


QString BinManager::Title;
QString BinManager::MyPartsBinLocation;
QString BinManager::MyPartsBinTemplateLocation;
QString BinManager::SearchBinLocation;
QString BinManager::SearchBinTemplateLocation;
QString BinManager::AllPartsBinLocation;
QString BinManager::NonCorePartsBinLocation;
QString BinManager::ContribPartsBinLocation;
QString BinManager::CorePartsBinLocation;

QString BinManager::StandardBinStyle = "background-color: gray;";
QString BinManager::CurrentBinStyle = "background-color: black;";


BinManager::BinManager(class ReferenceModel *refModel, class HtmlInfoView *infoView, WaitPushUndoStack *undoStack, MainWindow* parent)
	: QFrame(parent)
{
	BinManager::Title = tr("Parts");

	m_refModel = refModel;
	m_paletteModel = NULL;
	m_infoView = infoView;
	m_undoStack = undoStack;
	m_defaultSaveFolder = FolderUtils::getUserDataStorePath("bins");
	m_mainWindow = parent;
	m_currentBin = NULL;

	m_stackWidget = new StackWidget(this);
	m_stackWidget->setAcceptDrops(true);

	m_unsavedBinsCount = 0;

	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->addWidget(m_stackWidget);
	lo->setMargin(0);
	lo->setSpacing(0);
	setMaximumHeight(500);

	restoreStateAndGeometry();
}

BinManager::~BinManager() {

}

void BinManager::addBin(PartsBinPaletteWidget* bin) {
	StackTabWidget *tb = new StackTabWidget(m_stackWidget);
	tb->addTab(bin,bin->title());
	m_stackWidget->addWidget(tb);
	registerBin(bin,tb);
	connectTabWidget(tb);
	setAsCurrentBin(bin);
}

void BinManager::registerBin(PartsBinPaletteWidget* bin, StackTabWidget *tb) {
	bin->setTabWidget(tb);
	if(!m_tabWidgets.values().contains(tb)) {
		connectTabWidget(tb);
	}
	m_tabWidgets[bin] = tb;
	if(bin->fileName() != ___emptyString___) {
		m_openedBins[bin->fileName()] = bin;
	}

	if (bin->fileName().compare(CorePartsBinLocation) == 0) {
		bin->setAllowsChanges(false);
	}
	else if (bin->fileName().compare(SearchBinLocation) == 0) {
		bin->setAllowsChanges(false);
	}
	else if (bin->fileName().compare(ContribPartsBinLocation) == 0) {
		bin->setAllowsChanges(false);
	}
}

void BinManager::connectTabWidget(StackTabWidget *tw) {
	connect(
		tw, SIGNAL(currentChanged(StackTabWidget*,int)),
		this, SLOT(currentChanged(StackTabWidget*,int))
	);
	connect(
		tw, SIGNAL(tabCloseRequested(StackTabWidget*,int)),
		this, SLOT(tabCloseRequested(StackTabWidget*,int))
	);
}

void BinManager::insertBin(PartsBinPaletteWidget* bin, int index, StackTabWidget* tb) {
	registerBin(bin,tb);
	tb->insertTab(index,bin,bin->title());
	tb->setCurrentIndex(index);
	m_tabWidgets[bin] = tb;
}

void BinManager::loadFromModel(PaletteModel *model) {
	Q_UNUSED(model);
	/*PartsBinPaletteWidget* bin = newBin();
	m_paletteModel=model;
	bin->loadFromModel(model);
	addBin(bin);*/
}

void BinManager::setPaletteModel(PaletteModel *model) {
	m_paletteModel = model;
}


bool BinManager::beforeClosing() {
	bool retval = true;

	for(int i=0; i < m_stackWidget->count(); i++) {
		StackTabWidget *tw = dynamic_cast<StackTabWidget*>(m_stackWidget->widget(i));
		if(tw) {
			for(int j=0; j < tw->count(); j++) {
				PartsBinPaletteWidget *bin = dynamic_cast<PartsBinPaletteWidget*>(tw->widget(j));
				if(bin) {
					setAsCurrentTab(bin);
					retval = retval && bin->beforeClosing();
					if(!retval) break;
				}
			}
		}
	}
	if(retval) {
		saveStateAndGeometry();
	}

	return retval;
}

void BinManager::setAsCurrentTab(PartsBinPaletteWidget* bin) {
	m_tabWidgets[bin]->setCurrentWidget(bin);
}


bool BinManager::hasAlienParts() {
	return false;

}

void BinManager::setInfoViewOnHover(bool infoViewOnHover) {
	Q_UNUSED(infoViewOnHover);
}

void BinManager::addNewPart(ModelPart *modelPart) {
	PartsBinPaletteWidget* myPartsBin = getOrOpenMyPartsBin();
	myPartsBin->addPart(modelPart);
	setDirtyTab(myPartsBin);
}

PartsBinPaletteWidget* BinManager::getOrOpenMyPartsBin() {
    return getOrOpenBin(MyPartsBinLocation, MyPartsBinTemplateLocation);
}

PartsBinPaletteWidget* BinManager::getOrOpenSearchBin() {
    PartsBinPaletteWidget * bin = getOrOpenBin(SearchBinLocation, SearchBinTemplateLocation);
	if (bin) {
		bin->setSaveQuietly(true);
	}
	return bin;
}

PartsBinPaletteWidget* BinManager::getOrOpenBin(const QString & binLocation, const QString & binTemplateLocation) {

    PartsBinPaletteWidget* partsBin = NULL;

	foreach(PartsBinPaletteWidget* bin, m_tabWidgets.keys()) {
        if(bin->fileName() == binLocation) {
            partsBin = bin;
			break;
		}
	}

    if(!partsBin) {
        QString fileToOpen = QFileInfo(binLocation).exists()?
            binLocation:
            createIfBinNotExists(binLocation, binTemplateLocation);

        partsBin = openBinIn(m_tabWidgets.values()[0], fileToOpen);
	}

    return partsBin;
}

QString BinManager::createIfMyPartsNotExists() {
    return createIfBinNotExists(MyPartsBinLocation, MyPartsBinTemplateLocation);
}

QString BinManager::createIfSearchNotExists() {
    return createIfBinNotExists(SearchBinLocation, SearchBinTemplateLocation);
}

QString BinManager::createIfBinNotExists(const QString & dest, const QString & source)
{
    QString binPath = dest;
    QFile file(source);
	file.copy(binPath);
	return binPath;
}

void BinManager::addPart(ModelPart *modelPart, int position) {
	PartsBinPaletteWidget *bin = m_currentBin? m_currentBin: getOrOpenMyPartsBin();
	addPartAux(bin,modelPart,position);
}

void BinManager::addToMyPart(ModelPart *modelPart) {
	PartsBinPaletteWidget *bin = getOrOpenMyPartsBin();
	addPartAux(bin,modelPart);
	if (bin) {
		setAsCurrentTab(bin);
	}
}

void BinManager::addPartAux(PartsBinPaletteWidget *bin, ModelPart *modelPart, int position) {
	if(bin) {
		bin->addPart(modelPart, position);
		setDirtyTab(bin);
	}
}

void BinManager::load(const QString& filename) {
	openBin(filename);
}


void BinManager::setDirtyTab(PartsBinPaletteWidget* w, bool dirty) {
	w->setWindowModified(dirty);
	QTabWidget* tw = m_tabWidgets[w];
	if(tw) {
		int tabIdx = tw->indexOf(w);
		tw->setTabText(tabIdx, w->title()+(dirty? " *": ""));
	} else {
		qWarning() << tr("BinManager::setDirtyTab: Couldn't set the bin '%1' as dirty").arg(w->title());
	}
}

void BinManager::updateTitle(PartsBinPaletteWidget* w, const QString& newTitle) {
	QTabWidget* tw = m_tabWidgets[w];
	if(tw) {
		tw->setTabText(tw->indexOf(w), newTitle+" *");
		setDirtyTab(w);
	}
}

PartsBinPaletteWidget* BinManager::newBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget* bin = newBin();
	bin->setPaletteModel(new PaletteModel(true, false, false),true);
	bin->setTitle(tr("New bin (%1)").arg(++m_unsavedBinsCount));
	insertBin(bin, tb->currentIndex(), tb);
	bin->rename();
	return bin;
}

PartsBinPaletteWidget* BinManager::openBinIn(StackTabWidget* tb, QString fileName) {
	if(fileName.isNull() || fileName.isEmpty()) {
		fileName = QFileDialog::getOpenFileName(
				this,
				tr("Select a Fritzing file to open"),
				m_defaultSaveFolder,
				tr("Fritzing Bin Files (*%1 *%2);;Fritzing Bin (*%1);;Fritzing Shareable Bin (*%2)")
				.arg(FritzingBinExtension).arg(FritzingBundledBinExtension)
		);
		if (fileName.isNull()) return false;
	}
	PartsBinPaletteWidget* bin = NULL;
	bool createNewOne = false;
	if(m_openedBins.contains(fileName)) {
		bin = m_openedBins[fileName];
		if(m_tabWidgets[bin]) {
			m_tabWidgets[bin]->setCurrentWidget(bin);
		} else {
			m_openedBins.remove(fileName);
			createNewOne = true;
		}
	} else {
		createNewOne = true;
	}

	if(createNewOne) {
		bin = newBin();
		if(bin->open(fileName)) {
			m_openedBins[fileName] = bin;
			insertBin(bin, tb->currentIndex()+1, tb);

			// to force the user to take a decision of what to do with the imported parts
			if(fileName.endsWith(FritzingBundledBinExtension)) {
				setDirtyTab(bin);
			} else {
				bin->saveAsLastBin();
			}
		}
	}
	setAsCurrentBin(bin);
	return bin;
}

PartsBinPaletteWidget* BinManager::openCoreBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget* bin = newBin();
	bin->setAllowsChanges(false);
	bin->openCore();
	insertBin(bin, tb->currentIndex()+1, tb);
	setAsCurrentBin(bin);
	return bin;
}

PartsBinPaletteWidget* BinManager::newBin() {
	PartsBinPaletteWidget* bin = new PartsBinPaletteWidget(m_refModel,m_infoView,m_undoStack,this);
	connect(
		bin, SIGNAL(fileNameUpdated(PartsBinPaletteWidget*, const QString&, const QString&)),
		this, SLOT(updateFileName(PartsBinPaletteWidget*, const QString&, const QString&))
	);
	connect(
		bin, SIGNAL(focused(PartsBinPaletteWidget*)),
		this, SLOT(setAsCurrentBin(PartsBinPaletteWidget*))
	);
	connect(
		bin, SIGNAL(savePartAsBundled(const QString &)),
		m_mainWindow, SLOT(saveBundledPart(const QString &))
	);
	connect(bin, SIGNAL(saved(bool)), m_mainWindow, SLOT(binSaved(bool)));
	connect(m_mainWindow, SIGNAL(alienPartsDismissed()), bin, SLOT(removeAlienParts()));

	return bin;
}

void BinManager::currentChanged(StackTabWidget *tw, int index) {
	PartsBinPaletteWidget *bin = dynamic_cast<PartsBinPaletteWidget*>(tw->widget(index));
	if(bin) setAsCurrentBin(bin);
}

void BinManager::setAsCurrentBin(PartsBinPaletteWidget* bin) {
	if(bin) {
		if(m_currentBin != bin) {
			QString style = m_mainWindow->styleSheet();
			StackTabBar *currTabBar = NULL;
			if(m_currentBin && m_tabWidgets[m_currentBin]) {
				currTabBar = m_tabWidgets[m_currentBin]->stackTabBar();
				currTabBar->setProperty("current","false");
				currTabBar->setStyleSheet("");
				currTabBar->setStyleSheet(style);
			}
			if(m_tabWidgets[bin]) {
				m_currentBin = bin;
				currTabBar = m_tabWidgets[m_currentBin]->stackTabBar();
				currTabBar->setProperty("current","true");
				currTabBar->setStyleSheet("");
				currTabBar->setStyleSheet(style);
			}
		}
	} else {
		qWarning() << tr("Cannot set a NULL bin as the current one");
	}
}

void BinManager::closeBinIn(StackTabWidget* tb, int index) {
	int realIndex = index == -1? tb->currentIndex(): index;
	PartsBinPaletteWidget *w = getBin(tb, realIndex);
	if(w && w->beforeClosing()) {
		tb->removeTab(realIndex);
		m_tabWidgets.remove(w);
		m_openedBins.remove(w->fileName());

		bool emptyTabWidget = tb->count() == 0;
		if(emptyTabWidget && m_stackWidget->count() == 3) { // only the two separators
			openCoreBinIn(tb);
		} else if(emptyTabWidget) {
			m_stackWidget->removeWidget(tb);
		}
	}
}

PartsBinPaletteWidget* BinManager::getBin(StackTabWidget* tb, int index) {
	return dynamic_cast<PartsBinPaletteWidget*>(tb->widget(index));
}

PartsBinPaletteWidget* BinManager::currentBin(StackTabWidget* tb) {
	return getBin(tb, tb->currentIndex());
}

void BinManager::updateFileName(PartsBinPaletteWidget* bin, const QString &newFileName, const QString &oldFilename) {
	m_openedBins.remove(oldFilename);
	m_openedBins[newFileName] = bin;
}

void BinManager::saveStateAndGeometry() {
	QSettings settings;
	settings.remove("bins"); // clean up previous state
	settings.beginGroup("bins");
	for(int i=m_stackWidget->count()-1; i >= 0; i--) {
		StackTabWidget *tw = dynamic_cast<StackTabWidget*>(m_stackWidget->widget(i));
		if(tw) {
			bool groupBegan = false;
			for(int j=tw->count()-1; j >= 0; j--) {
				PartsBinPaletteWidget *bin = dynamic_cast<PartsBinPaletteWidget*>(tw->widget(j));
				if(bin) {
					if(!groupBegan) {
						settings.beginGroup(QString("%1").arg(i));
						groupBegan = true;
					}
					settings.setValue(QString("%1").arg(j),bin->fileName());
				}
			}
			if(groupBegan) {
				settings.endGroup();
			}
		}
	}
	settings.endGroup();
}

void BinManager::restoreStateAndGeometry() {
	QSettings settings;
	settings.beginGroup("bins");
	if(settings.childGroups().size()==0) { // first time? open core and my_parts then
		StackTabWidget *tw = new StackTabWidget(m_stackWidget);

		PartsBinPaletteWidget* core = newBin();
		core->openCore();
		tw->addTab(core,core->title());
		registerBin(core,tw);

		PartsBinPaletteWidget* myParts = newBin();
		myParts->open(MyPartsBinLocation);
		tw->addTab(myParts,myParts->title());
		registerBin(myParts,tw);

		m_stackWidget->addWidget(tw);
	} else {
		foreach(QString g, settings.childGroups()) {
			settings.beginGroup(g);

			StackTabWidget *tw = new StackTabWidget(m_stackWidget);
			foreach(QString k, settings.childKeys()) {
				PartsBinPaletteWidget* bin = newBin();
				QString filename = settings.value(k).toString();
				if(QFileInfo(filename).exists() && bin->open(filename)) {
					bin->setTabWidget(tw);
					tw->addTab(bin,bin->title());
					registerBin(bin,tw);
				} else {
					delete bin;
				}
			}
			m_stackWidget->addWidget(tw);

			settings.endGroup();
		}
	}
}

void BinManager::tabCloseRequested(StackTabWidget* tw, int index) {
	closeBinIn(tw,index);
}

void BinManager::addPartTo(PartsBinPaletteWidget* bin, ModelPart* mp) {
	if(mp) {
		bool alreadyIn = bin->contains(mp->moduleID());
		bin->addPart(mp);
		if(!alreadyIn) {
			setDirtyTab(bin,true);
		}
	}
}

void BinManager::newPartTo(PartsBinPaletteWidget* bin) {
	PartsEditorMainWindow *partsEditor = m_mainWindow->getPartsEditor(NULL, -1, NULL, bin);
	partsEditor->show();
	partsEditor->raise();
}

void BinManager::importPartTo(PartsBinPaletteWidget* bin) {
	QString newPartPath = QFileDialog::getOpenFileName(
		this,
		tr("Select a part to import"),
		"",
		tr("External Part (*%1)").arg(FritzingBundledPartExtension)
	);
	if(!newPartPath.isEmpty() && !newPartPath.isNull()) {
		ModelPart *mp = m_mainWindow->loadBundledPart(newPartPath,!bin->allowsChanges());
		if (bin->allowsChanges()) {
			addPartTo(bin,mp);
		}
	}
}

void BinManager::editSelectedPartFrom(PartsBinPaletteWidget* bin) {
	ModelPart *selectedMP = bin->selected();
	PartsEditorMainWindow *partsEditor = m_mainWindow->getPartsEditor(selectedMP, -1, NULL, bin);
	partsEditor->show();
	partsEditor->raise();
}

void BinManager::dockedInto(FDockWidget* dock) {
	m_stackWidget->setDock(dock);
}

bool BinManager::isTabReorderingEvent(QDropEvent* event) {
	const QMimeData *m = event->mimeData();
	QStringList formats = m->formats();
	return formats.contains("action") && (m->data("action") == "tab-reordering");
}

const QString &BinManager::getSelectedModuleIDFromSketch() {
	return m_mainWindow->selectedModuleID();
}

QList<QAction*> BinManager::openedBinsActions(const QString &moduleId) {
	QMap<QString,QAction*> titlesAndActions; // QMap sorts values by key
	QList<PartsBinPaletteWidget*> bins = m_tabWidgets.keys();

	foreach(PartsBinPaletteWidget* b, bins) {
		QAction *act = b->addPartToMeAction();
		act->setEnabled(!b->contains(moduleId));
		titlesAndActions[b->title()] = act;
	}

	return titlesAndActions.values();
}

void BinManager::openBin(const QString &filename) {
	m_tabWidgets.keys()[0]->openNewBin(filename);
}

MainWindow* BinManager::mainWindow() {
	return m_mainWindow;
}

void BinManager::initNames() {
    BinManager::MyPartsBinLocation = FolderUtils::getUserDataStorePath("bins")+"/my_parts.fzb";
    BinManager::MyPartsBinTemplateLocation =":/resources/bins/my_parts.fzb";
    BinManager::SearchBinLocation = FolderUtils::getUserDataStorePath("bins")+"/search.fzb";
    BinManager::SearchBinTemplateLocation =":/resources/bins/search.fzb";
    BinManager::AllPartsBinLocation = FolderUtils::getApplicationSubFolderPath("bins")+"/allParts.fzb";
	BinManager::NonCorePartsBinLocation = FolderUtils::getApplicationSubFolderPath("bins")+"/nonCoreParts.fzb";
	BinManager::ContribPartsBinLocation = FolderUtils::getApplicationSubFolderPath("bins")+"/contribParts.fzb";
    BinManager::CorePartsBinLocation = ":/resources/bins/bin.fzb";
}

void BinManager::search(const QString & searchText) {
    PartsBinPaletteWidget * searchBin = getOrOpenSearchBin();
    if (searchBin == NULL) return;

    QList<ModelPart *> modelParts = m_refModel->search(searchText, false);

    searchBin->removeParts();
    foreach (ModelPart * modelPart, modelParts) {
        DebugDialog::debug(modelPart->title());
        this->addPartTo(searchBin, modelPart);
    }

    setDirtyTab(searchBin);
}

PartsBinPaletteWidget * BinManager::clickedSearch(PartsBinPaletteWidget * bin) {
    PartsBinPaletteWidget * searchBin = getOrOpenSearchBin();
    if (searchBin == NULL) return NULL;
	if (searchBin == bin) {
		searchBin->focusSearch();
		return searchBin;
	}

    setAsCurrentTab(searchBin);
	searchBin->focusSearch();
    return searchBin;
}
