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



#include <QtGui>
#include <QtXml>
#include <QList>
#include <QFileInfo>
#include <QStringList>
#include <QFileInfoList>
#include <QDir>

#include <QTime>
#include <QCleanlooksStyle>
#include <QDir>
#include <QSettings>
#include <QRegExpValidator>
#include <QRegExp>
#include <QPaintDevice>
#include <QPixmap>

#include "paletteitem.h"
#include "mainwindow.h"
#include "debugdialog.h"
#include "connector.h"
#include "partseditor/mainpartseditorwindow.h"
#include "fdockwidget.h"
#include "htmlinfoview.h"
#include "waitpushundostack.h"
#include "fapplication.h"


const QString MainWindow::UntitledSketchName = "Untitled Sketch";
int MainWindow::UntitledSketchIndex = 1;
qreal MainWindow::m_printerScale = 1;
int MainWindow::CascadeFactor = 40;

static QString dockUnselectedColor = "QDockWidget#%1 { color: rgb(80, 80, 80); }";
static QString dockSelectedColor = "QDockWidget#%1 { color: rgb(84,24,44); }";
static QString clickToSeeString = QObject::tr("click to see %1");

MainWindow::MainWindow(PaletteModel * paletteModel, ReferenceModel *refModel) :
	FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension())
{
	QFile styleSheet(":/resources/styles/fritzing.qss");

	resize(740,500);

	// Create dot icons
	m_dotIcon = QIcon(":/resources/images/dot.png");
	m_emptyIcon = QIcon();

	m_currentWidget = NULL;
	m_firstOpen = true;

	setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_WS_MAC
	setAttribute(Qt::WA_QuitOnClose, false);
#endif
    m_dontClose = m_closing = false;
	m_savedState = NeverSaved;

	m_paletteModel = paletteModel;
	m_refModel = refModel;
	m_sketchModel = new SketchModel(true);

	m_tabWidget = new QStackedWidget(this); //   FTabWidget(this);
	m_tabWidget->setObjectName("sketch_tabs");
	setCentralWidget(m_tabWidget);


	// all this belongs in viewLayer.xml
	m_breadboardGraphicsView = new BreadboardSketchWidget(ItemBase::BreadboardView, this);
	initSketchWidget(m_breadboardGraphicsView);
	m_breadboardWidget = new SketchAreaWidget(m_breadboardGraphicsView,this);
	//m_tabWidget->addTab(m_breadboardWidget, tr("breadboard"));
	m_tabWidget->addWidget(m_breadboardWidget);

	m_schematicGraphicsView = new SchematicSketchWidget(ItemBase::SchematicView, this);
	initSketchWidget(m_schematicGraphicsView);
	m_schematicWidget = new SketchAreaWidget(m_schematicGraphicsView, this);
	//m_tabWidget->addTab(m_schematicWidget, tr("schematic"));
	m_tabWidget->addWidget(m_schematicWidget);

	m_pcbGraphicsView = new PCBSketchWidget(ItemBase::PCBView, this);
	initSketchWidget(m_pcbGraphicsView);
	m_pcbWidget = new SketchAreaWidget(m_pcbGraphicsView, this);
	//m_tabWidget->addTab(m_pcbWidget, tr("pcb"));
	m_tabWidget->addWidget(m_pcbWidget);

    m_undoView = new QUndoView();
    m_undoGroup = new QUndoGroup(this);
    m_undoView->setGroup(m_undoGroup);
    m_undoGroup->setActiveStack(m_breadboardGraphicsView->undoStack());

    createBinAndInfoViewDocks();
    createActions();
    createSketchButtons();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();

    m_breadboardWidget->setContent(
    	getButtonsForView(m_breadboardWidget->viewIdentifier()),
    	createZoomOptions(m_breadboardWidget)
    );
    m_schematicWidget->setContent(
    	getButtonsForView(m_schematicWidget->viewIdentifier()),
    	createZoomOptions(m_schematicWidget)
    );
	m_pcbWidget->setContent(
		getButtonsForView(m_pcbWidget->viewIdentifier()),
		createZoomOptions(m_pcbWidget)
	);


    if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/fritzing.qss");
	} else {
		/*
#ifdef Q_WS_WIN
		int marginTop = m_toolbar->height() */ /* - m_tabWidget->tabBar()->height() */ // + 4;
/*#else
		int marginTop = m_toolbar->height() */ /* - m_tabWidget->tabBar()->height() */ //+ 2;
/*#endif
		QString tabbarStyle =
			QString("QToolBar QTabBar::tab {margin-top: %1px;} ").arg(marginTop)+
			QString("#sketch_tabs::pane {top: -%2px;}").arg(8);*/
		setStyleSheet(styleSheet.readAll()+___MacStyle___);
	}

	m_itemMenu = new QMenu(QObject::tr("Part"), this);
	m_itemMenu->addAction(m_rotate90cwAct);
	m_itemMenu->addAction(m_rotate180Act);
	m_itemMenu->addAction(m_rotate90ccwAct);
	m_itemMenu->addAction(m_flipHorizontalAct);
	m_itemMenu->addAction(m_flipVerticalAct);
	m_itemMenu->addAction(m_bringToFrontAct);
	m_itemMenu->addAction(m_bringForwardAct);
	m_itemMenu->addAction(m_sendBackwardAct);
	m_itemMenu->addAction(m_sendToBackAct);
	m_itemMenu->addAction(m_openInPartsEditorAct);
	m_itemMenu->addAction(m_deleteAct);
	m_itemMenu->addAction(m_addToBinAct);

	QMenu *breadItemMenu = breadboardItemMenu();
    m_breadboardGraphicsView->setItemMenu(breadItemMenu);
    m_breadboardGraphicsView->setWireMenu(breadItemMenu);

    m_pcbGraphicsView->setWireMenu(pcbWireMenu());
    m_pcbGraphicsView->setItemMenu(pcbItemMenu());

    QMenu *schemItemMenu = schematicItemMenu();
    m_schematicGraphicsView->setItemMenu(schemItemMenu);
    m_schematicGraphicsView->setWireMenu(schemItemMenu);


    m_breadboardGraphicsView->setInfoView(m_infoView);
    m_pcbGraphicsView->setInfoView(m_infoView);
    m_schematicGraphicsView->setInfoView(m_infoView);

    m_breadboardGraphicsView->setBackground(QColor(204,204,204));
    m_schematicGraphicsView->setBackground(QColor(255,255,255));
    m_pcbGraphicsView->setBackground(QColor(0x8F, 0xBC, 0x8F));							// QColor(137,144,153)

	// make sure to set the connections after the views have been created
	connect(m_tabWidget, SIGNAL(currentChanged ( int )),
			this, SLOT(tabWidget_currentChanged( int )));

	connectPairs();

	// do this the first time, since the current_changed signal wasn't sent
	m_currentWidget = NULL;
	int tab = 0;
	currentNavigatorChanged(m_navigators[tab]);
	tabWidget_currentChanged(tab);

	this->installEventFilter(this);

	m_comboboxChanged = false;

	QSettings settings("Fritzing","Fritzing");
#ifndef QT_NO_DEBUG
	settings.clear();
#endif
	if(!settings.value("main/state").isNull()) {
		restoreState(settings.value("main/state").toByteArray());
		restoreGeometry(settings.value("main/geometry").toByteArray());
	}

	setMinimumSize(0,0);
	m_tabWidget->setMinimumWidth(500);
	m_tabWidget->setMinimumWidth(0);

	m_miniViewContainerBreadboard->setView(m_breadboardGraphicsView);
	m_miniViewContainerSchematic->setView(m_schematicGraphicsView);
	m_miniViewContainerPCB->setView(m_pcbGraphicsView);

	connect(this, SIGNAL(readOnlyChanged(bool)), this, SLOT(applyReadOnlyChange(bool)));
}

void MainWindow::initSketchWidget(SketchWidget * sketchWidget) {
	sketchWidget->setPaletteModel(m_paletteModel);
	sketchWidget->setSketchModel(m_sketchModel);
	sketchWidget->setRefModel(m_refModel);
	sketchWidget->setUndoStack(m_undoStack);
	sketchWidget->setChainDrag(true);			// enable bend points
	sketchWidget->addViewLayers();
}

void MainWindow::doOnce() {
	calcPrinterScale();
	preloadSlowParts();
}

void MainWindow::preloadSlowParts() {
	ViewGeometry viewGeometry;
	ItemBase * itemBase = m_breadboardGraphicsView->addItem(ItemBase::breadboardModuleIDName, BaseCommand::SingleView, viewGeometry, ItemBase::getNextID());
	if (itemBase == NULL) return;

	m_breadboardGraphicsView->deleteItem(itemBase, true, false);
}

void MainWindow::calcPrinterScale() {

	// note: I think that printerScale is probably just 90 dpi, since the calculation
	// result is 89.8407 across all three platforms

	m_printerScale = 90.0;
	return;

/*
	m_printerScale = 1;
	ViewGeometry viewGeometry;
	ItemBase * itemBase = m_breadboardGraphicsView->addItem(ItemBase::rulerModuleIDName, BaseCommand::SingleView, viewGeometry, ItemBase::getNextID());
	if (itemBase == NULL) return;

	QSize size = itemBase->size();
	QString filename = dynamic_cast<PaletteItemBase *>(itemBase)->filename();
	m_breadboardGraphicsView->deleteItem(itemBase, true, false);

	qreal width = getSvgWidthInInches(filename);
	if (width <= 0) return;

	m_printerScale = size.width() / width;
	DebugDialog::debug(QString("printerscale %1").arg(m_printerScale));
*/

}

qreal MainWindow::getSvgWidthInInches(const QString & filename)
{
	qreal result = 0;

	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return result;
	}

	result = getSvgWidthInInches(file);
	file.close();

	return result;
}

qreal MainWindow::getSvgWidthInInches(QFile & file)
{
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument* domDocument = new QDomDocument();

	if (!domDocument->setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return 0;
	}

	QDomElement root = domDocument->documentElement();
	if (root.isNull()) {
		return 0;
	}

	if (root.tagName() != "svg") {
		return 0;
	}

	QString stringWidth = root.attribute("width");
	if (stringWidth.isNull()) {
		return 0;
	}

	if (stringWidth.isEmpty()) {
		return 0;
	}

	bool ok;
	qreal result = convertToInches(stringWidth, &ok);
	if (!ok) return 0;

	return result;
}


void MainWindow::connectPairs() {
	connectPair(m_breadboardGraphicsView, m_schematicGraphicsView);
	connectPair(m_breadboardGraphicsView, m_pcbGraphicsView);
	connectPair(m_schematicGraphicsView, m_breadboardGraphicsView);
	connectPair(m_schematicGraphicsView, m_pcbGraphicsView);
	connectPair(m_pcbGraphicsView, m_breadboardGraphicsView);
	connectPair(m_pcbGraphicsView, m_schematicGraphicsView);

	bool succeeded = connect(m_breadboardGraphicsView, SIGNAL(findSketchWidgetSignal(ItemBase::ViewIdentifier, SketchWidget * &)),
							 this, SLOT(findSketchWidgetSlot(ItemBase::ViewIdentifier, SketchWidget * &)),
							 Qt::DirectConnection);

	succeeded = connect(m_schematicGraphicsView, SIGNAL(findSketchWidgetSignal(ItemBase::ViewIdentifier, SketchWidget * &)),
							 this, SLOT(findSketchWidgetSlot(ItemBase::ViewIdentifier, SketchWidget * &)),
							 Qt::DirectConnection);

	succeeded = connect(m_pcbGraphicsView, SIGNAL(findSketchWidgetSignal(ItemBase::ViewIdentifier, SketchWidget * &)),
							 this, SLOT(findSketchWidgetSlot(ItemBase::ViewIdentifier, SketchWidget * &)),
							 Qt::DirectConnection);

	succeeded = connect(m_pcbGraphicsView, SIGNAL(routingStatusSignal(int, int, int, int)),
						this, SLOT(routingStatusSlot(int, int, int, int)));

	succeeded = connect(m_pcbGraphicsView, SIGNAL(deletingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(deletingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_breadboardGraphicsView, SIGNAL(deletingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));

	succeeded = connect(m_pcbGraphicsView, SIGNAL(addingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(addingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_breadboardGraphicsView, SIGNAL(addingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));

	succeeded = connect(m_pcbGraphicsView, SIGNAL(changingConnectionSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(changingConnectionSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_breadboardGraphicsView, SIGNAL(changingConnectionSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));

	succeeded = connect(m_pcbGraphicsView, SIGNAL(rotatingFlippingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));

	succeeded = connect(m_pcbGraphicsView, SIGNAL(movingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));

	FApplication * fapp = dynamic_cast<FApplication *>(qApp);
	if (fapp != NULL) {
		succeeded = connect(fapp, SIGNAL(spaceBarIsPressedSignal(bool)), m_breadboardGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
		succeeded = connect(fapp, SIGNAL(spaceBarIsPressedSignal(bool)), m_schematicGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
		succeeded = connect(fapp, SIGNAL(spaceBarIsPressedSignal(bool)), m_pcbGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
	}
}

void MainWindow::connectPair(SketchWidget * signaller, SketchWidget * slotter)
{

	bool succeeded = connect(signaller, SIGNAL(itemAddedSignal(ModelPart *, const ViewGeometry & , long )),
							 slotter, SLOT(sketchWidget_itemAdded(ModelPart *, const ViewGeometry &, long)));

	succeeded = succeeded && connect(signaller, SIGNAL(itemDeletedSignal(long)),
									 slotter, SLOT(sketchWidget_itemDeleted(long)),
									 Qt::DirectConnection);

	succeeded = succeeded && connect(signaller, SIGNAL(clearSelectionSignal()),
									 slotter, SLOT(sketchWidget_clearSelection()));

	succeeded = succeeded && connect(signaller, SIGNAL(itemSelectedSignal(long, bool)),
									 slotter, SLOT(sketchWidget_itemSelected(long, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(selectAllItemsSignal(bool, bool)),
									 slotter, SLOT(selectAllItems(bool, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(tooltipAppliedToItem(long, const QString&)),
										 slotter, SLOT(sketchWidget_tooltipAppliedToItem(long, const QString&)));
	succeeded = succeeded && connect(signaller, SIGNAL(wireDisconnectedSignal(long, QString)),
									 slotter, SLOT(sketchWidget_wireDisconnected(long,  QString)));
	succeeded = succeeded && connect(signaller, SIGNAL(wireConnectedSignal(long,  QString, long,  QString)),
									 slotter, SLOT(sketchWidget_wireConnected(long, QString, long, QString)));
	succeeded = succeeded && connect(signaller, SIGNAL(changeConnectionSignal(long,  QString, long,  QString, bool, bool)),
									 slotter, SLOT(sketchWidget_changeConnection(long, QString, long, QString, bool, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(copyItemSignal(long, QHash<ItemBase::ViewIdentifier, ViewGeometry *> &)),
													   slotter, SLOT(sketchWidget_copyItem(long, QHash<ItemBase::ViewIdentifier, ViewGeometry *> &)),
									 Qt::DirectConnection);
	succeeded = succeeded && connect(signaller, SIGNAL(deleteItemSignal(long, QUndoCommand *)),
									 slotter, SLOT(sketchWidget_deleteItem(long, QUndoCommand *)),
									 Qt::DirectConnection);

	succeeded = succeeded && connect(signaller, SIGNAL(cleanUpWiresSignal()),
									 slotter, SLOT(sketchWidget_cleanUpWires()) );
	succeeded = succeeded && connect(signaller, SIGNAL(swapped(long, ModelPart*)),
									 slotter, SLOT(swap(long, ModelPart*)) );

	succeeded = succeeded && connect(signaller, SIGNAL(setChainedWireIDSignal(qint64, qint64)),
									 slotter, SLOT(setChainedWireIDSlot(qint64, qint64)) );


	if (!succeeded) {
		DebugDialog::debug("connectPair failed");
	}

}

void MainWindow::setCurrentFile(const QString &fileName, bool addToRecent) {
	m_fileName = fileName;
	setTitle();

	if(addToRecent) {
		QSettings settings("Fritzing","Fritzing");
		QStringList files = settings.value("recentFileList").toStringList();
		files.removeAll(fileName);
		files.prepend(fileName);
		while (files.size() > MaxRecentFiles)
			files.removeLast();

		settings.setValue("recentFileList", files);
	}

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}


ZoomComboBox *MainWindow::createZoomOptions(SketchAreaWidget* parent) {
	ZoomComboBox *zoomOptsComboBox = new ZoomComboBox(parent);

	setZoomComboBoxValue(parent->graphicsView()->currentZoom(), zoomOptsComboBox);
	connect(zoomOptsComboBox, SIGNAL(zoomChanged(qreal)), this, SLOT(updateViewZoom(qreal)));

    connect(parent->graphicsView(), SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));
    connect(parent->graphicsView(), SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));
	connect(parent->graphicsView(), SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));
	connect(parent->graphicsView(), SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));

	return zoomOptsComboBox;
}

void MainWindow::createToolBars() {
	/* TODO: Mariano this is too hacky and requires some styling
	 * around here and some else in the qss file
	 */
	/*m_toolbar = new QToolBar(this);
	m_toolbar->setObjectName("fake_tabbar");
	m_toolbar->setFloatable(false);
	m_toolbar->setMovable(false);
	int height = 0; //  m_tabWidget->tabBar()->height();
	m_toolbar->layout()->setMargin(0);
	m_toolbar->setFixedHeight(height+10);
	m_toolbar->setMinimumWidth(400); // connect to tabwidget resize event
	m_toolbar->toggleViewAction()->setVisible(false);
	// m_tabWidget->tabBar()->setParent(m_toolbar);
	addToolBar(m_toolbar);*/

	/*	QToolBar *tb2 = new QToolBar(this);
	tb2->setFloatable(false);
	tb2->setMovable(false);
	QToolButton *dummyButton = new QToolButton();
	dummyButton->setIcon(QIcon(":/resources/images/toolbar_icons/toolbarExport_pdf_icon.png"));
	tb2->addWidget(dummyButton);
	QToolButton *dummyButton2 = new QToolButton();
	dummyButton2->setIcon(QIcon(":/resources/images/toolbar_icons/toolbarOrder_icon.png"));
	tb2->addWidget(dummyButton2);
	addToolBar(tb2);*/

	/*
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->setObjectName("fileToolBar");
    m_fileToolBar->addAction(m_saveAct);
    m_fileToolBar->addAction(m_printAct);

    m_editToolBar = addToolBar(tr("Edit"));
    m_editToolBar->setObjectName("editToolBar");
    m_editToolBar->addAction(m_undoAct);
    m_editToolBar->addWidget(m_zoomOptsComboBox);
    */
}

void MainWindow::createSketchButtons() {
	m_routingStatusLabel = new ExpandingLabel(m_pcbWidget);
	m_routingStatusLabel->setObjectName(SketchAreaWidget::RoutingStateLabelName);

	routingStatusSlot(0,0,0,0);			// call this after the buttons have been created, because it calls updateTraceMenu
}

SketchToolButton *MainWindow::createRotateButton(SketchAreaWidget *parent) {
	QList<QAction*> rotateMenuActions;
	rotateMenuActions << m_rotate90ccwAct << m_rotate180Act << m_rotate90cwAct;
	SketchToolButton * rotateButton = new SketchToolButton("Rotate",parent, rotateMenuActions);
	rotateButton->setText(tr("Rotate"));
	connect(rotateButton, SIGNAL(menuUpdateNeeded()), this, SLOT(updateTransformationActions()));

	m_rotateButtons << rotateButton;
	return rotateButton;
}

SketchToolButton *MainWindow::createFlipButton(SketchAreaWidget *parent) {
	QList<QAction*> flipMenuActions;
	flipMenuActions << m_flipHorizontalAct << m_flipVerticalAct;
	SketchToolButton *flipButton = new SketchToolButton("Flip",parent, flipMenuActions);
	flipButton->setText(tr("Flip"));
	connect(flipButton, SIGNAL(menuUpdateNeeded()), this, SLOT(updateTransformationActions()));

	m_flipButtons << flipButton;
	return flipButton;
}

SketchToolButton *MainWindow::createAutorouteButton(SketchAreaWidget *parent) {
	SketchToolButton *autorouteButton = new SketchToolButton("Autoroute",parent, m_autorouteAct);
	autorouteButton->setText(tr("Autoroute"));

	return autorouteButton;
}

SketchToolButton *MainWindow::createExportDiyButton(SketchAreaWidget *parent) {
	SketchToolButton *exportDiyButton = new SketchToolButton("Diy",parent, m_exportDiyAct);
	exportDiyButton->setText(tr("DIY Etching"));

	return exportDiyButton;
}

QWidget *MainWindow::createToolbarSpacer(SketchAreaWidget *parent) {
	QFrame *toolbarSpacer = new QFrame(parent);
	QHBoxLayout *spacerLayout = new QHBoxLayout(toolbarSpacer);
	spacerLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));

	return toolbarSpacer;
}

QList<QWidget*> MainWindow::getButtonsForView(ItemBase::ViewIdentifier viewId) {
	QList<QWidget*> retval;
	SketchAreaWidget *parent;
	switch(viewId) {
		case ItemBase::BreadboardView: parent = m_breadboardWidget; break;
		case ItemBase::SchematicView: parent = m_schematicWidget; break;
		case ItemBase::PCBView: parent = m_pcbWidget; break;
		default: return retval;
	}
	/*if(viewId != ItemBase::PCBView) {
		retval << createExportToPdfButton(parent);
	}*/
	retval << createRotateButton(parent);
	if(viewId == ItemBase::BreadboardView) {
		retval << createFlipButton(parent);
	} else if(viewId == ItemBase::PCBView) {
		retval << SketchAreaWidget::separator(parent) << createAutorouteButton(parent)
			   << createExportDiyButton(parent) << m_routingStatusLabel;
	}

	if(viewId != ItemBase::PCBView) {
		retval << createToolbarSpacer(parent);
	}
	return retval;
}

void MainWindow::updateZoomOptions(qreal zoom) {
	if(!m_comboboxChanged) {
		setZoomComboBoxValue(zoom);
	} else {
		m_comboboxChanged = false;
	}
}

SketchAreaWidget *MainWindow::currentSketchArea() {
	return dynamic_cast<SketchAreaWidget*>(m_currentWidget->parent());
}

void MainWindow::updateZoomOptionsNoMatterWhat(qreal zoom) {
	currentSketchArea()->zoomComboBox()->setEditText(tr("%1%").arg(zoom));
}

void MainWindow::updateViewZoom(qreal newZoom) {
	m_comboboxChanged = true;
	if(m_currentWidget) m_currentWidget->absoluteZoom(newZoom);
}


void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::tabWidget_currentChanged(int index) {
	SketchAreaWidget * widgetParent = dynamic_cast<SketchAreaWidget *>(m_tabWidget->currentWidget());
	if (widgetParent == NULL) return;

	SketchWidget *widget = widgetParent->graphicsView();

	if(m_currentWidget) {
		disconnect(
			m_currentWidget,
			SIGNAL(selectionChangedSignal()),
			this,
			SLOT(updateTransformationActions())
		);
	}
	m_currentWidget = widget;
	if (widget == NULL) return;

	connect(
		m_currentWidget,					// don't connect directly to the scene here, connect to the widget's signal
		SIGNAL(selectionChangedSignal()),
		this,
		SLOT(updateTransformationActions())
	);

	//m_miniViewContainer0->setView(widget);

	//  TODO:  should be a cleaner way to do this
	switch( index ) {
		case 0 : setShowViewActionsIcons(m_showBreadboardAct, m_showSchematicAct,  m_showPCBAct); break;
		case 1 : setShowViewActionsIcons(m_showSchematicAct,  m_showBreadboardAct, m_showPCBAct); break;
		case 2 : setShowViewActionsIcons(m_showPCBAct,        m_showBreadboardAct, m_showSchematicAct); break;
		default :
			// Shouldn't get here
			DebugDialog::debug("Warning: not considered tab selected");
	}

	hideShowTraceMenu();
	updateTraceMenu();

	setZoomComboBoxValue(m_currentWidget->currentZoom());
}

void MainWindow::setShowViewActionsIcons(QAction * active, QAction * inactive1, QAction * inactive2) {
	active->setIcon(m_dotIcon);
	inactive1->setIcon(m_emptyIcon);
	inactive2->setIcon(m_emptyIcon);
}

void MainWindow::closeEvent(QCloseEvent *event) {
	if (m_dontClose) {
		event->ignore();
		return;
	}

	if(!beforeClosing() || !whatToDoWithAlienFiles() ||!m_paletteWidget->beforeClosing()) {
		event->ignore();
		return;
	}

	m_closing = true;
	emit aboutToClose();

	int count = 0;
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		if (widget == this) continue;
		if (dynamic_cast<QMainWindow *>(widget) == NULL) continue;
		if (dynamic_cast<MainPartsEditorWindow *>(widget) != NULL) continue;

		count++;
	}

	DebugDialog::debug(tr("current main windows: %1").arg(QApplication::topLevelWidgets().size()));

	if (count == 0) {
		DebugDialog::closeDebug();
	}

	QSettings settings("Fritzing","Fritzing");
	settings.setValue("main/state",saveState());
	settings.setValue("main/geometry",saveGeometry());

	QMainWindow::closeEvent(event);
}

bool MainWindow::whatToDoWithAlienFiles() {
	if (m_alienFiles.size() > 0) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, tr("Save %1").arg(QFileInfo(m_fileName).baseName()),
									 tr("Do you want to keep the parts that were loaded with this bundled sketch %1?")
									 .arg(QFileInfo(m_fileName).baseName()),
									 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (reply == QMessageBox::Yes) {
			return true;
		} else if (reply == QMessageBox::No) {
			foreach(QString pathToRemove, m_alienFiles) {
				QFile::remove(pathToRemove);
			}
			m_alienFiles.clear();
			recoverBackupedFiles();

			emit alienPartsDismissed();
			return true;
		}
		else {
			return false;
		}
	} else {
		return true;
	}
}


void MainWindow::setZoomComboBoxValue(qreal value, ZoomComboBox* zoomComboBox) {
	if(!zoomComboBox) zoomComboBox = currentSketchArea()->zoomComboBox();
	zoomComboBox->setEditText(tr("%1%").arg(value,0,'f',2));
}

void MainWindow::changeEvent ( QEvent * event ) {
	if (event) {
		if (event->type() == QEvent::ActivationChange && !m_closing) {
			//DebugDialog::debug(QObject::tr("change activation %1 %2").arg(m_savedState).arg((long) this));
			changeActivation(event);
		}
	}
	QMainWindow::changeEvent(event);
}

void MainWindow::changeActivation(QEvent *) {
	// tried using this->saveState() and this->restoreState() but couldn't get it to work

	QWidget * activeWindow = QApplication::activeWindow ();

	if (activeWindow == NULL) return;

	if (activeWindow == this || activeWindow->parent() == this) {
		if (m_savedState == Saved) {
			m_savedState = Restored;
			//DebugDialog::debug("restore state", this);
			//restoreState(m_savedStateData, 0);
			for (int i = 0; i < children().count(); i++) {
				FDockWidget * dock = dynamic_cast<FDockWidget *>(children()[i]);
				if (dock == NULL) continue;

				dock->restoreState();
			}
		}
	}
	else {
		if (!(m_savedState == Saved)) {
			//m_savedStateData = saveState(0);
			m_savedState = Saved;

			//DebugDialog::debug("save state", this);
			for (int i = 0; i < children().count(); i++) {
				FDockWidget * dock = dynamic_cast<FDockWidget *>(children()[i]);
				if (dock == NULL) continue;

				dock->saveState();

				if (dock->isFloating() && dock->isVisible()) {
					dock->hide();
				}
			}
		}
	}

}

void MainWindow::dockChangeActivation(FDockWidget *) {
	if (!m_closing) {
		changeActivation(NULL);
	}
}

void MainWindow::createBinAndInfoViewDocks() {
	m_infoView = new HtmlInfoView(m_refModel);

	m_paletteWidget = new PartsBinPaletteWidget(m_refModel, m_infoView, this);
	connect(m_paletteWidget, SIGNAL(saved(bool)), this, SLOT(binSaved(bool)));
	connect(this, SIGNAL(alienPartsDismissed()), m_paletteWidget, SLOT(removeAlienParts()));

	if (m_paletteModel->loadedFromFile()) {
		m_paletteWidget->loadFromModel(m_paletteModel);
	} else {
		m_paletteWidget->setPaletteModel(m_paletteModel);
	}
}

void MainWindow::createDockWindows()
{
	dockIt(m_paletteWidget, PartsBinMinHeight, PartsBinDefaultHeight);

    makeDock(tr("Part Inspector"), m_infoView, InfoViewMinHeight, InfoViewDefaultHeight);

    m_navigators << (m_miniViewContainerBreadboard = new MiniViewContainer(this));
    FDockWidget * dock = makeDock(tr("Breadboard view"), m_miniViewContainerBreadboard, NavigatorMinHeight, NavigatorDefaultHeight);
	dock->setObjectName("Breadboard_view");
	setDockColorAnd(m_miniViewContainerBreadboard, dockSelectedColor, ___emptyString___);
 	dock->setFeatures (QDockWidget::DockWidgetMovable);
	m_miniViewContainerBreadboard->filterMousePress();
	connect(m_miniViewContainerBreadboard, SIGNAL(navigatorMousePressedSignal(MiniViewContainer *)),
								this, SLOT(currentNavigatorChanged(MiniViewContainer *)));

    m_navigators << (m_miniViewContainerSchematic = new MiniViewContainer(this));
    dock = makeDock(tr("Schematic view"), m_miniViewContainerSchematic, NavigatorMinHeight, NavigatorDefaultHeight);
	dock->setObjectName("Schematic_view");
	setDockColorAnd(m_miniViewContainerSchematic, dockUnselectedColor, clickToSeeString);
 	dock->setFeatures (QDockWidget::DockWidgetMovable);
	m_miniViewContainerSchematic->filterMousePress();
	connect(m_miniViewContainerSchematic, SIGNAL(navigatorMousePressedSignal(MiniViewContainer *)),
								this, SLOT(currentNavigatorChanged(MiniViewContainer *)));

    m_navigators << (m_miniViewContainerPCB = new MiniViewContainer(this));
    dock = makeDock(tr("PCB view"), m_miniViewContainerPCB, NavigatorMinHeight, NavigatorDefaultHeight);
	dock->setObjectName("PCB_view");
	setDockColorAnd(m_miniViewContainerPCB, dockUnselectedColor, clickToSeeString);
	dock->setFeatures (QDockWidget::DockWidgetMovable);
	m_miniViewContainerPCB->filterMousePress();
	connect(m_miniViewContainerPCB, SIGNAL(navigatorMousePressedSignal(MiniViewContainer *)),
								this, SLOT(currentNavigatorChanged(MiniViewContainer *)));

    makeDock(tr("Undo History"), m_undoView, UndoHistoryMinHeight, UndoHistoryDefaultHeight)->hide();
    m_undoView->setMinimumSize(DockMinWidth, UndoHistoryMinHeight);

    m_consoleView = new Console();
    dock = makeDock(tr("Console"), m_consoleView, DockMinHeight, DockDefaultHeight, Qt::BottomDockWidgetArea);
	dock->hide();

    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_toggleDebuggerOutputAct);
    connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));
}

FDockWidget * MainWindow::makeDock(const QString & title, QWidget * widget, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area) {
    FDockWidget * dock = new FDockWidget(title, this);
    dock->setWidget(widget);
    widget->setParent(dock);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	return dockIt(dock, dockMinHeight, dockDefaultHeight, area);
}

FDockWidget *MainWindow::dockIt(FDockWidget* dock, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area) {
    //dock->setStyle(new QCleanlooksStyle());
	dock->setAllowedAreas(area);
    addDockWidget(area, dock);
    m_windowMenu->addAction(dock->toggleViewAction());

    dock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dock->setMinimumSize(DockMinWidth, dockMinHeight);
	dock->resize(DockDefaultWidth, dockDefaultHeight);
    connect(dock, SIGNAL(dockChangeActivationSignal(FDockWidget *)), this, SLOT(dockChangeActivation(class FDockWidget *)));

    return dock;
}


void MainWindow::loadPart(QString newPartPath) {
	ModelPart * modelPart = ((PaletteModel*)m_refModel)->addPart(newPartPath, true, true);
	if(modelPart && modelPart->isValid()) {
		//ModelPart * modelPart = m_paletteModel->addPart(newPartPath, true, true);
		m_paletteWidget->addPart(modelPart);
		m_infoView->reloadContent();
	}
}

bool MainWindow::eventFilter(QObject *object, QEvent *event) {
	if (object == this &&
		(event->type() == QEvent::KeyPress
		// || event->type() == QEvent::KeyRelease
		|| event->type() == QEvent::ShortcutOverride))
	{
		//DebugDialog::debug(QString("event filter %1").arg(event->type()) );
		updatePartMenu();
		updateTraceMenu();

		// On the mac, the first time the delete key is pressed, to be used as a shortcut for QAction m_deleteAct,
		// for some reason, the enabling of the m_deleteAct in UpdateEditMenu doesn't "take" until the next time the event loop is processed
		// Thereafter, the delete key works as it should.
		// So this call to processEvents() makes sure m_deleteAct is enabled.
		QCoreApplication::processEvents();
	}
	return QMainWindow::eventFilter(object, event);
}

void MainWindow::findSketchWidgetSlot(ItemBase::ViewIdentifier viewIdentifier, SketchWidget * & sketchWidget ) {
	if (m_breadboardGraphicsView->viewIdentifier() == viewIdentifier) {
		sketchWidget = m_breadboardGraphicsView;
		return;
	}

	if (m_schematicGraphicsView->viewIdentifier() == viewIdentifier) {
		sketchWidget = m_schematicGraphicsView;
		return;
	}

	if (m_pcbGraphicsView->viewIdentifier() == viewIdentifier) {
		sketchWidget = m_pcbGraphicsView;
		return;
	}

	sketchWidget = NULL;

}

const QString MainWindow::untitledFileName() {
	return UntitledSketchName;
}

int &MainWindow::untitledFileCount() {
	return UntitledSketchIndex;
}

const QString MainWindow::fileExtension() {
	return FritzingExtension;
}

const QString MainWindow::defaultSaveFolder() {
	return FApplication::openSaveFolder();
}

const QString & MainWindow::fileName() {
	return m_fileName;
}

bool MainWindow::undoStackIsEmpty() {
	return m_undoStack->count() == 0;
}

void MainWindow::setInfoViewOnHover(bool infoViewOnHover) {
	m_breadboardGraphicsView->setInfoViewOnHover(infoViewOnHover);
	m_schematicGraphicsView->setInfoViewOnHover(infoViewOnHover);
	m_pcbGraphicsView->setInfoViewOnHover(infoViewOnHover);

	m_paletteWidget->setInfoViewOnHover(infoViewOnHover);
}

void MainWindow::swapSelected() {
	ModelPart *selInParts = m_paletteWidget->selected();
	if(selInParts) m_currentWidget->swapSelected(selInParts);
}

#define ZIP_PART QString("part.")
#define ZIP_SVG  QString("svg.")

void MainWindow::saveBundledSketch() {
	QString fileExt;
	QString path;

	path = defaultSaveFolder() + "/" + m_fileName+"z";

	QString bundledFileName = QFileDialog::getSaveFileName(
			this,
			tr("Choose a file name"),
			path,
			tr("Fritzing (*%1)").arg(FritzingExtension+"z"),
			&fileExt
		  );

	if (bundledFileName.isEmpty()) return; // Cancel pressed

	if(!alreadyHasExtension(bundledFileName)) {
		fileExt = getExtFromFileDialog(fileExt);
		bundledFileName += fileExt;
	}

	QDir destFolder = QDir::temp();

	createFolderAnCdIntoIt(destFolder, getRandText());
	QString dirToRemove = destFolder.path();

	QString aux = QFileInfo(bundledFileName).fileName();
	QString destSketchPath = // remove the last "z" from the extension
			destFolder.path()+"/"+aux.left(aux.size()-1);
	DebugDialog::debug("saving sketch temporarily to "+destSketchPath);

	bool wasModified = isWindowModified();
	QString prevFileName = m_fileName;
	saveAsAux(destSketchPath);
	m_fileName = prevFileName;
	setWindowModified(wasModified);
	setTitle();

	QList<ModelPart*> partsToSave = m_sketchModel->root()->getAllNonCoreParts();
	foreach(ModelPart* mp, partsToSave) {
		QString partPath = mp->modelPartStuff()->path();
		QFile file(partPath);
		file.copy(destFolder.path()+"/"+ZIP_PART+QFileInfo(partPath).fileName());
		QList<SvgAndPartFilePath> views = mp->getAvailableViewFiles();
		foreach(SvgAndPartFilePath view, views) {
			if(view.coreContribOrUser() != "core") {
				QFile file(view.absolutePath());
				QString svgRelativePath = view.fileRelativePath();
				file.copy(destFolder.path()+"/"+ZIP_SVG+svgRelativePath.replace("/","."));
			}
		}
	}

	if(!createZipAndSaveTo(destFolder, bundledFileName)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to export %1 to bundled").arg(bundledFileName)
		);
	}

	rmdir(dirToRemove);
}

void MainWindow::loadBundledSketch(QString &fileName) {
	QDir destFolder = QDir::temp();

	createFolderAnCdIntoIt(destFolder, getRandText());
	QString unzipDir = destFolder.path();

	if(!unzipTo(fileName, unzipDir)) {
		QMessageBox::warning(
			this,
			tr("fritzing"),
			tr("Unable to open bundled file %1").arg(fileName)
		);
	}

	moveToPartsFolderAndLoad(unzipDir);

	rmdir(unzipDir);
}

void MainWindow::moveToPartsFolderAndLoad(const QString &unzipDirPath) {
	QDir unzipDir(unzipDirPath);
	QStringList namefilters;

	MainWindow* mw = new MainWindow(m_paletteModel, m_refModel);

	namefilters << ZIP_SVG+"*";
	foreach(QFileInfo file, unzipDir.entryInfoList(namefilters)) { // svg files
		mw->copyToSvgFolder(file);
	}

	namefilters.clear();
	namefilters << ZIP_PART+"*";
	foreach(QFileInfo file, unzipDir.entryInfoList(namefilters)) { // part files
		mw->copyToPartsFolder(file);
	}

	namefilters.clear();
	namefilters << "*"+FritzingExtension;

	// the sketch itself
	mw->load(unzipDir.entryInfoList(namefilters)[0].filePath(), false);
	mw->move(x()+CascadeFactor,y()+CascadeFactor);
	mw->show();
	mw->setWindowModified(true);

	closeIfEmptySketch();
}

void MainWindow::copyToSvgFolder(const QFileInfo& file, const QString &destFolder) {
	QFile svgfile(file.filePath());
	// let's make sure that we remove just the suffix
	QString fileName = file.fileName().remove(QRegExp("^"+ZIP_SVG));
	QString viewFolder = fileName.left(fileName.indexOf("."));
	fileName.remove(viewFolder+".");

	QString destFilePath =
		getApplicationSubFolderPath("parts")+"/svg/"+destFolder+"/"+viewFolder+"/"+fileName;

	backupExistingFileIfExists(destFilePath);
	if(svgfile.copy(destFilePath)) {
		m_alienFiles << destFilePath;
	}
}

void MainWindow::copyToPartsFolder(const QFileInfo& file, const QString &destFolder) {
	QFile partfile(file.filePath());
	// let's make sure that we remove just the suffix
	QString destFilePath =
		getApplicationSubFolderPath("parts")+"/"+destFolder+"/"+file.fileName().remove(QRegExp("^"+ZIP_PART));

	backupExistingFileIfExists(destFilePath);
	if(partfile.copy(destFilePath)) {
		m_alienFiles << destFilePath;
	}
	ModelPart *mp = m_refModel->loadPart(destFilePath, true);
	m_paletteWidget->addPart(mp);
}

void MainWindow::binSaved(bool hasPartsFromBundled) {
	if(hasPartsFromBundled) {
		// the bin will need those parts, so just keep them
		m_alienFiles.clear();
		resetTempFolder();
	}
}

#undef ZIP_PART
#undef ZIP_SVG


void MainWindow::backupExistingFileIfExists(const QString &destFilePath) {
	if(QFileInfo(destFilePath).exists()) {
		if(m_tempDir.path() == ".") {
			m_tempDir = QDir::temp();
			createFolderAnCdIntoIt(m_tempDir, getRandText());
			DebugDialog::debug("debug folder for overwritten files: "+m_tempDir.path());
		}

		QString fileBackupName = QFileInfo(destFilePath).fileName();
		m_filesReplacedByAlienOnes << destFilePath;
		QFile file(destFilePath);
		bool alreadyExists = file.exists();
		file.copy(m_tempDir.path()+"/"+fileBackupName);

		if(alreadyExists) {
			file.remove(destFilePath);
		}
	}
}

void MainWindow::recoverBackupedFiles() {
	foreach(QString originalFilePath, m_filesReplacedByAlienOnes) {
		QFile file(m_tempDir.path()+"/"+QFileInfo(originalFilePath).fileName());
		if(file.exists(originalFilePath)) {
			file.remove();
		}
		file.copy(originalFilePath);
	}
	resetTempFolder();
}

void MainWindow::resetTempFolder() {
	if(m_tempDir.path() != ".") {
		rmdir(m_tempDir);
		m_tempDir = QDir::temp();
	}
	m_filesReplacedByAlienOnes.clear();
}

void MainWindow::routingStatusSlot(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumpers) {
	QString theText;
	if (netCount == 0) {
		theText = tr("No connections to route");
	} else if (netCount == netRoutedCount) {
		if (jumpers == 0) {
			theText = tr("Routing completed");
		}
		else {
			theText = tr("Routing completed using %1 jumper %2")
				.arg(jumpers)
				.arg(makeGrammaticalNumber(jumpers, tr("wire"), tr("wires")));
		}
	} else {
		theText = tr("%1 of %2 nets routed - %3 %4 still to be routed")
			.arg(netRoutedCount)
			.arg(netCount)
			.arg(connectorsLeftToRoute)
			.arg(makeGrammaticalNumber(connectorsLeftToRoute, tr("connector"), tr("connectors")));
	}
	m_routingStatusLabel->setLabelText(theText);

	updateTraceMenu();
}


void MainWindow::clearRoutingSlot(SketchWidget * sketchWidget, QUndoCommand * parentCommand) {
	Q_UNUSED(sketchWidget);

	m_pcbGraphicsView->clearRouting(parentCommand);
}

QMenu *MainWindow::breadboardItemMenu() {
	QMenu *menu = new QMenu(QObject::tr("Part"), this);
	menu->addAction(m_rotate90cwAct);
	menu->addAction(m_rotate180Act);
	menu->addAction(m_rotate90ccwAct);
	menu->addAction(m_flipHorizontalAct);
	menu->addAction(m_flipVerticalAct);
	return viewItemMenuAux(menu);
}

QMenu *MainWindow::schematicItemMenu() {
	QMenu *menu = new QMenu(QObject::tr("Part"), this);
	menu->addAction(m_rotate90cwAct);
	menu->addAction(m_rotate180Act);
	menu->addAction(m_rotate90ccwAct);
	menu->addAction(m_flipHorizontalAct);
	menu->addAction(m_flipVerticalAct);
	return viewItemMenuAux(menu);
}

QMenu *MainWindow::pcbItemMenu() {
	QMenu *menu = new QMenu(QObject::tr("Part"), this);
	menu->addAction(m_rotate90cwAct);
	menu->addAction(m_rotate180Act);
	menu->addAction(m_rotate90ccwAct);
	menu = viewItemMenuAux(menu);
	return menu;
}

QMenu *MainWindow::pcbWireMenu() {
	QMenu *menu = new QMenu(QObject::tr("Wire"), this);
	menu->addAction(m_createTraceAct);
	menu->addAction(m_createJumperAct);
	return menu;
}

QMenu *MainWindow::viewItemMenuAux(QMenu* menu) {
	menu->addSeparator();
	menu->addAction(m_bringToFrontAct);
	menu->addAction(m_bringForwardAct);
	menu->addAction(m_sendBackwardAct);
	menu->addAction(m_sendToBackAct);
	menu->addSeparator();
	menu->addAction(m_copyAct);
	menu->addAction(m_duplicateAct);
	menu->addAction(m_deleteAct);
	menu->addSeparator();
	menu->addAction(m_openInPartsEditorAct);
	menu->addAction(m_addToBinAct);

#ifndef QT_NO_DEBUG
	menu->addSeparator();
	menu->addAction(m_infoViewOnHoverAction);
	//menu->addAction(m_swapPartAction);
#endif

    connect(
    	menu,
    	SIGNAL(aboutToShow()),
    	this,
    	SLOT(updatePartMenu())
    );

    return menu;
}

void MainWindow::applyReadOnlyChange(bool isReadOnly) {
	m_saveAct->setDisabled(isReadOnly);
}

void MainWindow::currentNavigatorChanged(MiniViewContainer * miniView)
{
	int index = m_navigators.indexOf(miniView);
	if (index < 0) return;

	int oldIndex = m_tabWidget->currentIndex();
	if (oldIndex == index) return;

	this->m_tabWidget->setCurrentIndex(index);

	setDockColorAnd(m_navigators[index], dockSelectedColor, ___emptyString___);
	setDockColorAnd(m_navigators[oldIndex], dockUnselectedColor, clickToSeeString);
}

void MainWindow::setDockColorAnd(MiniViewContainer * miniView, const QString & colorString, const QString & prefix)
{
	QWidget * parentWidget = miniView->parentWidget();
	QString name = parentWidget->objectName();
	parentWidget->setStyleSheet(colorString.arg(name));
	QStringList names = name.split("_");
	QString useName = names.join(" ");
	if (prefix.length() == 0) {
		parentWidget->setWindowTitle(useName);
	}
	else {
		parentWidget->setWindowTitle(prefix.arg(useName));
	}
}

