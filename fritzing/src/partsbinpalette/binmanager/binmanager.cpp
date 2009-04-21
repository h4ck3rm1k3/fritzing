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


#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

#include "binmanager.h"
#include "stacktabwidget.h"
#include "../partsbinpalettewidget.h"
#include "../../modelpart.h"
#include "../../palettemodel.h"
#include "../../waitpushundostack.h"
#include "../../debugdialog.h"


QString BinManager::Title;
QString BinManager::MyPartsBinLocation;

BinManager::BinManager(class ReferenceModel *refModel, class HtmlInfoView *infoView, WaitPushUndoStack *undoStack, QWidget* parent)
	: QFrame(parent)
{
	BinManager::Title = tr("Parts");

	m_refModel = refModel;
	m_paletteModel = NULL;
	m_infoView = infoView;
	m_undoStack = undoStack;
	m_defaultSaveFolder = getApplicationSubFolderPath("bins");

	m_widget = new StackWidget(this);
	m_widget->setAcceptDrops(true);
	//m_activeBinTabWidget = new QTabWidget(m_widget);

	m_unsavedBinsCount = 0;

	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->addWidget(m_widget);
	setMaximumHeight(500);
}

BinManager::~BinManager() {

}

void BinManager::addBin(PartsBinPaletteWidget* bin) {
	StackTabWidget *tb = new StackTabWidget(m_widget);
	bin->setTabWidget(tb);
	tb->addTab(bin,bin->title());
	// this functions are only available on 4.5.0 or later
#if QT_VERSION >= 0x040500
	//tb->setTabsClosable(true);
	//tb->setMovable(true);
#endif
	m_widget->addWidget(tb);
	m_tabWidgets[bin] = tb;
	if(bin->fileName() != ___emptyString___) {
		m_openedBins[bin->fileName()] = bin;
	}
}

void BinManager::insertBin(PartsBinPaletteWidget* bin, int index, StackTabWidget* tb) {
	bin->setTabWidget(tb);
	tb->insertTab(index,bin,bin->title());
	tb->setCurrentIndex(index);
	m_tabWidgets[bin] = tb;
}

void BinManager::loadFromModel(PaletteModel *model) {
	PartsBinPaletteWidget* bin = newBin();
	m_paletteModel=model;
	bin->loadFromModel(model);
	addBin(bin);
}

void BinManager::setPaletteModel(PaletteModel *model) {
	m_paletteModel = model;
}


bool BinManager::beforeClosing() {
	return true;
}


bool BinManager::hasAlienParts() {
	return false;

}

void BinManager::setInfoViewOnHover(bool infoViewOnHover) {
	Q_UNUSED(infoViewOnHover);
}

void BinManager::addNewPart(ModelPart *modelPart) {
	QList<PartsBinPaletteWidget*> myPartsBins;
	foreach(PartsBinPaletteWidget* bin, m_tabWidgets.keys()) {
		if(bin->fileName() == MyPartsBinLocation) {
			myPartsBins << bin;
		}
	}

	if(myPartsBins.isEmpty()) {
		QString fileToOpen = QFileInfo(MyPartsBinLocation).exists()?
			MyPartsBinLocation:
			createIfMyPartsNotExists();

		PartsBinPaletteWidget* bin = openBinIn(m_tabWidgets.values()[0], fileToOpen);
		bin->addPart(modelPart);
		setDirtyTab(bin);
	} else {
		foreach(PartsBinPaletteWidget* bin, myPartsBins) {
			bin->addPart(modelPart);
			setDirtyTab(bin);
		}
	}
}

QString BinManager::createIfMyPartsNotExists() {
	QDateTime now = QDateTime::currentDateTime();
	/*QString binPath = getApplicationSubFolderPath("bins")+
		QString("/my_parts_%1.fzb").arg(now.toString("yyyy-MM-dd_hh-mm-ss"));*/
	QString binPath = MyPartsBinLocation;
	QFile file(":/resources/bins/my_parts.fzb");
	file.copy(binPath);
	return binPath;
}

void BinManager::addPart(ModelPart *modelPart, int position) {
	Q_UNUSED(modelPart);
	Q_UNUSED(position);
}

void BinManager::load(const QString& filename) {
	Q_UNUSED(filename);
}

void BinManager::addPartCommand(const QString& moduleID) {
	Q_UNUSED(moduleID);
}

void BinManager::removeAlienParts() {

}

void BinManager::setDirtyTab(PartsBinPaletteWidget* w, bool dirty) {
	w->setWindowModified(dirty);
	QTabWidget* tw = m_tabWidgets[w];
	Q_ASSERT(tw);
	int tabIdx = tw->indexOf(w);
	tw->setTabText(tabIdx, w->title()+(dirty? " (*)": ""));
}

void BinManager::updateTitle(PartsBinPaletteWidget* w, const QString& newTitle) {
	QTabWidget* tw = m_tabWidgets[w];
	if(tw) {
		tw->setTabText(tw->indexOf(w), newTitle+" (*)");
		setDirtyTab(w);
	}
}

PartsBinPaletteWidget* BinManager::newBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget* bin = newBin();
	bin->setPaletteModel(new PaletteModel(true,false),true);
	bin->setTitle(tr("New bin (%1)").arg(++m_unsavedBinsCount));
	insertBin(bin, tb->currentIndex(), tb);
	return bin;
}

PartsBinPaletteWidget* BinManager::openBinIn(StackTabWidget* tb, QString fileName) {
	if(fileName.isNull() || fileName.isEmpty()) {
		fileName = QFileDialog::getOpenFileName(
				this,
				tr("Select a Fritzing file to open"),
				m_defaultSaveFolder,
				tr("Fritzing (*%1)").arg(FritzingBinExtension) );
		if (fileName.isNull()) return false;
	}
	PartsBinPaletteWidget* bin = NULL;
	if(m_openedBins.contains(fileName)) {
		bin = m_openedBins[fileName];
		m_tabWidgets[bin]->setCurrentWidget(bin);
	} else {
		bin = newBin();
		if(bin->open(fileName)) {
			m_openedBins[fileName] = bin;
			insertBin(bin, tb->currentIndex(), tb);
		}
	}
	return bin;
}

PartsBinPaletteWidget* BinManager::openCoreBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget* bin = newBin();
	bin->openCore();
	insertBin(bin, tb->currentIndex(), tb);
	return bin;
}

PartsBinPaletteWidget* BinManager::newBin() {
	PartsBinPaletteWidget* bin = new PartsBinPaletteWidget(m_refModel,m_infoView,m_undoStack,this);
	connect(
		bin, SIGNAL(fileNameUpdated(PartsBinPaletteWidget*, const QString&, const QString&)),
		this, SLOT(updateFileName(PartsBinPaletteWidget*, const QString&, const QString&))
	);
	return bin;
}

void BinManager::closeBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget *w = currentBin(tb);
	if(w && w->beforeClosing()) {
		tb->removeTab(tb->currentIndex());
		m_tabWidgets.remove(w);
		m_openedBins.remove(w->fileName());
		if(tb->count() == 0 && m_widget->count() <= 3) { // one tab widget and two separators
			openCoreBinIn(tb);
		}
	}
}

PartsBinPaletteWidget* BinManager::currentBin(StackTabWidget* tb) {
	return dynamic_cast<PartsBinPaletteWidget*>(tb->currentWidget());
}

void BinManager::updateFileName(PartsBinPaletteWidget* bin, const QString &newFileName, const QString &oldFilename) {
	m_openedBins.remove(oldFilename);
	m_openedBins[newFileName] = bin;
}
