/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 6319 $:
$Author: cohen@irascible.com $:
$Date: 2012-08-20 14:36:09 +0200 (Mon, 20 Aug 2012) $

********************************************************************/


#include "pemainwindow.h"
#include "../debugdialog.h"
#include "../model/palettemodel.h"
#include "../sketch/breadboardsketchwidget.h"
#include "../sketch/schematicsketchwidget.h"
#include "../sketch/pcbsketchwidget.h"
#include "../referencemodel/referencemodel.h"


#ifdef QT_NO_DEBUG
	#define CORE_EDITION_ENABLED false
#else
	#define CORE_EDITION_ENABLED false
#endif


PEMainWindow::PEMainWindow(PaletteModel * paletteModel, ReferenceModel * referenceModel, QWidget * parent)
	: MainWindow(paletteModel, referenceModel, parent)
{
    m_settingsPrefix = "pe/";
}

PEMainWindow::~PEMainWindow()
{
}

void PEMainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
}

void PEMainWindow::initLockedFiles(bool) {
}

void PEMainWindow::initSketchWidgets()
{
    MainWindow::initSketchWidgets();
}

void PEMainWindow::initDock()
{
}

void PEMainWindow::moreInitDock()
{
}

void PEMainWindow::createActions()
{
    createFileMenuActions();
    createEditMenuActions();
    createViewMenuActions();
    createHelpMenuActions();
}

void PEMainWindow::createMenus()
{
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createHelpMenu();
}

QList<QWidget*> PEMainWindow::getButtonsForView(ViewIdentifierClass::ViewIdentifier) {
	QList<QWidget*> retval;
    return retval;
}

void PEMainWindow::connectPairs() {
}

QMenu *PEMainWindow::breadboardWireMenu() {
    return NULL;
}
    
QMenu *PEMainWindow::breadboardItemMenu() {
    return NULL;
}

QMenu *PEMainWindow::schematicWireMenu() {
    return NULL;
}
    
QMenu *PEMainWindow::schematicItemMenu() {
    return NULL;
}

QMenu *PEMainWindow::pcbWireMenu() {
    return NULL;
}
    
QMenu *PEMainWindow::pcbItemMenu() {
    return NULL;
}

void PEMainWindow::setInitialModuleID(const QString & moduleID) {

    long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));

    ModelPart * modelPart = m_paletteModel->retrieveModelPart(moduleID);
    m_breadboardGraphicsView->addItem(modelPart, m_breadboardGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    m_schematicGraphicsView->addItem(modelPart, m_schematicGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    m_pcbGraphicsView->addItem(modelPart, m_pcbGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
    QTimer::singleShot(10, this, SLOT(initZoom()));
}

bool PEMainWindow::eventFilter(QObject *object, QEvent *event) 
{
	return QMainWindow::eventFilter(object, event);
}

void PEMainWindow::initHelper()
{
}

void PEMainWindow::initZoom() {
    m_breadboardGraphicsView->fitInWindow();
    m_schematicGraphicsView->fitInWindow();
    m_pcbGraphicsView->fitInWindow();
}
