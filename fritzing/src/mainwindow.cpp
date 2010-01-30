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

#include <QtGui>
#include <QtXml>
#include <QList>
#include <QFileInfo>
#include <QStringList>
#include <QFileInfoList>
#include <QDir>
#include <QLabel>
#include <QTime>
#include <QCleanlooksStyle>
#include <QSettings>
#include <QRegExp>
#include <QPaintDevice>
#include <QPixmap>
#include <QTimer>

#include "items/paletteitem.h"
#include "mainwindow.h"
#include "debugdialog.h"
#include "connectors/connector.h"
#include "partsbinpalette/partsbinpalettewidget.h"
#include "fdockwidget.h"
#include "infoview/htmlinfoview.h"
#include "waitpushundostack.h"
#include "layerattributes.h"
#include "navigator/triplenavigator.h"
#include "sketch/breadboardsketchwidget.h"
#include "sketch/schematicsketchwidget.h"
#include "sketch/pcbsketchwidget.h"
#include "svg/svgfilesplitter.h"
#include "utils/folderutils.h"
#include "utils/textutils.h"
#include "utils/graphicsutils.h"
#include "items/mysterypart.h"
#include "items/pinheader.h"

#include "help/helper.h"
#include "dockmanager.h"
#include "group/saveasmoduledialog.h"

#include "partsbinpalette/binmanager/binmanager.h"

#include "fsvgrenderer.h"
#include "utils/fsizegrip.h"
#include "utils/expandinglabel.h"
#include "viewswitcher/viewswitcher.h"
#include "viewswitcher/viewswitcherdockwidget.h"

#include "utils/autoclosemessagebox.h"
#include "utils/fileprogressdialog.h"
#include "items/resizableboard.h"
#include "items/resistor.h"
#include "items/symbolpaletteitem.h"

const QString MainWindow::UntitledSketchName = "Untitled Sketch";
int MainWindow::UntitledSketchIndex = 1;
int MainWindow::CascadeFactorX = 21;
int MainWindow::CascadeFactorY = 19;
int MainWindow::RestartNeeded = 0;

static const int MainWindowDefaultWidth = 840;
static const int MainWindowDefaultHeight = 600;

MainWindow::MainWindow(PaletteModel * paletteModel, ReferenceModel *refModel) :
	FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension())
{
	m_wireColorMenu = NULL;
	m_viewSwitcherDock = NULL;
	m_checkForUpdatesAct = NULL;
	m_fileProgressDialog = NULL;
	m_currentGraphicsView = NULL;
	m_comboboxChanged = false;
	m_helper = NULL;

	resize(MainWindowDefaultWidth, MainWindowDefaultHeight);

	// Create dot icons
	m_dotIcon = QIcon(":/resources/images/dot.png");
	m_emptyIcon = QIcon();

	m_currentWidget = NULL;
	m_currentGraphicsView = NULL;
	m_firstOpen = true;

	m_statusBar = new QStatusBar(this);
	setStatusBar(m_statusBar);
	m_statusBar->setSizeGripEnabled(false);

	setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_WS_MAC
	//setAttribute(Qt::WA_QuitOnClose, false);					// restoring this temporarily (2008.12.19)
#endif
    m_dontClose = m_closing = false;

	m_paletteModel = paletteModel;
	m_refModel = refModel;
	m_sketchModel = new SketchModel(true);

	m_tabWidget = new QStackedWidget(this); //   FTabWidget(this);
	m_tabWidget->setObjectName("sketch_tabs");

	setCentralWidget(m_tabWidget);

	connect(this, SIGNAL(changeActivationSignal(bool, QWidget *)), qApp, SLOT(changeActivation(bool, QWidget *)), Qt::DirectConnection);
	connect(this, SIGNAL(destroyed(QObject *)), qApp, SLOT(topLevelWidgetDestroyed(QObject *)));
	connect(this, SIGNAL(externalProcessSignal(QString &, QString &, QStringList &)),
			qApp, SLOT(externalProcessSlot(QString &, QString &, QStringList &)), 
			Qt::DirectConnection);
}

void MainWindow::init() {
	m_restarting = false;

	// all this belongs in viewLayer.xml
	m_breadboardGraphicsView = new BreadboardSketchWidget(ViewIdentifierClass::BreadboardView, this);
	initSketchWidget(m_breadboardGraphicsView);
	m_breadboardWidget = new SketchAreaWidget(m_breadboardGraphicsView,this);
	m_tabWidget->addWidget(m_breadboardWidget);

	m_schematicGraphicsView = new SchematicSketchWidget(ViewIdentifierClass::SchematicView, this);
	initSketchWidget(m_schematicGraphicsView);
	m_schematicWidget = new SketchAreaWidget(m_schematicGraphicsView, this);
	m_tabWidget->addWidget(m_schematicWidget);

	m_pcbGraphicsView = new PCBSketchWidget(ViewIdentifierClass::PCBView, this);
	initSketchWidget(m_pcbGraphicsView);
	m_pcbWidget = new SketchAreaWidget(m_pcbGraphicsView, this);
	m_tabWidget->addWidget(m_pcbWidget);

    m_undoView = new QUndoView();
    m_undoGroup = new QUndoGroup(this);
    m_undoView->setGroup(m_undoGroup);
    m_undoGroup->setActiveStack(m_undoStack);

    m_dockManager = new DockManager(this);
    m_dockManager->createBinAndInfoViewDocks();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

	m_viewSwitcher = new ViewSwitcher();
	connect(m_viewSwitcher, SIGNAL(viewSwitched(int)), this, SLOT(viewSwitchedTo(int)));
	connect(this, SIGNAL(viewSwitched(int)), m_viewSwitcher, SLOT(viewSwitchedTo(int)));
	m_viewSwitcher->viewSwitchedTo(0);

    m_dockManager->createDockWindows();

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

	QFile styleSheet(":/resources/styles/fritzing.qss");
    if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/fritzing.qss");
	} else {
		QString platformDependantStyle = "";
		QString platformDependantStylePath;
#ifdef Q_WS_X11
		if(style()->metaObject()->className()==QString("OxygenStyle")) {
			QFile oxygenStyleSheet(":/resources/styles/linux-kde-oxygen.qss");
			if(oxygenStyleSheet.open(QIODevice::ReadOnly)) {
				platformDependantStyle += oxygenStyleSheet.readAll();
			}
		}
		platformDependantStylePath = ":/resources/styles/linux.qss";
#endif

#ifdef Q_WS_MAC
		platformDependantStylePath = ":/resources/styles/mac.qss";
#endif

#ifdef Q_WS_WIN
		platformDependantStylePath = ":/resources/styles/win.qss";
#endif

		QFile platformDependantStyleSheet(platformDependantStylePath);
		if(platformDependantStyleSheet.open(QIODevice::ReadOnly)) {
			platformDependantStyle += platformDependantStyleSheet.readAll();
		}
		setStyleSheet(styleSheet.readAll()+platformDependantStyle);
	}

    m_breadboardGraphicsView->setItemMenu(breadboardItemMenu());
    m_breadboardGraphicsView->setWireMenu(breadboardWireMenu());

    m_pcbGraphicsView->setWireMenu(pcbWireMenu());
    m_pcbGraphicsView->setItemMenu(pcbItemMenu());

    m_schematicGraphicsView->setItemMenu(schematicItemMenu());
    m_schematicGraphicsView->setWireMenu(schematicWireMenu());

    m_breadboardGraphicsView->setInfoView(m_infoView);
    m_pcbGraphicsView->setInfoView(m_infoView);
    m_schematicGraphicsView->setInfoView(m_infoView);

	// make sure to set the connections after the views have been created
	connect(m_tabWidget, SIGNAL(currentChanged ( int )),
			this, SLOT(tabWidget_currentChanged( int )));

	connectPairs();

	// do this the first time, since the current_changed signal wasn't sent
	int tab = 0;
	currentNavigatorChanged(m_navigators[tab]);
	tabWidget_currentChanged(tab+1);
	tabWidget_currentChanged(tab);

	this->installEventFilter(this);

	QSettings settings;
	m_viewSwitcherDock->prestorePreference();
	if(!settings.value("main/state").isNull()) {
		restoreState(settings.value("main/state").toByteArray());
		restoreGeometry(settings.value("main/geometry").toByteArray());
	}
	m_viewSwitcherDock->restorePreference();

	setMinimumSize(0,0);
	m_tabWidget->setMinimumWidth(500);
	m_tabWidget->setMinimumWidth(0);

	m_miniViewContainerBreadboard->setView(m_breadboardGraphicsView);
	m_miniViewContainerSchematic->setView(m_schematicGraphicsView);
	m_miniViewContainerPCB->setView(m_pcbGraphicsView);

	connect(this, SIGNAL(readOnlyChanged(bool)), this, SLOT(applyReadOnlyChange(bool)));

	m_helper = new Helper(this, true);

	m_setUpDockManagerTimer.setSingleShot(true);
	connect(&m_setUpDockManagerTimer, SIGNAL(timeout()), m_dockManager, SLOT(keepMargins()));
	m_setUpDockManagerTimer.start(1000);
}

MainWindow::~MainWindow()
{
	delete m_sketchModel;
	m_dockManager->dontKeepMargins();
	m_setUpDockManagerTimer.stop();
}


void MainWindow::initSketchWidget(SketchWidget * sketchWidget) {
	sketchWidget->setPaletteModel(m_paletteModel);
	sketchWidget->setSketchModel(m_sketchModel);
	sketchWidget->setRefModel(m_refModel);
	sketchWidget->setUndoStack(m_undoStack);
	sketchWidget->setChainDrag(true);			// enable bend points
	sketchWidget->addViewLayers();
}

void MainWindow::connectPairs() {
	connectPair(m_breadboardGraphicsView, m_schematicGraphicsView);
	connectPair(m_breadboardGraphicsView, m_pcbGraphicsView);
	connectPair(m_schematicGraphicsView, m_breadboardGraphicsView);
	connectPair(m_schematicGraphicsView, m_pcbGraphicsView);
	connectPair(m_pcbGraphicsView, m_breadboardGraphicsView);
	connectPair(m_pcbGraphicsView, m_schematicGraphicsView);

	bool succeeded = connect(m_pcbGraphicsView, SIGNAL(routingStatusSignal(SketchWidget *, int, int, int, int)),
						this, SLOT(routingStatusSlot(SketchWidget *, int, int, int, int)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(routingStatusSignal(SketchWidget *, int, int, int, int)),
						this, SLOT(routingStatusSlot(SketchWidget *, int, int, int, int)));

	succeeded = connect(m_breadboardGraphicsView, SIGNAL(swapSignal(const QString &, const QString &, QMap<QString, QString> &)), 
						this, SLOT(swapSelectedMap(const QString &, const QString &, QMap<QString, QString> &)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(swapSignal(const QString &, const QString &, QMap<QString, QString> &)), 
						this, SLOT(swapSelectedMap(const QString &, const QString &, QMap<QString, QString> &)));
	succeeded = connect(m_pcbGraphicsView, SIGNAL(swapSignal(const QString &, const QString &, QMap<QString, QString> &)), 
						this, SLOT(swapSelectedMap(const QString &, const QString &, QMap<QString, QString> &)));

	
	succeeded = connect(m_pcbGraphicsView, SIGNAL(ratsnestChangeSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_pcbGraphicsView, SIGNAL(movingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_pcbGraphicsView, SIGNAL(rotatingFlippingSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));

	succeeded = connect(m_schematicGraphicsView, SIGNAL(ratsnestChangeSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	succeeded = connect(m_breadboardGraphicsView, SIGNAL(ratsnestChangeSignal(SketchWidget *, QUndoCommand *)),
						this, SLOT(clearRoutingSlot(SketchWidget *, QUndoCommand *)));
	

	/*
	succeeded = connect(m_schematicGraphicsView, SIGNAL(schematicDisconnectWireSignal(ConnectorPairHash &, QSet<ItemBase *> &, QHash<ItemBase *, ConnectorPairHash *> &, QUndoCommand *)),
						m_breadboardGraphicsView, SLOT(schematicDisconnectWireSlot(ConnectorPairHash &, QSet<ItemBase *> &, QHash<ItemBase *, ConnectorPairHash *> &, QUndoCommand *)),
						Qt::DirectConnection);
	*/

	succeeded = connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_breadboardGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
	succeeded = connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_schematicGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
	succeeded = connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_pcbGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
}

void MainWindow::connectPair(SketchWidget * signaller, SketchWidget * slotter)
{

	bool succeeded = connect(signaller, SIGNAL(itemAddedSignal(ModelPart *, const ViewGeometry &, long, SketchWidget *)),
							 slotter, SLOT(sketchWidget_itemAdded(ModelPart *, const ViewGeometry &, long, SketchWidget *)));

	succeeded = succeeded && connect(signaller, SIGNAL(itemDeletedSignal(long)),
									 slotter, SLOT(sketchWidget_itemDeleted(long)),
									 Qt::DirectConnection);

	succeeded = succeeded && connect(signaller, SIGNAL(clearSelectionSignal()),
									 slotter, SLOT(sketchWidget_clearSelection()));

	succeeded = succeeded && connect(signaller, SIGNAL(itemSelectedSignal(long, bool)),
									 slotter, SLOT(sketchWidget_itemSelected(long, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(selectAllItemsSignal(bool, bool)),
									 slotter, SLOT(selectAllItems(bool, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(wireDisconnectedSignal(long, QString)),
									 slotter, SLOT(sketchWidget_wireDisconnected(long,  QString)));
	succeeded = succeeded && connect(signaller, SIGNAL(wireConnectedSignal(long,  QString, long,  QString)),
									 slotter, SLOT(sketchWidget_wireConnected(long, QString, long, QString)));
	succeeded = succeeded && connect(signaller, SIGNAL(changeConnectionSignal(long,  QString, long,  QString, bool, bool)),
									 slotter, SLOT(sketchWidget_changeConnection(long, QString, long, QString, bool, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(copyItemSignal(long, QHash<ViewIdentifierClass::ViewIdentifier, ViewGeometry *> &)),
													   slotter, SLOT(sketchWidget_copyItem(long, QHash<ViewIdentifierClass::ViewIdentifier, ViewGeometry *> &)),
									 Qt::DirectConnection);

	succeeded = succeeded && connect(signaller, SIGNAL(cleanUpWiresSignal(CleanUpWiresCommand *)),
									 slotter, SLOT(sketchWidget_cleanUpWires(CleanUpWiresCommand *)) );
	succeeded = succeeded && connect(signaller, SIGNAL(dealWithRatsnestSignal(long, const QString &,
																			  long, const QString &,
																			  bool, RatsnestCommand *)),
									 slotter, SLOT(dealWithRatsnestSlot(long, const QString &,
																			  long, const QString &,
																			  bool, RatsnestCommand *)) );

	succeeded = succeeded && connect(signaller, SIGNAL(groupSignal(const QString &, long, QList<long> &, const ViewGeometry &, bool)),
									 slotter, SLOT(group(const QString &, long, QList<long> &, const ViewGeometry &, bool)) );
	succeeded = succeeded && connect(signaller, SIGNAL(restoreIndexesSignal(ModelPart *, ModelPartTiny *, bool )),
									 slotter, SLOT(restoreIndexes(ModelPart *, ModelPartTiny *, bool )) );

	succeeded = succeeded && connect(signaller, SIGNAL(checkStickySignal(long, bool, bool, CheckStickyCommand *)),
									 slotter, SLOT(checkSticky(long, bool, bool, CheckStickyCommand *)) );
	succeeded = succeeded && connect(signaller, SIGNAL(rememberStickySignal(long, QUndoCommand *)),
									 slotter, SLOT(rememberSticky(long, QUndoCommand *)),
									 Qt::DirectConnection);

	succeeded = succeeded && connect(signaller, SIGNAL(disconnectAllSignal(QList<ConnectorItem *>, QHash<ItemBase *, SketchWidget *> &, QUndoCommand *)),
									 slotter, SLOT(disconnectAllSlot(QList<ConnectorItem *>, QHash<ItemBase *, SketchWidget *> &, QUndoCommand *)),
									 Qt::DirectConnection);
	succeeded = succeeded && connect(signaller, SIGNAL(setResistanceSignal(long, QString, QString, bool)),
									 slotter, SLOT(setResistance(long, QString, QString, bool)));

	succeeded = succeeded && connect(signaller, SIGNAL(setPropSignal(long,  const QString &,  const QString &, bool)),
									 slotter, SLOT(setProp(long,  const QString &,  const QString &, bool)));

	succeeded = succeeded && connect(signaller, SIGNAL(setInstanceTitleSignal(long, const QString &, bool, bool )),
									 slotter, SLOT(setInstanceTitle(long, const QString &, bool, bool )));

	succeeded = succeeded && connect(signaller, SIGNAL(setVoltageSignal(qreal, bool )),
									 slotter, SLOT(setVoltage(qreal, bool )));

	succeeded = succeeded && connect(signaller, SIGNAL(showLabelFirstTimeSignal(long, bool, bool )),
									 slotter, SLOT(showLabelFirstTime(long, bool, bool )));


	if (!succeeded) {
		DebugDialog::debug("connectPair failed");
	}

}

void MainWindow::setCurrentFile(const QString &fileName, bool addToRecent) {
	m_fileName = fileName;

	updateRaiseWindowAction();
	setTitle();

	if(addToRecent) {
		QSettings settings;
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

ExpandingLabel * MainWindow::createRoutingStatusLabel(SketchAreaWidget * parent) {
	ExpandingLabel * routingStatusLabel = new ExpandingLabel(m_pcbWidget);
	routingStatusLabel->setObjectName(SketchAreaWidget::RoutingStateLabelName);
	parent->setRoutingStatusLabel(routingStatusLabel);
	routingStatusSlot(parent->graphicsView(), 0,0,0,0);
	return routingStatusLabel;
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

SketchToolButton *MainWindow::createShareButton(SketchAreaWidget *parent) {
	SketchToolButton *shareButton = new SketchToolButton("Share",parent, m_shareOnlineAct);
	shareButton->setText(tr("Share"));
	shareButton->setEnabledIcon();					// seems to need this to display button icon first time
	return shareButton;
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

SketchToolButton *MainWindow::createNoteButton(SketchAreaWidget *parent) {
	SketchToolButton *noteButton = new SketchToolButton("Notes",parent, m_addNoteAct);
	noteButton->setText(tr("Add a note"));
	noteButton->setEnabledIcon();					// seems to need this to display button icon first time
	return noteButton;
}

SketchToolButton *MainWindow::createExportEtchableButton(SketchAreaWidget *parent) {
	SketchToolButton *exportEtchableButton = new SketchToolButton("Diy",parent, m_exportEtchableAct);
	exportEtchableButton->setText(tr("Export Etchable PDF"));
	exportEtchableButton->setEnabledIcon();				// seems to need this to display button icon first time
	return exportEtchableButton;
}

QWidget *MainWindow::createToolbarSpacer(SketchAreaWidget *parent) {
	QFrame *toolbarSpacer = new QFrame(parent);
	QHBoxLayout *spacerLayout = new QHBoxLayout(toolbarSpacer);
	spacerLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));

	return toolbarSpacer;
}

QList<QWidget*> MainWindow::getButtonsForView(ViewIdentifierClass::ViewIdentifier viewId) {
	QList<QWidget*> retval;
	SketchAreaWidget *parent;
	switch(viewId) {
		case ViewIdentifierClass::BreadboardView: parent = m_breadboardWidget; break;
		case ViewIdentifierClass::SchematicView: parent = m_schematicWidget; break;
		case ViewIdentifierClass::PCBView: parent = m_pcbWidget; break;
		default: return retval;
	}
	retval << createShareButton(parent) << createNoteButton(parent) << createRotateButton(parent);
	switch (viewId) {
		case ViewIdentifierClass::BreadboardView:
			retval << createFlipButton(parent); 
			break;
		case ViewIdentifierClass::SchematicView:
			retval << createFlipButton(parent) << createToolbarSpacer(parent) << createAutorouteButton(parent) << createRoutingStatusLabel(parent);
			break;
		case ViewIdentifierClass::PCBView:
			retval << SketchAreaWidget::separator(parent) << createAutorouteButton(parent)
				   << createExportEtchableButton(parent) << createRoutingStatusLabel(parent);
			break;
		default:
			break;
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
	return dynamic_cast<SketchAreaWidget*>(m_currentGraphicsView->parent());
}

void MainWindow::updateZoomOptionsNoMatterWhat(qreal zoom) {
	currentSketchArea()->zoomComboBox()->setEditText(tr("%1%").arg(zoom));
}

void MainWindow::updateViewZoom(qreal newZoom) {
	m_comboboxChanged = true;
	if(m_currentGraphicsView) m_currentGraphicsView->absoluteZoom(newZoom);
}


void MainWindow::createStatusBar()
{
    m_statusBar->showMessage(tr("Ready"));
}

void MainWindow::tabWidget_currentChanged(int index) {
	SketchAreaWidget * widgetParent = dynamic_cast<SketchAreaWidget *>(m_tabWidget->currentWidget());
	if (widgetParent == NULL) return;

	m_currentWidget = widgetParent;

	QStatusBar *sb = statusBar();
	connect(sb, SIGNAL(messageChanged(const QString &)), m_statusBar, SLOT(showMessage(const QString &)));
	widgetParent->addStatusBar(m_statusBar);
	if(sb != m_statusBar) sb->hide();

	if (m_breadboardGraphicsView) m_breadboardGraphicsView->setCurrent(false);
	if (m_schematicGraphicsView) m_schematicGraphicsView->setCurrent(false);
	if (m_pcbGraphicsView) m_pcbGraphicsView->setCurrent(false);

	SketchWidget *widget = widgetParent->graphicsView();

	if(m_currentGraphicsView) {
		disconnect(
			m_currentGraphicsView,
			SIGNAL(selectionChangedSignal()),
			this,
			SLOT(updateTransformationActions())
		);
	}
	m_currentGraphicsView = widget;
	if (widget == NULL) return;

	connect(
		m_currentGraphicsView,					// don't connect directly to the scene here, connect to the widget's signal
		SIGNAL(selectionChangedSignal()),
		this,
		SLOT(updateTransformationActions())
	);


	m_currentGraphicsView->setCurrent(true);

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
	updateTransformationActions();

	setTitle();

	// triggers a signal to the navigator widget
	m_navigators[index]->miniViewMousePressedSlot();
	emit viewSwitched(index);

	if (m_helper == NULL) {
		m_showInViewHelpAct->setChecked(false);
	}
	else {
		m_showInViewHelpAct->setChecked(m_helper->helpVisible(m_tabWidget->currentIndex()));
	}

	m_currentGraphicsView->updateInfoView();

	// update issue with 4.5.1?
	m_currentGraphicsView->updateConnectors();

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

	bool whatWithAliens = whatToDoWithAlienFiles();
	if(!beforeClosing() || !whatWithAliens ||!m_paletteWidget->beforeClosing()) {
		event->ignore();
		return;
	}

	if(whatWithAliens && m_paletteWidget->hasAlienParts()) {
		m_paletteWidget->createIfMyPartsNotExists();
	}


	DebugDialog::debug(QString("current top level windows: %1").arg(QApplication::topLevelWidgets().size()));
	foreach (QWidget * widget, QApplication::topLevelWidgets()) {
		QMenu * menu = qobject_cast<QMenu *>(widget);
		if (menu != NULL) {
			continue;				// QMenus are always top level widgets, even if they have parents...
		}
		DebugDialog::debug(QString("top level widget %1 %2 %3")
			.arg(widget->metaObject()->className())
			.arg(widget->windowTitle())
			.arg(widget->toolTip())
			);
	}

	m_closing = true;
	emit aboutToClose();

	int count = 0;
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		if (widget == this) continue;
		if (dynamic_cast<QMainWindow *>(widget) == NULL) continue;

		count++;
	}

	if (count == 0) {
		DebugDialog::closeDebug();
	}

	QSettings settings;
	settings.setValue("main/state",saveState());
	settings.setValue("main/geometry",saveGeometry());

	QMainWindow::closeEvent(event);
}

bool MainWindow::whatToDoWithAlienFiles() {
	if (m_alienFiles.size() > 0) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, tr("Save %1").arg(QFileInfo(m_fileName).baseName()),
						 m_alienPartsMsg
						 .arg(QFileInfo(m_fileName).baseName()),
						 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		// TODO: translate button text
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

void MainWindow::acceptAlienFiles() {
	m_alienFiles.clear();
}


void MainWindow::setZoomComboBoxValue(qreal value, ZoomComboBox* zoomComboBox) {
	if(!zoomComboBox) zoomComboBox = currentSketchArea()->zoomComboBox();
	zoomComboBox->setEditText(tr("%1%").arg(value,0,'f',2));
}

void MainWindow::saveDocks()
{
	for (int i = 0; i < children().count(); i++) {
		FDockWidget * dock = dynamic_cast<FDockWidget *>(children()[i]);
		if (dock == NULL) continue;

		//DebugDialog::debug(QString("saving dock %1").arg(dock->windowTitle()));
		dock->saveState();

		if (dock->isFloating() && dock->isVisible()) {
			//DebugDialog::debug(QString("hiding dock %1").arg(dock->windowTitle()));
			dock->hide();
		}
	}
}

void MainWindow::restoreDocks() {
	for (int i = 0; i < children().count(); i++) {
		FDockWidget * dock = dynamic_cast<FDockWidget *>(children()[i]);
		if (dock == NULL) continue;

		// DebugDialog::debug(QString("restoring dock %1").arg(dock->windowTitle()));
		dock->restoreState();
	}
}


ModelPart *MainWindow::loadPartFromFile(const QString& newPartPath, bool connectorsChanged) {
	if(connectorsChanged && wannaRestart()) {
		QApplication::exit(RestartNeeded);
		return NULL;
	} else {
		ModelPart* mp = ((PaletteModel*)m_refModel)->addPart(newPartPath, true, true);
		m_refModel->addPart(mp,true);
		FSvgRenderer::removeFromHash(mp->moduleID(), newPartPath);
		return mp;
	}
}

bool MainWindow::wannaRestart() {
	QMessageBox::StandardButton btn = QMessageBox::question(this,
		tr("Updating existing part"),
		tr("Some connectors have changed.\n"
			"In order to see the changes, you have to restart fritzing.\n"
			"Do you want to restart now?"
		),
		QMessageBox::Yes|QMessageBox::No
	);
	bool result = (btn == QMessageBox::Yes);
	if(result) {
		m_restarting = true;
		close();
		m_restarting = false;
	}
	return result;
}

void MainWindow::loadPart(const QString &newPartPath, long partsEditorId, bool connectorsChanged) {
	ModelPart * modelPart = loadPartFromFile(newPartPath, connectorsChanged);
	if(modelPart && modelPart->isValid()) {
		if(m_binsWithPartsEditorRequests.contains(partsEditorId)
		   && !m_binsWithPartsEditorRequests[partsEditorId]->currentBinIsCore()	) {
			m_paletteWidget->addPartTo(m_binsWithPartsEditorRequests[partsEditorId],modelPart);
		} else {
			m_paletteWidget->addNewPart(modelPart);
		}
		m_infoView->reloadContent(m_currentGraphicsView);
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

const QString MainWindow::untitledFileName() {
	return UntitledSketchName;
}

int &MainWindow::untitledFileCount() {
	return UntitledSketchIndex;
}

const QString MainWindow::fileExtension() {
	return FritzingSketchExtension;
}

const QString MainWindow::defaultSaveFolder() {
	return FolderUtils::openSaveFolder();
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

#define ZIP_PART QString("part.")
#define ZIP_SVG  QString("svg.")

void MainWindow::saveBundledSketch() {
	bool wasModified = isWindowModified();
	bool prevSaveBtnState = m_saveAct->isEnabled();
	saveBundledNonAtomicEntity(
		m_fileName, FritzingBundleExtension, this,
		m_sketchModel->root()->getAllNonCoreParts()
	);
	m_saveAct->setEnabled(prevSaveBtnState);
	setWindowModified(wasModified);
	setTitle();
}

void MainWindow::saveBundledNonAtomicEntity(QString &filename, const QString &extension, Bundler *bundler, const QList<ModelPart*> &partsToSave) {
	QString fileExt;
	QString path = defaultSaveFolder() + "/" + QFileInfo(filename).fileName()+"z";
	QString bundledFileName = FolderUtils::getSaveFileName(
			this,
			tr("Specify a file name"),
			path,
			tr("Fritzing (*%1)").arg(extension),
			&fileExt
		  );

	if (bundledFileName.isEmpty()) return; // Cancel pressed

	FileProgressDialog progress("Saving...", 0, this);

	if(!alreadyHasExtension(bundledFileName, extension)) {
		bundledFileName += extension;
	}

	QApplication::processEvents();

	QDir destFolder = QDir::temp();
	FolderUtils::createFolderAnCdIntoIt(destFolder, getRandText());
	QString dirToRemove = destFolder.path();

	QString aux = QFileInfo(bundledFileName).fileName();
	QString destSketchPath = // remove the last "z" from the extension
			destFolder.path()+"/"+aux.left(aux.size()-1);
	DebugDialog::debug("saving entity temporarily to "+destSketchPath);

	QString prevFileName = filename;
	QApplication::processEvents();
	bundler->saveAsAux(destSketchPath);
	filename = prevFileName;

	foreach(ModelPart* mp, partsToSave) {
		saveBundledAux(mp, destFolder);
	}

	QApplication::processEvents();

	if(!createZipAndSaveTo(destFolder, bundledFileName)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to export %1 as shareable").arg(bundledFileName)
		);
	}

	rmdir(dirToRemove);
}

void MainWindow::loadBundledSketch(const QString &fileName) {
	loadBundledNonAtomicEntity(fileName, this, /*addToBin*/true);
}

void MainWindow::loadBundledNonAtomicEntity(const QString &fileName, Bundler* bundler, bool addToBin) {
	QDir destFolder = QDir::temp();

	FolderUtils::createFolderAnCdIntoIt(destFolder, getRandText());
	QString unzipDirPath = destFolder.path();

	if(!unzipTo(fileName, unzipDirPath)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to open shareable %1").arg(fileName)
		);

		// gotta return now, or loadBundledSketchAux will crash
		return;
	}

	QDir unzipDir(unzipDirPath);

	QList<ModelPart*> mps = moveToPartsFolder(unzipDir,this,addToBin);
	// the bundled itself
	bundler->loadBundledAux(unzipDir,mps);
	m_fileName.clear();							// clear m_fileName, so "save" will become "save as"; otherwise it will attempt to save this in the unzipDirPath that you're about to rmdir

	rmdir(unzipDirPath);
}

void MainWindow::loadBundledAux(QDir &unzipDir, QList<ModelPart*> mps) {
	Q_UNUSED(mps);

	QStringList namefilters;
	namefilters << "*"+FritzingSketchExtension;

	this->load(unzipDir.entryInfoList(namefilters)[0].filePath(), false);
	this->setWindowModified(true);

	m_alienPartsMsg = tr("Do you want to keep the parts that were loaded with this shareable sketch %1?");

	closeIfEmptySketch(this);
}

void MainWindow::loadBundledPartFromWeb() {
	QMainWindow *mw = new QMainWindow();
	QString url = "http://localhost:8081/parts_gen/choose";
	QWebView *view = new QWebView(mw);
	mw->setCentralWidget(view);
	view->setUrl(QUrl(url));
	mw->show();
	mw->raise();
}

void MainWindow::loadBundledPart() {
	QString fileName = FolderUtils::getOpenFileName(
		this,
		tr("Select a part to import"),
		defaultSaveFolder(),
		tr("External Part (*%1)").arg(FritzingBundledPartExtension)
	);
	if (fileName.isNull()) return;

	loadBundledPart(fileName);
}

ModelPart* MainWindow::loadBundledPart(const QString &fileName, bool addToBin) {
	QDir destFolder = QDir::temp();

	FolderUtils::createFolderAnCdIntoIt(destFolder, getRandText());
	QString unzipDirPath = destFolder.path();

	if(!unzipTo(fileName, unzipDirPath)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to open shareable part %1").arg(fileName)
		);
	}

	QDir unzipDir(unzipDirPath);

	QList<ModelPart*> mps = moveToPartsFolder(unzipDir,this,addToBin);
	Q_ASSERT(mps.count()==1); // if this fails, that means that the bundled was wrong

	rmdir(unzipDirPath);

	return mps[0];
}

void MainWindow::saveBundledPart(const QString &moduleId) {
	QString modIdToExport;
	ModelPart* mp;

	if(moduleId == ___emptyString___) {
		if (m_currentGraphicsView == NULL) return;
		PaletteItem *selectedPart = m_currentGraphicsView->getSelectedPart();
		mp = selectedPart->modelPart();
		modIdToExport = mp->moduleID();
	} else {
		modIdToExport = moduleId;
		mp = m_refModel->retrieveModelPart(moduleId);
	}
	QString partTitle = mp->title();

	QString fileExt;
	QString path = defaultSaveFolder()+"/"+partTitle+FritzingBundledPartExtension;
	QString bundledFileName = FolderUtils::getSaveFileName(
			this,
			tr("Specify a file name"),
			path,
			tr("Fritzing Part (*%1)").arg(FritzingBundledPartExtension),
			&fileExt
		  );

	if (bundledFileName.isEmpty()) return; // Cancel pressed

	if(!alreadyHasExtension(bundledFileName, FritzingBundledPartExtension)) {
		bundledFileName += FritzingBundledPartExtension;
	}

	QDir destFolder = QDir::temp();

	FolderUtils::createFolderAnCdIntoIt(destFolder, getRandText());
	QString dirToRemove = destFolder.path();

	QString aux = QFileInfo(bundledFileName).fileName();
	QString destPartPath = // remove the last "z" from the extension
			destFolder.path()+"/"+aux.left(aux.size()-1);
	DebugDialog::debug("saving part temporarily to "+destPartPath);

	bool wasModified = isWindowModified();
	QString prevFileName = m_fileName;
	//saveAsAux(destPartPath);
	m_fileName = prevFileName;
	setWindowModified(wasModified);
	setTitle();

	saveBundledAux(mp, destFolder);


	if(!createZipAndSaveTo(destFolder, bundledFileName)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to export %1 to shareable sketch").arg(bundledFileName)
		);
	}

	rmdir(dirToRemove);
}

void MainWindow::saveBundledAux(ModelPart *mp, const QDir &destFolder) {
	QString partPath = mp->modelPartShared()->path();
	QFile file(partPath);
	file.copy(destFolder.path()+"/"+ZIP_PART+QFileInfo(partPath).fileName());
	QList<SvgAndPartFilePath> views = mp->getAvailableViewFiles();
	foreach(SvgAndPartFilePath view, views) {
		if(view.coreContribOrUser() != "core") {
			QFile file(view.concat());
			QString svgRelativePath = view.relativePath();
			file.copy(destFolder.path()+"/"+ZIP_SVG+svgRelativePath.replace("/","."));
		}
	}
}

QList<ModelPart*> MainWindow::moveToPartsFolder(QDir &unzipDir, MainWindow* mw, bool addToBin) {
	QStringList namefilters;
	QList<ModelPart*> retval;

	Q_ASSERT(mw);
	if(mw) {
		namefilters << ZIP_SVG+"*";
		foreach(QFileInfo file, unzipDir.entryInfoList(namefilters)) { // svg files
			mw->copyToSvgFolder(file);
		}

		namefilters.clear();
		namefilters << ZIP_PART+"*";

		foreach(QFileInfo file, unzipDir.entryInfoList(namefilters)) { // part files
			retval << mw->copyToPartsFolder(file,addToBin);
		}
	}

	return retval;
}

void MainWindow::copyToSvgFolder(const QFileInfo& file, const QString &destFolder) {
	QFile svgfile(file.filePath());
	// let's make sure that we remove just the suffix
	QString fileName = file.fileName().remove(QRegExp("^"+ZIP_SVG));
	QString viewFolder = fileName.left(fileName.indexOf("."));
	fileName.remove(viewFolder+".");

	QString destFilePath =
		FolderUtils::getUserDataStorePath("parts")+"/svg/"+destFolder+"/"+viewFolder+"/"+fileName;

	backupExistingFileIfExists(destFilePath);
	if(svgfile.copy(destFilePath)) {
		m_alienFiles << destFilePath;
	}
}

ModelPart* MainWindow::copyToPartsFolder(const QFileInfo& file, bool addToBin, const QString &destFolder) {
	QFile partfile(file.filePath());
	// let's make sure that we remove just the suffix
	QString destFilePath =
		FolderUtils::getUserDataStorePath("parts")+"/"+destFolder+"/"+file.fileName().remove(QRegExp("^"+ZIP_PART));

	backupExistingFileIfExists(destFilePath);
	if(partfile.copy(destFilePath)) {
		m_alienFiles << destFilePath;
		m_alienPartsMsg = tr("Do you want to keep the imported parts?");
	}
	ModelPart *mp = m_refModel->loadPart(destFilePath, true);
	mp->setAlien(true);

	if(addToBin) {
		//m_paletteWidget->addPart(mp);
		m_paletteWidget->addToMyPart(mp);
	}

	return mp;
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
			FolderUtils::createFolderAnCdIntoIt(m_tempDir, getRandText());
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

void MainWindow::routingStatusSlot(SketchWidget * sketchWidget, int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumpers) {
	QString theText;
	if (netCount == 0) {
		theText = tr("No connections to route");
	} else if (netCount == netRoutedCount) {
		if (jumpers == 0) {
			theText = tr("Routing completed");
		}
		else {
			theText = tr("Routing completed using %n jumper(s)", "", jumpers);
		}
	} else {
		theText = tr("%1 of %2 nets routed - %n connector(s) still to be routed", "", connectorsLeftToRoute)
			.arg(netRoutedCount)
			.arg(netCount);
	}

	dynamic_cast<SketchAreaWidget *>(sketchWidget->parent())->routingStatusLabel()->setLabelText(theText);

	updateTraceMenu();
}


void MainWindow::clearRoutingSlot(SketchWidget * sketchWidget, QUndoCommand * parentCommand) {
	Q_UNUSED(sketchWidget);

	m_pcbGraphicsView->clearRouting(parentCommand);
}


void MainWindow::applyReadOnlyChange(bool isReadOnly) {
	Q_UNUSED(isReadOnly);
	//m_saveAct->setDisabled(isReadOnly);
}

void MainWindow::currentNavigatorChanged(MiniViewContainer * miniView)
{
	int index = m_navigators.indexOf(miniView);
	if (index < 0) return;

	int oldIndex = m_tabWidget->currentIndex();
	if (oldIndex == index) return;

	this->m_tabWidget->setCurrentIndex(index);
}

void MainWindow::viewSwitchedTo(int viewIndex) {
	m_tabWidget->setCurrentIndex(viewIndex);
}

const QString MainWindow::fritzingTitle() {
	if (m_currentGraphicsView == NULL) {
		return FritzingWindow::fritzingTitle();
	}

	QString fritzing = FritzingWindow::fritzingTitle();
	return tr("%1 - [%2]").arg(fritzing).arg(m_currentGraphicsView->viewName());
}

QAction *MainWindow::raiseWindowAction() {
	return m_raiseWindowAct;
}

void MainWindow::raiseAndActivate() {
	if(isMinimized()) {
		showNormal();
	}
	raise();
	QTimer::singleShot(20, this, SLOT(activateWindowAux()));
}

void MainWindow::activateWindowAux() {
	activateWindow();
}

void MainWindow::updateRaiseWindowAction() {
	QString actionText;
	QFileInfo fileInfo(m_fileName);
	if(fileInfo.exists()) {
		int lastSlashIdx = m_fileName.lastIndexOf("/");
		int beforeLastSlashIdx = m_fileName.left(lastSlashIdx).lastIndexOf("/");
		actionText = beforeLastSlashIdx > -1 && lastSlashIdx > -1 ? "..." : "";
		actionText += m_fileName.right(m_fileName.size()-beforeLastSlashIdx-1);
	} else {
		actionText = m_fileName;
	}
	m_raiseWindowAct->setText(actionText);
	m_raiseWindowAct->setToolTip(m_fileName);
	m_raiseWindowAct->setStatusTip("raise \""+m_fileName+"\" window");
}

QSizeGrip *MainWindow::sizeGrip() {
	return m_sizeGrip;
}

QStatusBar *MainWindow::realStatusBar() {
	return m_statusBar;
}

void MainWindow::moveEvent(QMoveEvent * event) {
	FritzingWindow::moveEvent(event);
	emit mainWindowMoved(this);
}

bool MainWindow::event(QEvent * e) {
	switch (e->type()) {
		case QEvent::WindowActivate:
			emit changeActivationSignal(true, this);
			break;
		case QEvent::WindowDeactivate:
			emit changeActivationSignal(false, this);
			break;
		default:
			break;
	}
	return FritzingWindow::event(e);
}

void MainWindow::resizeEvent(QResizeEvent * event) {
	m_sizeGrip->rearrange();
	FritzingWindow::resizeEvent(event);
}

void MainWindow::showInViewHelp() {
	//delete m_helper;
	if (m_helper == NULL) {
		m_helper = new Helper(this, true);
		return;
	}

	bool toggle = !m_helper->helpVisible(m_tabWidget->currentIndex());
	showAllFirstTimeHelp(toggle);

	/*
	m_helper->toggleHelpVisibility(m_tabWidget->currentIndex());
	*/

	m_showInViewHelpAct->setChecked(m_helper->helpVisible(m_tabWidget->currentIndex()));
}


void MainWindow::showAllFirstTimeHelp(bool show) {
	if (m_helper) {
		for (int i = 0; i < 3; i++) {
			m_helper->setHelpVisibility(i, show);
		}
	}
	m_showInViewHelpAct->setChecked(show);
}

void MainWindow::enableCheckUpdates(bool enabled)
{
	if (m_checkForUpdatesAct != NULL) {
		m_checkForUpdatesAct->setEnabled(enabled);
	}
}

void MainWindow::editModule() {
	ItemBase * itemBase = NULL;
	QList<QGraphicsItem *> items = m_currentGraphicsView->scene()->selectedItems();
	if (items.count() == 1) {
		ItemBase * item = dynamic_cast<ItemBase *>(items[0]);
		if (item != NULL) {
			itemBase = (item->itemType() == ModelPart::Module) ? item : NULL;
		}
	}

	if (itemBase == NULL) return;

    MainWindow* mw = newMainWindow(m_paletteModel, m_refModel, itemBase->modelPartShared()->path(), true);
	mw->loadWhich(itemBase->modelPartShared()->path(), false, false);
	mw->m_sketchModel->walk(mw->m_sketchModel->root(), 0);
    mw->clearFileProgressDialog();
	closeIfEmptySketch(mw);
}

void MainWindow::saveAsModule()
{
	if (!m_pcbGraphicsView->ratsAllRouted()) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Before you can save a sketch as a module, all traces must be routed."));
		return;
	}

	QList<ConnectorItem *> externalConnectors;
	initExternalConnectors(externalConnectors);

	SaveAsModuleDialog dialog(m_breadboardGraphicsView, externalConnectors, this);
	if (dialog.exec() != QDialog::Accepted) return;

	QHash<QString, QString> svgs;

	QList<ViewLayer::ViewLayerID> partViewLayerIDs;
	QList<ViewLayer::ViewLayerID> wireViewLayerIDs;

	partViewLayerIDs << ViewLayer::BreadboardBreadboard << ViewLayer::Breadboard;
	wireViewLayerIDs << ViewLayer::BreadboardWire;
	QString svg1 = genIcon(m_breadboardGraphicsView, partViewLayerIDs, wireViewLayerIDs);
	svgs.insert("breadboard", svg1);

	partViewLayerIDs.clear();
	wireViewLayerIDs.clear();
	partViewLayerIDs << ViewLayer::Schematic;
	wireViewLayerIDs << ViewLayer::SchematicWire;
	QString svg2 = genIcon(m_schematicGraphicsView, partViewLayerIDs, wireViewLayerIDs);
	svgs.insert("schematic", svg2);

	partViewLayerIDs.clear();
	wireViewLayerIDs.clear();
	partViewLayerIDs << ViewLayer::GroundPlane << ViewLayer::Copper0  << ViewLayer::Copper0Trace << ViewLayer::Silkscreen;   // TODO: what layers should be visible
	wireViewLayerIDs << ViewLayer::Jumperwires << ViewLayer::Ratsnest;
	QString svg3 = genIcon(m_pcbGraphicsView, partViewLayerIDs, wireViewLayerIDs);
	svgs.insert("pcb", svg3);

	foreach (QString svg, svgs.values()) {
		if (svg.isEmpty()) {
			// tell the user
			return;
		}
	}

	SketchModel partSketchModel(true);
	ModelPartShared modelPartShared;
	partSketchModel.root()->setModelPartShared(&modelPartShared);

	QString moduleID;
	QString uri;
	QString version("1");

	if(moduleID.isNull() || moduleID.isEmpty()) {
		moduleID = FritzingWindow::getRandText();
	}

	modelPartShared.setModuleID(moduleID);
	modelPartShared.setUri(uri);
	modelPartShared.setVersion(version);

	modelPartShared.setAuthor(dialog.author());
	modelPartShared.setTitle(dialog.title());
	modelPartShared.setDate(dialog.createdOn());
	modelPartShared.setLabel(dialog.label());
	modelPartShared.setDescription(dialog.description());
	modelPartShared.setTags(dialog.tags());
	modelPartShared.setProperties(dialog.properties());

	QByteArray partXml;
	QXmlStreamWriter partStreamWriter(&partXml);

	partSketchModel.save(partStreamWriter, true);				// get part xml

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument partDocument;
	bool result = partDocument.setContent(partXml, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		// signal error
		return;
	}

	QDomElement partModule = partDocument.documentElement();
	if (partModule.isNull()) {
		// signal error
		return;
	}

	QByteArray sketchXml;
	QXmlStreamWriter sketchStreamWriter(&sketchXml);
	m_sketchModel->save(sketchStreamWriter, false);				// get sketch xml

	QDomDocument sketchDocument;
	result = sketchDocument.setContent(sketchXml, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		// signal error
		return;
	}

	QDomElement sketchModule = sketchDocument.documentElement();
	if (sketchModule.isNull()) {
		// signal error
		return;
	}

	QDomElement instances = sketchModule.firstChildElement("instances");
   	if (instances.isNull()) {
		// signal error
   		return;
	}

	sketchModule.removeChild(instances);
	partModule.appendChild(instances);

	QString userPartsSvgFolderPath = FolderUtils::getUserDataStorePath("parts")+"/svg/user/";
	foreach (QString view, svgs.keys()) {
		QFile file(userPartsSvgFolderPath + view + "/" + moduleID + ".svg");
		file.open(QIODevice::WriteOnly);
		QTextStream out(&file);
		out << svgs.value(view, "");
		file.close();
	}

	svgs.insert("pcb", "pcb");
	svgs.insert("schematic", "schematic");
	svgs.insert("breadboard", "breadboard");
	svgs.insert("icon", "breadboard");

	QHash<QString, QString> layerids(svgs);
	layerids.insert("icon", "icon");
	layerids.insert("pcb", "groundplane");
	layerids.insert("pcb", "copper0");
	layerids.insert("pcb", "copper0trace");

	QDomElement views = partDocument.createElement("views");
	partModule.appendChild(views);

	foreach (QString view, svgs.keys()) {
		QString imagePath = svgs.value(view) + "/"  + moduleID + ".svg";

		QDomElement viewElement = partDocument.createElement(view + "View");
		views.appendChild(viewElement);
		QDomElement layers = partDocument.createElement("layers");
		viewElement.appendChild(layers);
		layers.setAttribute("image", imagePath);
		QDomElement layer = partDocument.createElement("layer");
		layers.appendChild(layer);
		layer.setAttribute("layerId", layerids.value(view));
	}

	QByteArray tempXml;
	QXmlStreamWriter tempStreamWriter(&tempXml);
	tempStreamWriter.writeStartDocument();
	tempStreamWriter.writeStartElement("module");
	tempStreamWriter.writeStartElement("externals");
	foreach (ConnectorItem * connectorItem, dialog.externalConnectorItems()) {
		connectorItem->writeConnector(tempStreamWriter, "external");
	}
	tempStreamWriter.writeEndElement();
	tempStreamWriter.writeEndElement();
	tempStreamWriter.writeEndDocument();

	QDomDocument tempDocument;
	result = tempDocument.setContent(tempXml, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		// signal error
		return;
	}

	QDomElement tempModule = tempDocument.documentElement();
	if (tempModule.isNull()) {
		// signal error
		return;
	}

	QDomElement externals = tempModule.firstChildElement("externals");
   	if (externals.isNull()) {
		// signal error
   		return;
	}

	tempModule.removeChild(externals);
	partModule.appendChild(externals);

	QString userPartsFolderPath = FolderUtils::getUserDataStorePath("parts")+"/user/";

	QFile file2(userPartsFolderPath + moduleID + FritzingModuleExtension);
	file2.open(QIODevice::WriteOnly);
	QTextStream out2(&file2);
	partDocument.save(out2, 0);
	file2.close();

	loadPart(userPartsFolderPath + moduleID + FritzingModuleExtension);

}

void MainWindow::initExternalConnectors(QList<ConnectorItem *> & externalConnectors) {
	QFile file(m_fileName);

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument domDocument;

	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		return;
	}

	if (root.tagName() != "module") {
		return;
	}

	QDomElement externals = root.firstChildElement("externals");
	if (externals.isNull()) return;

	QDomElement external = externals.firstChildElement("external");
	while (!external.isNull()) {
		ModelPart * mp = NULL;
		bool ok;
		QString connectorID = external.attribute("connectorId");
		long oldModelIndex = external.attribute("modelIndex").toLong(&ok);
		if (ok) {
			mp = m_sketchModel->findModelPartFromOriginal(m_sketchModel->root(), oldModelIndex);
		}
		else {
			// we're connected to something inside a module; fixup the first modelIndex
			QDomElement p = external.firstChildElement("mp");
			if (!p.isNull()) {
				oldModelIndex = p.attribute("i").toLong();
				mp = m_sketchModel->findModelPartFromOriginal(m_sketchModel->root(), oldModelIndex);
				if (mp != NULL) {
					while (true) {
						p = p.firstChildElement("mp");
						if (p.isNull()) break;

						mp = m_sketchModel->findModelPartFromOriginal(mp, p.attribute("i").toLong());
						if (mp == NULL) break;
					}
				}
			}
		}
		if (mp != NULL) {
			ItemBase * itemBase = mp->viewItem(m_breadboardGraphicsView->scene());
			if (itemBase != NULL) {
				ConnectorItem * connectorItem = itemBase->findConnectorItemNamed(connectorID);
				if (connectorItem != NULL) {
					externalConnectors.append(connectorItem);
				}
			}
		}

		external = external.nextSiblingElement("external");
	}
}

QString MainWindow::genIcon(SketchWidget * sketchWidget, QList<ViewLayer::ViewLayerID> &  partViewLayerIDs, QList<ViewLayer::ViewLayerID> & wireViewLayerIDs) {
	QSizeF imageSize;
	return sketchWidget->renderToSVG(FSvgRenderer::printerScale(), partViewLayerIDs, wireViewLayerIDs, false, imageSize, NULL, GraphicsUtils::StandardFritzingDPI, false, false);
}

void MainWindow::swapSelectedMap(const QString & family, const QString & prop, QMap<QString, QString> & currPropsMap) 
{
	if (swapSpecial(currPropsMap)) {
		return;
	}

	foreach (QString key, currPropsMap.keys()) {
		QString value = currPropsMap.value(key);
		m_refModel->recordProperty(key, value);
	}

	QString moduleID = m_refModel->retrieveModuleIdWith(family, prop, true);
	bool exactMatch = m_refModel->lastWasExactMatch();

	if(moduleID == ___emptyString___) {
		QMessageBox::information(
			this,
			tr("Sorry!"),
			tr(
			 "No part with those characteristics.\n"
			 "We're working to avoid this message, and only let you choose between properties that do exist")
		);
		return;
	}

	ItemBase * itemBase = m_infoView->currentItem();
	if (itemBase == NULL) return;

	itemBase = itemBase->layerKinChief();

	if(!exactMatch) {
		AutoCloseMessageBox * acmb = new AutoCloseMessageBox(this);
		acmb->setText(tr("No exactly matching part found; Fritzing chose the closest match."));
		if (m_statusBar != NULL) {
			QRect dest = m_statusBar->geometry(); // toolbar->geometry();
			QRect r = this->geometry();
			acmb->setFixedSize(QSize(dest.width(), dest.height()));
			QPoint p(dest.x(), dest.y());
			p = m_statusBar->parentWidget()->mapTo(this, p);
			acmb->setStartPos(p.x(), r.height());
			acmb->setEndPos(p.x(), p.y());
			acmb->start();
		}
	}

	swapSelectedAux(itemBase, moduleID);
}

bool MainWindow::swapSpecial(QMap<QString, QString> & currPropsMap) {
	ItemBase * itemBase = m_infoView->currentItem();
	QString pinSpacing, resistance;
	foreach (QString key, currPropsMap.keys()) {
		if (key.compare("shape", Qt::CaseInsensitive) == 0) {
			ResizableBoard * board = dynamic_cast<ResizableBoard *>(itemBase);
			if (board == NULL) continue;

			QString value = currPropsMap.value(key, "");
			if (value.compare(ResizableBoard::customShapeTranslated) == 0) {
				if (!loadCustomBoardShape()) {
					
					// restores the infoview size menu
					m_currentGraphicsView->viewItemInfo(itemBase);
				}
				return true;
			}
		}

		if (key.compare("form", Qt::CaseInsensitive) == 0) {
			PinHeader * pinHeader = dynamic_cast<PinHeader *>(itemBase);
			if (pinHeader == NULL) continue;

			if (pinHeader->onlyFormChanges(currPropsMap)) {
				m_currentGraphicsView->setForm(currPropsMap.value(key));
				return true;
			}

			continue;
		}

		if (key.compare("spacing", Qt::CaseInsensitive) == 0) {
			MysteryPart * mysteryPart = dynamic_cast<MysteryPart *>(itemBase);
			if (mysteryPart == NULL) continue;

			if (mysteryPart->onlySpacingChanges(currPropsMap)) {
				m_currentGraphicsView->setSpacing(currPropsMap.value(key));
				return true;
			}

			continue;
		}

		if (key.compare("resistance", Qt::CaseInsensitive) == 0) {
			resistance = currPropsMap.value(key);
			continue;
		}
		if (key.compare("pin spacing", Qt::CaseInsensitive) == 0) {
			pinSpacing = currPropsMap.value(key);
			continue;
		}
	}

	if (!resistance.isEmpty() || !pinSpacing.isEmpty()) {
		Resistor * resistor = dynamic_cast<Resistor *>(itemBase);
		if (resistor != NULL) {
			m_currentGraphicsView->setResistance(resistance, pinSpacing);
			return true;
		}
	}

	return false;
}

void MainWindow::swapSelectedAux(ItemBase * itemBase, const QString & moduleID) {

	QUndoCommand* parentCommand = new QUndoCommand(tr("Swapped %1 with module %2").arg(itemBase->instanceTitle()).arg(moduleID));
	swapSelectedAuxAux(itemBase, moduleID, parentCommand);
	// need to defer execution so the content of the info view doesn't change during an event that started in the info view
	m_undoStack->waitPush(parentCommand, 10);
}


long MainWindow::swapSelectedAuxAux(ItemBase * itemBase, const QString & moduleID, QUndoCommand * parentCommand) 
{
	long modelIndex = ModelPart::nextIndex();

	QList<bool> masterflags;
	masterflags << false << false << false;
	if (itemBase->modelPart()->viewItem(m_breadboardGraphicsView->scene()) != NULL) {
		masterflags[2] = true;
	}
	else if (itemBase->modelPart()->viewItem(m_pcbGraphicsView->scene()) != NULL) {
		masterflags[1] = true;
	}
	else {
		masterflags[0] = true;
	}

	long newID1 = m_schematicGraphicsView->setUpSwap(itemBase->id(), modelIndex, moduleID, masterflags[0], parentCommand);
	long newID2 = m_pcbGraphicsView->setUpSwap(itemBase->id(), modelIndex, moduleID, masterflags[1], parentCommand);

	// master view must go last, since it creates the delete command
	long newID3 = m_breadboardGraphicsView->setUpSwap(itemBase->id(), modelIndex, moduleID, masterflags[2], parentCommand);

	// TODO:  z-order?

	if (newID3 != 0) return newID3;
	if (newID2 != 0) return newID2;
	return newID1;
}

bool MainWindow::loadCustomBoardShape()
{
	ItemBase * itemBase = m_infoView->currentItem();
	if (itemBase == NULL) return false;

	itemBase = itemBase->layerKinChief();

	QString path = FolderUtils::getOpenFileName(this,
		tr("Open custom board shape SVG file"),
		defaultSaveFolder(),
		tr("SVG Files (%1)").arg("*.svg")
	);

	if (path.isEmpty()) {
		return false; // Cancel pressed
	}

	SvgFileSplitter splitter;
	if (!splitter.split(path, "board")) {
		svgMissingLayer("board", path);
		return false;
	}

	if (!splitter.split(path, "silkscreen")) {
		svgMissingLayer("silkscreen", path);
		return false;
	}

	QString wStr, hStr, vbStr;
	if (!SvgFileSplitter::getSvgSizeAttributes(path, wStr, hStr, vbStr)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Svg file '%1' is missing width, height, or viewbox attribute").arg(path)
		);
		return false;
	}

	bool ok;
	qreal w = TextUtils::convertToInches(wStr, &ok);
	if (!ok) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Svg file '%1': bad width attribute").arg(path)
		);
		return false;
	}

	qreal h = TextUtils::convertToInches(hStr, &ok);
	if (!ok) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Svg file '%1': bad height attribute").arg(path)
		);
		return false;
	}


	QString moduleID = FritzingWindow::getRandText();
	QString userPartsSvgFolderPath = FolderUtils::getUserDataStorePath("parts")+"/svg/user/";
	QString newName = userPartsSvgFolderPath + "pcb" + "/" + moduleID + ".svg";
	bool result = QFile(path).copy(newName);
	if (result == false) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Sorry, Fritzing is unable to copy the svg file.")
		);
		return false;
	}

	QFile file(":/resources/templates/resizableBoard_fzpTemplate.txt");
	file.open(QFile::ReadOnly);
	QString fzpTemplate = file.readAll();
	file.close();

	if (fzpTemplate.isEmpty()) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Sorry, Fritzing is unable to load the part template file.")
		);
		return false;
	}

	// %1 = author
	// %2 = width
	// %3 = height
	// %4 = filename (minus path and extension)
	// %5 = date string
	// %6 = module id
	// %7 = time string

	QString fzp = fzpTemplate
		.arg(getenvUser())
		.arg(w * 25.4)
		.arg(h * 25.4)
		.arg(QFileInfo(path).baseName())
		.arg(QDate::currentDate().toString(Qt::ISODate))
		.arg(moduleID)
		.arg(QTime::currentTime().toString("HH:mm:ss"));


	QString userPartsFolderPath = FolderUtils::getUserDataStorePath("parts")+"/user/";
	QFile file2(userPartsFolderPath + moduleID + FritzingPartExtension);
	file2.open(QIODevice::WriteOnly);
	QTextStream out2(&file2);
	out2 << fzp;
	file2.close();

	loadPart(userPartsFolderPath + moduleID + FritzingPartExtension);

	swapSelectedAux(itemBase, moduleID);

	itemBase->resetValues(itemBase->modelPart()->properties().value("family", "").toLower(), "shape");

	return true;
}

void MainWindow::svgMissingLayer(const QString & layername, const QString & path) {
	QMessageBox::warning(
		this,
		tr("Fritzing"),
		tr("Svg %1 is missing a '%2' layer. "
			"For more information on how to create a custom board shape, "
			"see the tutorial at <a href='http://fritzing.org/learning/tutorials/designing-pcb/pcb-custom-shape/'>http://fritzing.org/learning/tutorials/designing-pcb/pcb-custom-shape/</a>.")
		.arg(path)
		.arg(layername)
	);
}

void MainWindow::addBoard() {
	if (m_pcbGraphicsView == NULL) return;

	m_pcbGraphicsView->addBoard();
}

MainWindow * MainWindow::newMainWindow(PaletteModel * paletteModel, ReferenceModel *refModel, const QString & path, bool showProgress) {
	MainWindow * mw = new MainWindow(paletteModel, refModel);
	if (showProgress) {
		mw->showFileProgressDialog(path);
	}

	mw->init();
	return mw;
}

void  MainWindow::clearFileProgressDialog() {
	if (m_fileProgressDialog) {
		m_fileProgressDialog->close();
		delete m_fileProgressDialog;
		m_fileProgressDialog = NULL;
	}
}

void MainWindow::showFileProgressDialog(const QString & path) {
	m_fileProgressDialog = new FileProgressDialog("Loading...", 100, this);
	if (!path.isEmpty()) {
		m_fileProgressDialog->setMessage(QString("loading %1").arg(QFileInfo(path).baseName()));
	}
}

const QString &MainWindow::selectedModuleID() {
	if(m_currentGraphicsView) {
		return m_currentGraphicsView->selectedModuleID();
	} else {
		return ___emptyString___;
	}
}

void MainWindow::redrawSketch() {
	foreach (QGraphicsItem * item, m_currentGraphicsView->scene()->items()) {
		item->update();
		ConnectorItem * c = dynamic_cast<ConnectorItem *>(item);
		if (c != NULL) {
			c->restoreColor(false, -1);
		}
	}
}

void MainWindow::statusMessage(QString message, int timeout) {
	QStatusBar * sb = realStatusBar();
	if (sb != NULL) {
		sb->showMessage(message, timeout);
	}
}

