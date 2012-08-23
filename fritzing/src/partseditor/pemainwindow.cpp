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

/* TO DO ******************************************

	clean up menus

	add connectors tab

	underlying data structure

	first time help?

	disable dragging wires

	icon view

	change pin count

	add status bar to connectors tab

    on svg import detect all connector IDs
        if any are invisible, tell user this is obsolete

    allow user to select connectors by driving through svg elements

    eagle lbr
    eagle brd
    kicad footprint and mod?
    gEDA footprint

    for schematic view 
        offer generate option
        offer pins, rects (or lines), and a selection of standard schematic icons
        text?

    for breadboard view
        import 
        generate ICs, dips, sips, breakouts

    for pcb view
        pads, pins (circle, rect, oblong)
        lines and curves?
        import silkscreen
        text?

    allow but discourage png imports

    for svg import check for flaws:
        internal coords
        corel draw not saved for presentation
        inkscape not saved as plain
        inkscape scaling?
        illustrator px
        <gradient>, <pattern>, <marker>, <tspan>, etc.

    holes

    smd vs. tht

    buses

    bendable legs

    flip and rotate

    terminal points

    undo/redo as xml file: use index + guid for uniqueness

    nudge via arrow keys

    taxonomy?

    new schematic layout specs

    deal with modified svgs (pin labels, chip label, pin size, etc...)

    disable family entry?  should always be "custom_"  + original family (unless it already starts with custom_)


***************************************************/

#include "pemainwindow.h"
#include "metadataview.h"
#include "../debugdialog.h"
#include "../model/palettemodel.h"
#include "../sketch/breadboardsketchwidget.h"
#include "../sketch/schematicsketchwidget.h"
#include "../sketch/pcbsketchwidget.h"
#include "../sketchareawidget.h"
#include "../referencemodel/referencemodel.h"


#ifdef QT_NO_DEBUG
	#define CORE_EDITION_ENABLED false
#else
	#define CORE_EDITION_ENABLED false
#endif

////////////////////////////////////////////////////

IconSketchWidget::IconSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : SketchWidget(viewIdentifier, parent)
{
	m_shortName = QObject::tr("ii");
	m_viewName = QObject::tr("Icon View");
	initBackgroundColor();
}

void IconSketchWidget::addViewLayers() {
	addIconViewLayers();
}

/////////////////////////////////////////////////////

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

	m_iconGraphicsView = new IconSketchWidget(ViewIdentifierClass::IconView, this);
	initSketchWidget(m_iconGraphicsView);
	m_iconWidget = new SketchAreaWidget(m_iconGraphicsView,this);
	m_tabWidget->addWidget(m_iconWidget);
    initSketchWidget(m_iconGraphicsView);

    m_metadataView = new MetadataView(this);
	SketchAreaWidget * sketchAreaWidget = new SketchAreaWidget(m_metadataView, this);
	m_tabWidget->addWidget(sketchAreaWidget);
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

void PEMainWindow::setInitialItem(PaletteItem * paletteItem) {
    ModelPart * modelPart = NULL;

    if (paletteItem == NULL) {
        modelPart = m_paletteModel->retrieveModelPart("generic_ic_dip_8_300mil");
    }
    else {
        modelPart = paletteItem->modelPart();
    }

    long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));

    m_iconGraphicsView->addItem(modelPart, m_iconGraphicsView->defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
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
    m_iconGraphicsView->fitInWindow();
}

void PEMainWindow::setTitle() {
    setWindowTitle(tr("New Parts Editor"));
}

void PEMainWindow::createViewMenuActions() {
    MainWindow::createViewMenuActions();

	m_showIconAct = new QAction(tr("Show Icon"), this);
	m_showIconAct->setShortcut(tr("Ctrl+4"));
	m_showIconAct->setStatusTip(tr("Show the icon view"));
	connect(m_showIconAct, SIGNAL(triggered()), this, SLOT(showIconView()));

	m_showMetadataAct = new QAction(tr("Show Metatdata"), this);
	m_showMetadataAct->setShortcut(tr("Ctrl+5"));
	m_showMetadataAct->setStatusTip(tr("Show the metadata view"));
	connect(m_showMetadataAct, SIGNAL(triggered()), this, SLOT(showMetadataView()));
}

void PEMainWindow::createViewMenu() {
    MainWindow::createViewMenu();

    bool afterNext = false;
    foreach (QAction * action, m_viewMenu->actions()) {
        if (action == m_showPCBAct) {
            afterNext = true;
        }
        else if (afterNext) {
            m_viewMenu->insertAction(action, m_showIconAct);
            m_viewMenu->insertAction(action, m_showMetadataAct);
            break;
        }
    }
}

void PEMainWindow::showMetadataView() {
    this->m_tabWidget->setCurrentIndex(4);
}

void PEMainWindow::showIconView() {
    this->m_tabWidget->setCurrentIndex(3);
}
