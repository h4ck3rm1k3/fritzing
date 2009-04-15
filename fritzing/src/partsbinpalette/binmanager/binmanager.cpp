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

BinManager::BinManager(class ReferenceModel *refModel, class HtmlInfoView *infoView, WaitPushUndoStack *undoStack, QWidget* parent)
	: QFrame(parent)
{
	BinManager::Title = tr("Parts");

	m_refModel = refModel;
	m_infoView = infoView;
	m_undoStack = undoStack;

	m_widget = new StackWidget(this);
	m_widget->setAcceptDrops(true);
	//m_activeBinTabWidget = new QTabWidget(m_widget);

	m_unsavedBins = 0;

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
	tb->setTabsClosable(true);
	//tb->setMovable(true);
#endif
	m_widget->addWidget(tb);
	m_tabWidgets[bin] = tb;
}

void BinManager::insertBin(PartsBinPaletteWidget* bin, int index, StackTabWidget* tb) {
	tb->insertTab(index,bin,bin->title());
	tb->setCurrentIndex(index);
	m_tabWidgets[bin] = tb;
}

void BinManager::loadFromModel(PaletteModel *model) {
	PartsBinPaletteWidget* bin = new PartsBinPaletteWidget(m_refModel,m_infoView,m_undoStack,this);
	bin->loadFromModel(model);
	addBin(bin);
}

void BinManager::setPaletteModel(PaletteModel *model, bool clear) {

}


bool BinManager::beforeClosing() {
	return true;
}


bool BinManager::hasAlienParts() {
	return false;

}

void BinManager::saveAndCreateNewBinIfCore() {

}

void BinManager::setInfoViewOnHover(bool infoViewOnHover) {

}

void BinManager::addNewPart(ModelPart *modelPart) {

}

void BinManager::addPart(ModelPart *modelPart, int position) {

}

void BinManager::load(const QString&) {

}

void BinManager::addPartCommand(const QString& moduleID) {

}

void BinManager::removeAlienParts() {

}

void BinManager::setDirtyTab(QWidget* w, bool dirty) {
	QTabWidget* tw = m_tabWidgets[w];
	if(tw) {
		int tabIdx = tw->indexOf(w);
		tw->setTabText(tabIdx, tw->tabText(tabIdx)+(dirty? " (*)": ""));
	}
}

void BinManager::updateTitle(QWidget* w, const QString& newTitle) {
	QTabWidget* tw = m_tabWidgets[w];
	if(tw) tw->setTabText(tw->indexOf(w), newTitle+" (*)");
}

void BinManager::newBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget* bin = new PartsBinPaletteWidget(m_refModel,m_infoView,m_undoStack,this);
	bin->setTitle(tr("New bin (%1)").arg(++m_unsavedBins));
	insertBin(bin, tb->currentIndex(), tb);
}

void BinManager::openBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget* bin = new PartsBinPaletteWidget(m_refModel,m_infoView,m_undoStack,this);
	if(bin->open()) {
		insertBin(bin, tb->currentIndex(), tb);
	}
}

void BinManager::openCoreBinIn(StackTabWidget* tb) {
	PartsBinPaletteWidget* bin = new PartsBinPaletteWidget(m_refModel,m_infoView,m_undoStack,this);
	bin->openCore();
	insertBin(bin, tb->currentIndex(), tb);
}

void BinManager::closeBinIn(StackTabWidget* tb) {
	tb->removeTab(tb->currentIndex());
	if(tb->count() == 0 && m_widget->count() <= 3) { // one tab widget and two separators
		openCoreBinIn(tb);
	}
}
