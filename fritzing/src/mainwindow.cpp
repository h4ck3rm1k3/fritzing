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
#include <QWebView>
#include <QStackedWidget>
#include <QXmlStreamReader>

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
#include "layerpalette.h"
#include "items/paletteitem.h"

#include "help/helper.h"
#include "dockmanager.h"

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
#include "utils/zoomslider.h"

const QString MainWindow::UntitledSketchName = "Untitled Sketch";
int MainWindow::UntitledSketchIndex = 1;
int MainWindow::CascadeFactorX = 21;
int MainWindow::CascadeFactorY = 19;
int MainWindow::RestartNeeded = 0;

static const int MainWindowDefaultWidth = 840;
static const int MainWindowDefaultHeight = 600;

int MainWindow::AutosaveTimeoutMinutes = 10;   // in minutes
bool MainWindow::AutosaveEnabled = true;
QString MainWindow::BackupFolder;

MainWindow::MainWindow(PaletteModel * paletteModel, ReferenceModel *refModel) :
	FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension())
{
	m_activeLayerButtonWidget = NULL;
	m_programWindow = NULL;
	m_windowMenuSeparator = NULL;
	m_wireColorMenu = NULL;
	m_viewSwitcherDock = NULL;
	m_checkForUpdatesAct = NULL;
	m_fileProgressDialog = NULL;
	m_currentGraphicsView = NULL;
	m_comboboxChanged = false;
	m_helper = NULL;
	m_recovered = false;

    // Add a timer for autosaving
	m_backingUp = m_autosaveNeeded = false;
    connect(&m_autosaveTimer, SIGNAL(timeout()), this, SLOT(backupSketch()));
    m_autosaveTimer.start(AutosaveTimeoutMinutes * 60 * 1000);

	resize(MainWindowDefaultWidth, MainWindowDefaultHeight);

	m_backupFileNameAndPath = MainWindow::BackupFolder + "/" + FolderUtils::getRandText() + fileExtension();
    // Connect the undoStack to our autosave stuff
    connect(m_undoStack, SIGNAL(indexChanged(int)), this, SLOT(autosaveNeeded(int)));
    connect(m_undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(undoStackCleanChanged(bool)));

	// Create dot icons
	m_dotIcon = QIcon(":/resources/images/dot.png");
	m_emptyIcon = QIcon();

	m_currentWidget = NULL;
	m_firstOpen = true;

	m_statusBar = new QStatusBar(this);
	setStatusBar(m_statusBar);
	m_statusBar->setSizeGripEnabled(false);
	m_zoomSlider = new ZoomSlider(m_statusBar);
	connect(m_zoomSlider, SIGNAL(zoomChanged(qreal)), this, SLOT(updateViewZoom(qreal)));
	m_statusBar->addPermanentWidget(m_zoomSlider);

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

	m_layerPalette = new LayerPalette(this);

    m_dockManager = new DockManager(this);
    m_dockManager->createBinAndInfoViewDocks();

	// This is the magic translation that changes all the shortcut text on the menu items
	// to the native language instead of "Ctrl", so the German menu items will now read "Strg"
	// You don't actually have to translate every menu item in the .ts file, you can just leave it as "Ctrl".
	QShortcut::tr("Ctrl", "for naming shortcut keys on menu items");
	QShortcut::tr("Alt", "for naming shortcut keys on menu items");
	QShortcut::tr("Shift", "for naming shortcut keys on menu items");
	QShortcut::tr("Meta", "for naming shortcut keys on menu items");

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

	m_layerPalette->setShowAllLayersAction(m_showAllLayersAct);
	m_layerPalette->setHideAllLayersAction(m_hideAllLayersAct);

	m_viewSwitcher = new ViewSwitcher();
	connect(m_viewSwitcher, SIGNAL(viewSwitched(int)), this, SLOT(viewSwitchedTo(int)));
	connect(this, SIGNAL(viewSwitched(int)), m_viewSwitcher, SLOT(viewSwitchedTo(int)));
	m_viewSwitcher->viewSwitchedTo(0);

    m_dockManager->createDockWindows();

	createZoomOptions(m_breadboardWidget);
	createZoomOptions(m_schematicWidget);
	createZoomOptions(m_pcbWidget);

    m_breadboardWidget->setToolbarWidgets(getButtonsForView(m_breadboardWidget->viewIdentifier()));
    m_schematicWidget->setToolbarWidgets(getButtonsForView(m_schematicWidget->viewIdentifier()));
	m_pcbWidget->setToolbarWidgets(getButtonsForView(m_pcbWidget->viewIdentifier()));

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
	connect(m_tabWidget, SIGNAL(currentChanged ( int )), this, SLOT(tabWidget_currentChanged( int )));

	connectPairs();

	m_helper = new Helper(this, true);

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

	m_setUpDockManagerTimer.setSingleShot(true);
	connect(&m_setUpDockManagerTimer, SIGNAL(timeout()), m_dockManager, SLOT(keepMargins()));
	m_setUpDockManagerTimer.start(1000);

	m_pcbGraphicsView->jumperItemHack();
}

MainWindow::~MainWindow()
{
    // Delete backup of this sketch if one exists.
    QFile::remove(m_backupFileNameAndPath);	
	
	delete m_sketchModel;
	m_dockManager->dontKeepMargins();
	m_setUpDockManagerTimer.stop();

	foreach (LinkedFile * linkedFile, m_linkedProgramFiles) {
		delete linkedFile;
	}
	m_linkedProgramFiles.clear();
}


void MainWindow::initSketchWidget(SketchWidget * sketchWidget) {
	sketchWidget->setPaletteModel(m_paletteModel);
	sketchWidget->setSketchModel(m_sketchModel);
	sketchWidget->setRefModel(m_refModel);
	sketchWidget->setUndoStack(m_undoStack);
	sketchWidget->setChainDrag(true);			// enable bend points
	sketchWidget->initGrid();
	sketchWidget->addViewLayers();
}

void MainWindow::connectPairs() {
	connectPair(m_breadboardGraphicsView, m_schematicGraphicsView);
	connectPair(m_breadboardGraphicsView, m_pcbGraphicsView);
	connectPair(m_schematicGraphicsView, m_breadboardGraphicsView);
	connectPair(m_schematicGraphicsView, m_pcbGraphicsView);
	connectPair(m_pcbGraphicsView, m_breadboardGraphicsView);
	connectPair(m_pcbGraphicsView, m_schematicGraphicsView);

	bool succeeded = connect(m_pcbGraphicsView, SIGNAL(routingStatusSignal(SketchWidget *, const RoutingStatus &)),
						this, SLOT(routingStatusSlot(SketchWidget *, const RoutingStatus &)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(routingStatusSignal(SketchWidget *, const RoutingStatus &)),
						this, SLOT(routingStatusSlot(SketchWidget *, const RoutingStatus &)));

	succeeded = connect(m_breadboardGraphicsView, SIGNAL(swapSignal(const QString &, const QString &, QMap<QString, QString> &)), 
						this, SLOT(swapSelectedMap(const QString &, const QString &, QMap<QString, QString> &)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(swapSignal(const QString &, const QString &, QMap<QString, QString> &)), 
						this, SLOT(swapSelectedMap(const QString &, const QString &, QMap<QString, QString> &)));
	succeeded = connect(m_pcbGraphicsView, SIGNAL(swapSignal(const QString &, const QString &, QMap<QString, QString> &)), 
						this, SLOT(swapSelectedMap(const QString &, const QString &, QMap<QString, QString> &)));

	succeeded = connect(m_breadboardGraphicsView, SIGNAL(dropPasteSignal(SketchWidget *)), 
						this, SLOT(dropPaste(SketchWidget *)));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(dropPasteSignal(SketchWidget *)), 
						this, SLOT(dropPaste(SketchWidget *)));
	succeeded = connect(m_pcbGraphicsView, SIGNAL(dropPasteSignal(SketchWidget *)), 
						this, SLOT(dropPaste(SketchWidget *)));
	
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

	succeeded = connect(m_pcbGraphicsView, SIGNAL(subSwapSignal(SketchWidget *, ItemBase *, ViewLayer::ViewLayerSpec, QUndoCommand *)),
						this, SLOT(subSwapSlot(SketchWidget *, ItemBase *, ViewLayer::ViewLayerSpec, QUndoCommand *)),
						Qt::DirectConnection);


	succeeded = connect(m_pcbGraphicsView, SIGNAL(firstTimeHelpHidden()), this, SLOT(firstTimeHelpHidden()));
	succeeded = connect(m_schematicGraphicsView, SIGNAL(firstTimeHelpHidden()), this, SLOT(firstTimeHelpHidden()));
	succeeded = connect(m_breadboardGraphicsView, SIGNAL(firstTimeHelpHidden()), this, SLOT(firstTimeHelpHidden()));

	succeeded = connect(m_pcbGraphicsView, SIGNAL(updateLayerMenuSignal()), this, SLOT(updateLayerMenuSlot()));
	succeeded = connect(m_pcbGraphicsView, SIGNAL(changeBoardLayersSignal(int, bool )), this, SLOT(changeBoardLayers(int, bool )));



	/*
	succeeded = connect(m_schematicGraphicsView, SIGNAL(schematicDisconnectWireSignal(ConnectorPairHash &, QSet<ItemBase *> &, QHash<ItemBase *, ConnectorPairHash *> &, QUndoCommand *)),
						m_breadboardGraphicsView, SLOT(schematicDisconnectWireSlot(ConnectorPairHash &, QSet<ItemBase *> &, QHash<ItemBase *, ConnectorPairHash *> &, QUndoCommand *)),
						Qt::DirectConnection);
	*/

	succeeded = connect(m_schematicGraphicsView, SIGNAL(disconnectWireSignal(QSet<ItemBase *> &)),
					m_breadboardGraphicsView, SLOT(disconnectWireSlot(QSet<ItemBase *> &)),
					Qt::DirectConnection);
	succeeded = connect(m_pcbGraphicsView, SIGNAL(disconnectWireSignal(QSet<ItemBase *> &)),
					m_breadboardGraphicsView, SLOT(disconnectWireSlot(QSet<ItemBase *> &)),
					Qt::DirectConnection);

	succeeded = connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_breadboardGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
	succeeded = connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_schematicGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
	succeeded = connect(qApp, SIGNAL(spaceBarIsPressedSignal(bool)), m_pcbGraphicsView, SLOT(spaceBarIsPressedSlot(bool)));
}

void MainWindow::connectPair(SketchWidget * signaller, SketchWidget * slotter)
{

	bool succeeded = connect(signaller, SIGNAL(itemAddedSignal(ModelPart *, ViewLayer::ViewLayerSpec, const ViewGeometry &, long, SketchWidget *)),
							 slotter, SLOT(sketchWidget_itemAdded(ModelPart *, ViewLayer::ViewLayerSpec, const ViewGeometry &, long, SketchWidget *)));

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
	succeeded = succeeded && connect(signaller, SIGNAL(changeConnectionSignal(long,  QString, long,  QString, ViewLayer::ViewLayerSpec, bool, bool)),
									 slotter, SLOT(sketchWidget_changeConnection(long, QString, long, QString, ViewLayer::ViewLayerSpec, bool, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(copyBoundingRectsSignal(QHash<QString, QRectF> &)),
													   slotter, SLOT(copyBoundingRectsSlot(QHash<QString, QRectF> &)),
									 Qt::DirectConnection);

	succeeded = succeeded && connect(signaller, SIGNAL(cleanUpWiresSignal(CleanUpWiresCommand *)),
									 slotter, SLOT(sketchWidget_cleanUpWires(CleanUpWiresCommand *)) );
	succeeded = succeeded && connect(signaller, SIGNAL(dealWithRatsnestSignal(long, const QString &,
																			  long, const QString &,
																			  ViewLayer::ViewLayerSpec,
																			  bool, RatsnestCommand *)),
									 slotter, SLOT(dealWithRatsnestSlot(long, const QString &,
																			  long, const QString &,
																			  ViewLayer::ViewLayerSpec,
																			  bool, RatsnestCommand *)) );

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

	succeeded = succeeded && connect(signaller, SIGNAL(changeBoardLayersSignal(int, bool )),
									 slotter, SLOT(changeBoardLayers(int, bool )));


	if (!succeeded) {
		DebugDialog::debug("connectPair failed");
	}

}


void MainWindow::setCurrentFile(const QString &fileName, bool addToRecent, bool recovered, const QString & backupName) {
	m_fileName = fileName;

    // If this is an untitled sketch, increment the untitled sketch number
    // to something reasonable.
	if (recovered) {
		if (fileName.startsWith(untitledFileName())) {
			DebugDialog::debug(QString("Comparing untitled documents: %1 %2").arg(fileName).arg(untitledFileName()));
			QRegExp regexp("\\d+");
			int ix = regexp.indexIn(fileName);
			int untitledSketchNumber = ix >= 0 ? regexp.cap(0).toInt() : 1;
			untitledSketchNumber++;
			DebugDialog::debug(QString("%1 untitled documents open, currently thinking %2").arg(untitledSketchNumber).arg(UntitledSketchIndex));
			UntitledSketchIndex = UntitledSketchIndex >= untitledSketchNumber ? UntitledSketchIndex : untitledSketchNumber;
		}
		else {
			// sketch files aren't actually set to read-only
			QString examplesPath = FolderUtils::getApplicationSubFolderPath("sketches");
			if (fileName.contains(examplesPath, Qt::CaseInsensitive)) {
				// If the file is read-only be sure to set this
				setReadOnly(true);
			}
		}

		// If this was a sketch restored from the backup directory, preserve the
		// backup file name
        m_backupFileNameAndPath = backupName;
    }

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


void MainWindow::createZoomOptions(SketchAreaWidget* parent) {

    connect(parent->graphicsView(), SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomSlider(qreal)));
    connect(parent->graphicsView(), SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));
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

	connect(routingStatusLabel, SIGNAL(mousePressSignal(QMouseEvent*)), this, SLOT(routingStatusLabelMousePress(QMouseEvent*)));
	connect(routingStatusLabel, SIGNAL(mouseReleaseSignal(QMouseEvent*)), this, SLOT(routingStatusLabelMouseRelease(QMouseEvent*)));

	routingStatusLabel->setTextInteractionFlags(Qt::NoTextInteraction);
	routingStatusLabel->setCursor(Qt::ArrowCursor);
	routingStatusLabel->viewport()->setCursor(Qt::ArrowCursor);

	routingStatusLabel->setObjectName(SketchAreaWidget::RoutingStateLabelName);
	parent->setRoutingStatusLabel(routingStatusLabel);
	RoutingStatus routingStatus;
	routingStatus.zero();
	routingStatusSlot(parent->graphicsView(), routingStatus);
	return routingStatusLabel;
}

SketchToolButton *MainWindow::createRotateButton(SketchAreaWidget *parent) {
	QList<QAction*> rotateMenuActions;
	rotateMenuActions << m_rotate45ccwAct << m_rotate90ccwAct << m_rotate180Act << m_rotate90cwAct << m_rotate45cwAct;
	SketchToolButton * rotateButton = new SketchToolButton("Rotate",parent, rotateMenuActions);
	rotateButton->setDefaultAction(m_rotate90ccwAct);
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

QWidget *MainWindow::createActiveLayerButton(SketchAreaWidget *parent) 
{
	QList<QAction *> actions;
	actions << m_activeLayerBothAct << m_activeLayerBottomAct << m_activeLayerTopAct;

	m_activeLayerButtonWidget = new QStackedWidget;
	m_activeLayerButtonWidget->setMaximumWidth(90);

	SketchToolButton * button = new SketchToolButton("ActiveLayer", parent, actions);
	button->setDefaultAction(m_activeLayerBottomAct);
	button->setText(tr("Both Layers"));
	m_activeLayerButtonWidget->addWidget(button);

	button = new SketchToolButton("ActiveLayerB", parent, actions);
	button->setDefaultAction(m_activeLayerTopAct);
	button->setText(tr("Bottom Layer"));
	m_activeLayerButtonWidget->addWidget(button);

	button = new SketchToolButton("ActiveLayerT", parent, actions);
	button->setDefaultAction(m_activeLayerBothAct);
	button->setText(tr("Top Layer"));
	m_activeLayerButtonWidget->addWidget(button);

	return m_activeLayerButtonWidget;
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
			retval << SketchAreaWidget::separator(parent) 
				<< createActiveLayerButton(parent) 
				<< createAutorouteButton(parent) << createExportEtchableButton(parent) << createRoutingStatusLabel(parent);
			break;
		default:
			break;
	}

	return retval;
}

void MainWindow::updateZoomSlider(qreal zoom) {
	m_zoomSlider->setValue(zoom);
}

SketchAreaWidget *MainWindow::currentSketchArea() {
	return dynamic_cast<SketchAreaWidget*>(m_currentGraphicsView->parent());
}

void MainWindow::updateZoomOptionsNoMatterWhat(qreal zoom) {
	m_zoomSlider->setValue(zoom);
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
		m_currentGraphicsView->saveZoom(m_zoomSlider->value());
		disconnect(
			m_currentGraphicsView,
			SIGNAL(selectionChangedSignal()),
			this,
			SLOT(updateTransformationActions())
		);
	}
	m_currentGraphicsView = widget;
	if (widget == NULL) return;

	m_zoomSlider->setValue(m_currentGraphicsView->retrieveZoom());

	connect(
		m_currentGraphicsView,					// don't connect directly to the scene here, connect to the widget's signal
		SIGNAL(selectionChangedSignal()),
		this,
		SLOT(updateTransformationActions())
	);

	static QKeySequence zeroSequence(0);
	if (m_currentGraphicsView == m_schematicGraphicsView) {
		m_rotate90ccwAct->setShortcut(tr("Alt+Ctrl+R"));
		m_rotate90cwAct->setShortcut(tr("Ctrl+R"));
		m_rotate45ccwAct->setShortcut(zeroSequence);
		m_rotate45cwAct->setShortcut(zeroSequence);
	}
	else {
		m_rotate45ccwAct->setShortcut(tr("Alt+Ctrl+R"));
		m_rotate45cwAct->setShortcut(tr("Ctrl+R"));
		m_rotate90ccwAct->setShortcut(zeroSequence);
		m_rotate90cwAct->setShortcut(zeroSequence);
	}

	updateActiveLayerButtons();

	m_currentGraphicsView->setCurrent(true);

	// !!!!!! hack alert  !!!!!!!  
	// this item update loop seems to deal with a qt update bug:
	// if one view is visible and you change something in another view, 
	// the change might not appear when you switch views until you move the item in question
	foreach(QGraphicsItem * item, m_currentGraphicsView->items()) {
		item->update();
	}

	updateLayerMenu(true);
	QList<QAction *> actions;
	actions << m_showBreadboardAct << m_showSchematicAct << m_showPCBAct;
	setActionsIcons(index, actions);

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
	RoutingStatus routingStatus;
	m_currentGraphicsView->updateRatsnestStatus(NULL, NULL, routingStatus, false);
}

void MainWindow::setActionsIcons(int index, QList<QAction *> & actions) {
	for (int i = 0; i < actions.count(); i++) {
		actions[i]->setIcon(index == i ? m_dotIcon : m_emptyIcon);
	}
}

void MainWindow::closeEvent(QCloseEvent *event) {
	if (m_dontClose) {
		event->ignore();
		return;
	}

	if (m_programWindow) {
		m_programWindow->close();
		if (m_programWindow->isVisible()) {
			event->ignore();
			return;
		}
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
	if(modelPart && modelPart->hasViewIdentifier(ViewIdentifierClass::IconView)) {
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
	return m_undoStack->count() == 0 && !m_recovered;
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
	FolderUtils::createFolderAnCdIntoIt(destFolder, FolderUtils::getRandText());
	QString dirToRemove = destFolder.path();

	QString aux = QFileInfo(bundledFileName).fileName();
	QString destSketchPath = // remove the last "z" from the extension
			destFolder.path()+"/"+aux.left(aux.size()-1);
	DebugDialog::debug("saving entity temporarily to "+destSketchPath);

	for (int i = 0; i < m_linkedProgramFiles.count(); i++) {
		LinkedFile * linkedFile = m_linkedProgramFiles.at(i);
		QFileInfo fileInfo(linkedFile->filename);
		QFile file(linkedFile->filename);
		file.copy(destFolder.absoluteFilePath(fileInfo.fileName()));
	}

	QString prevFileName = filename;
	QApplication::processEvents();
	bundler->saveAsAux(destSketchPath);
	filename = prevFileName;

	foreach(ModelPart* mp, partsToSave) {
		saveBundledAux(mp, destFolder);
	}

	QApplication::processEvents();

	if(!FolderUtils::createZipAndSaveTo(destFolder, bundledFileName)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to export %1 as shareable").arg(bundledFileName)
		);
	}

	FolderUtils::rmdir(dirToRemove);
}

void MainWindow::loadBundledSketch(const QString &fileName) {
	loadBundledNonAtomicEntity(fileName, this, /*addToBin*/true);
}

void MainWindow::loadBundledNonAtomicEntity(const QString &fileName, Bundler* bundler, bool addToBin) {
	QDir destFolder = QDir::temp();

	FolderUtils::createFolderAnCdIntoIt(destFolder, FolderUtils::getRandText());
	QString unzipDirPath = destFolder.path();

	if(!FolderUtils::unzipTo(fileName, unzipDirPath)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to open shareable %1").arg(fileName)
		);

		// gotta return now, or loadBundledSketchAux will crash
		return;
	}

	QDir unzipDir(unzipDirPath);

	if (bundler->preloadBundledAux(unzipDir)) {
		QList<ModelPart*> mps = moveToPartsFolder(unzipDir,this,addToBin);
		// the bundled itself
		bundler->loadBundledAux(unzipDir,mps);
	}

	FolderUtils::rmdir(unzipDirPath);
}

bool MainWindow::preloadBundledAux(QDir &unzipDir) 
{
	QStringList namefilters;
	namefilters << "*"+FritzingSketchExtension;
	QFileInfoList entryInfoList = unzipDir.entryInfoList(namefilters);
	if (entryInfoList.count() == 0) return false;

	QFileInfo sketchInfo = entryInfoList[0];
	QString sketchName = defaultSaveFolder() + "/" + sketchInfo.fileName();

	QStringList linkedProgramFiles;
	if (!hasLinkedProgramFiles(sketchInfo.filePath(), linkedProgramFiles)) {
		QString fileExt;
		sketchName = 
			FolderUtils::getSaveFileName(
				this,
				tr("Please specify a location for the archived sketch"),
				sketchName,
				tr("Fritzing File (*%1)").arg(FritzingSketchExtension),
				&fileExt
			);
		if (sketchName.isEmpty()) return false;
	}
	else {
		// ask the user for a folder to store the sketch and program files
		QString path = QFileDialog::getExistingDirectory(
							this,
							tr("Please choose a folder to save the archived sketch and program files"),
							defaultSaveFolder(),
							QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (path.isEmpty()) return false;

		sketchName = path + "/" + sketchInfo.fileName();
	}

	if (!QFile::copy(sketchInfo.filePath(), sketchName)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to save sketch to %1").arg(sketchName)
		);
		return false;
	}

	QDir sketchDir = QFileInfo(sketchName).absoluteDir();

	foreach (QString fname, linkedProgramFiles) {
		QFileInfo finfo(fname);
		QFile::copy(unzipDir.absoluteFilePath(finfo.fileName()), sketchDir.absoluteFilePath(finfo.fileName()));
	}

	m_bundledSketchName = sketchName;
	return true;
}

bool MainWindow::loadBundledAux(QDir &unzipDir, QList<ModelPart*> mps) {

	Q_UNUSED(unzipDir);
        Q_UNUSED(mps);

	this->load(m_bundledSketchName, true, true, "");

	if (m_linkedProgramFiles.count() > 0) {
		// since the temporary dir containing the program files is about to be deleted
		// update the names to where the new copy is
		QFileInfo sketchInfo(m_bundledSketchName);
		QDir newdir = sketchInfo.absoluteDir();
		foreach (LinkedFile * linkedFile, m_linkedProgramFiles) {
			QFileInfo p(linkedFile->filename);
			linkedFile->filename = newdir.absoluteFilePath(p.fileName());
		}
	}

	m_alienPartsMsg = tr("Do you want to keep the parts that were loaded with this shareable sketch %1?");
	return true;
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
	QStringList fileNames = FolderUtils::getOpenFileNames(
		this,
		tr("Select one or more parts to import"),
		defaultSaveFolder(),
		tr("External Part (*%1)").arg(FritzingBundledPartExtension)
	);

	if (fileNames.count() == 0) return;

	foreach (QString fileName, fileNames) {
		loadBundledPart(fileName);
	}
}

ModelPart* MainWindow::loadBundledPart(const QString &fileName, bool addToBin) {
	QDir destFolder = QDir::temp();

	FolderUtils::createFolderAnCdIntoIt(destFolder, FolderUtils::getRandText());
	QString unzipDirPath = destFolder.path();

	if(!FolderUtils::unzipTo(fileName, unzipDirPath)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to open shareable part %1").arg(fileName)
		);
		return NULL;
	}

	QDir unzipDir(unzipDirPath);

	QList<ModelPart*> mps = moveToPartsFolder(unzipDir,this,addToBin);
	if (mps.count()!=1) {
		// if this fails, that means that the bundled was wrong
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to read shareable part %1").arg(fileName)
		);
		return NULL;
	}

	FolderUtils::rmdir(unzipDirPath);

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

	FolderUtils::createFolderAnCdIntoIt(destFolder, FolderUtils::getRandText());
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


	if(!FolderUtils::createZipAndSaveTo(destFolder, bundledFileName)) {
		QMessageBox::warning(
			this,
			tr("Fritzing"),
			tr("Unable to export %1 to shareable sketch").arg(bundledFileName)
		);
	}

	FolderUtils::rmdir(dirToRemove);
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

	if (mw == NULL) {
		throw "MainWindow::moveToPartsFolder mainwindow missing";
	}

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
	ModelPart *mp = m_refModel->loadPart(destFilePath, true, false);
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
			FolderUtils::createFolderAnCdIntoIt(m_tempDir, FolderUtils::getRandText());
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
		FolderUtils::rmdir(m_tempDir);
		m_tempDir = QDir::temp();
	}
	m_filesReplacedByAlienOnes.clear();
}

void MainWindow::routingStatusSlot(SketchWidget * sketchWidget, const RoutingStatus & routingStatus) {
	m_routingStatus = routingStatus;
	QString theText;
	if (routingStatus.m_netCount == 0) {
		theText = tr("No connections to route");
	} else if (routingStatus.m_netCount == routingStatus.m_netRoutedCount) {
		if (routingStatus.m_jumperWireCount == 0 && routingStatus.m_jumperItemCount == 0) {
			theText = tr("Routing completed");
		}
		else {
			theText = tr("Routing completed using %n jumper wire(s)", "", routingStatus.m_jumperWireCount) +
						tr(" and %n jumper part(s)", "", routingStatus.m_jumperItemCount);
		}
	} else {
		theText = tr("%1 of %2 nets routed - %n connector(s) still to be routed", "", routingStatus.m_connectorsLeftToRoute)
			.arg(routingStatus.m_netRoutedCount)
			.arg(routingStatus.m_netCount);
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
		AutoCloseMessageBox::showMessage(this, tr("No exactly matching part found; Fritzing chose the closest match."));
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

		if (key.compare("layers", Qt::CaseInsensitive) == 0) {
			Board * board = dynamic_cast<Board *>(itemBase);
			if (board == NULL) continue;

			QString value = currPropsMap.value(key, "");
			if (value.compare(Board::oneLayerTranslated) == 0) {
				currPropsMap.insert(key, "1");
				return false;
			}
			if (value.compare(Board::twoLayersTranslated) == 0) {
				currPropsMap.insert(key, "2");
				return false;
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
	swapSelectedAuxAux(itemBase, moduleID, itemBase->viewLayerSpec(), parentCommand);
	// need to defer execution so the content of the info view doesn't change during an event that started in the info view
	m_undoStack->waitPush(parentCommand, 10);
}


void MainWindow::subSwapSlot(SketchWidget * sketchWidget, ItemBase * itemBase, ViewLayer::ViewLayerSpec viewLayerSpec, QUndoCommand * parentCommand) {
	Q_UNUSED(sketchWidget);
	swapSelectedAuxAux(itemBase, itemBase->modelPart()->moduleID(), viewLayerSpec, parentCommand);
}

long MainWindow::swapSelectedAuxAux(ItemBase * itemBase, const QString & moduleID,  ViewLayer::ViewLayerSpec viewLayerSpec, QUndoCommand * parentCommand) 
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

	long newID1 = m_schematicGraphicsView->setUpSwap(itemBase, modelIndex, moduleID, viewLayerSpec, masterflags[0], parentCommand);
	long newID2 = m_pcbGraphicsView->setUpSwap(itemBase, modelIndex, moduleID, viewLayerSpec, masterflags[1], parentCommand);

	// master view must go last, since it creates the delete command
	long newID3 = m_breadboardGraphicsView->setUpSwap(itemBase, modelIndex, moduleID, viewLayerSpec, masterflags[2], parentCommand);

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


	QString moduleID = FolderUtils::getRandText();
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
	// %8 = layers

	QString layers = itemBase->modelPart()->prop("layers").toString();
	if (layers.isEmpty()) {
		layers = itemBase->modelPart()->properties().value("layers", "1");
	}
	if (layers.isEmpty()) {
		layers = "1";
	}

	QString fzp = fzpTemplate
		.arg(getenvUser())
		.arg(w * 25.4)
		.arg(h * 25.4)
		.arg(QFileInfo(path).baseName())
		.arg(QDate::currentDate().toString(Qt::ISODate))
		.arg(moduleID)
		.arg(QTime::currentTime().toString("HH:mm:ss"))
		.arg(layers);


	QString userPartsFolderPath = FolderUtils::getUserDataStorePath("parts")+"/user/";
	QFile file2(userPartsFolderPath + moduleID + FritzingPartExtension);
	file2.open(QIODevice::WriteOnly);
	QTextStream out2(&file2);
	out2.setCodec("UTF-8");
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
			c->restoreColor(false, -1, true);
		}
	}
}

void MainWindow::statusMessage(QString message, int timeout) {
	QStatusBar * sb = realStatusBar();
	if (sb != NULL) {
		sb->showMessage(message, timeout);
	}
}

void MainWindow::dropPaste(SketchWidget * sketchWidget) {
	Q_UNUSED(sketchWidget);
	pasteInPlace();
}

void MainWindow::updateLayerMenuSlot() {
	updateLayerMenu(true);
}

bool MainWindow::save() {
	bool result = FritzingWindow::save();
	if (result) {
		QSettings settings;
		settings.setValue("lastOpenSketch", m_fileName);
	}
	return result;
}

bool MainWindow::saveAs() {
	bool result = FritzingWindow::saveAs();
	if (result) {
		QSettings settings;
		settings.setValue("lastOpenSketch", m_fileName);
	}
	return result;
}

void MainWindow::changeBoardLayers(int layers, bool doEmit) {
	Q_UNUSED(doEmit);
	Q_UNUSED(layers);
	updateActiveLayerButtons();
	QTimer::singleShot(10, m_currentGraphicsView, SLOT(updateConnectors()));
	m_currentGraphicsView->updateConnectors();
}

void MainWindow::updateActiveLayerButtons() {
	bool enabled = false;
	int index = 0;
	if (m_currentGraphicsView->boardLayers() == 2) {
		bool copper0Visible = m_currentGraphicsView->layerIsActive(ViewLayer::Copper0);
		bool copper1Visible = m_currentGraphicsView->layerIsActive(ViewLayer::Copper1);
		if (copper0Visible && copper1Visible) {
			index = 0;
		}
		else if (copper1Visible) {
			index = 2;
		}
		else if (copper0Visible) {
			index = 1;
		}
		enabled = true;
	}

	m_activeLayerButtonWidget->setCurrentIndex(index);
	m_activeLayerButtonWidget->setVisible(enabled);

	m_activeLayerBothAct->setEnabled(enabled);
	m_activeLayerBottomAct->setEnabled(enabled);
	m_activeLayerTopAct->setEnabled(enabled);

	QList<QAction *> actions;
	actions << m_activeLayerBothAct << m_activeLayerBottomAct << m_activeLayerTopAct;
	setActionsIcons(index, actions);
}

/**
 * A slot for saving a copy of the current sketch to a temp location.
 * This should be called every X minutes as well as just before certain
 * events, such as saves, part imports, file export/printing. This relies
 * on the m_autosaveNeeded variable and the undoStack being dirty for
 * an autosave to be attempted.
 */
void  MainWindow::backupSketch() {
    if (m_autosaveNeeded && !m_undoStack->isClean()) {
        m_autosaveNeeded = false;			// clear this now in case the save takes a really long time

        DebugDialog::debug(QString("%1 autosaved as %2").arg(m_fileName).arg(m_backupFileNameAndPath));
        statusBar()->showMessage(tr("Backing up '%1'").arg(m_fileName), 2000);
		QApplication::processEvents();
		m_backingUp = true;
		saveAsAuxAux(m_backupFileNameAndPath);
		m_backingUp = false;
    }
}

/**
 * This function is used to trigger an autosave at the next autosave
 * timer event. It is connected to the QUndoStack::indexChanged(int)
 * signal so that any change to the undo stack triggers autosaves.
 * This function can be called independent of this signal and
 * still work properly.
 */
void MainWindow::autosaveNeeded(int index) {
    Q_UNUSED(index);
    //DebugDialog::debug(QString("Triggering autosave"));
    m_autosaveNeeded = true;
}

/**
 * delete the backup file when the undostack is clean.
 */
void MainWindow::undoStackCleanChanged(bool isClean) {
    DebugDialog::debug(QString("Clean status changed to %1").arg(isClean));
    if (isClean) {
        QFile::remove(m_backupFileNameAndPath);
    }
}

void MainWindow::setAutosavePeriod(int minutes) {
	setAutosave(minutes, AutosaveEnabled);
}

void MainWindow::setAutosaveEnabled(bool enabled) {
	setAutosave(AutosaveTimeoutMinutes, enabled);
}

void MainWindow::setAutosave(int minutes, bool enabled) {
	AutosaveTimeoutMinutes = minutes;
	AutosaveEnabled = enabled;
	foreach (QWidget * widget, QApplication::topLevelWidgets()) {
		MainWindow * mainWindow = qobject_cast<MainWindow *>(widget);
		if (mainWindow == NULL) continue;

		mainWindow->m_autosaveTimer.stop();
		if (AutosaveEnabled) {
			// is there a way to get the current timer offset so that all the timers aren't running in sync?
			// or just add some random time...
			mainWindow->m_autosaveTimer.start(AutosaveTimeoutMinutes * 60 * 1000);
		}
	}
}

void MainWindow::setRecovered(bool recovered) {
	m_recovered = recovered;
}

bool MainWindow::hasLinkedProgramFiles(const QString & filename, QStringList & linkedProgramFiles) 
{
	QFile file(filename);
	file.open(QFile::ReadOnly);
	QXmlStreamReader xml(&file);
    xml.setNamespaceProcessing(false);

	bool done = false;
	while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
			if (xml.name().toString().compare("program") == 0) {
				linkedProgramFiles.append(xml.readElementText());
				break;
			}
			if (xml.name().toString().compare("views") == 0) {
				done = true;
				break;
			}
			if (xml.name().toString().compare("instances") == 0) {
				done = true;
				break;
			}
		default:
			break;
		}

		if (done) break;
	}

	return linkedProgramFiles.count() > 0;
}

QString MainWindow::getExtensionString() {
	return tr("Fritzing (*%1)").arg(fileExtension());
}

QStringList MainWindow::getExtensions() {
	QStringList extensions;
	extensions.append(fileExtension());
	return extensions;
}

void MainWindow::firstTimeHelpHidden() {
	m_showInViewHelpAct->setChecked(false);
}

void MainWindow::routingStatusLabelMousePress(QMouseEvent* event) {
	routingStatusLabelMouse(event, true);
}

void MainWindow::routingStatusLabelMouseRelease(QMouseEvent* event) {
	routingStatusLabelMouse(event, false);
}

void MainWindow::routingStatusLabelMouse(QMouseEvent*, bool show) {
	if (show) DebugDialog::debug("-------");
	foreach (ConnectorItem * iConnectorItem, m_routingStatus.m_unroutedConnectors.uniqueKeys()) {
		iConnectorItem->showEqualPotential(show);
		if (show)
			DebugDialog::debug(QString("unrouted %1 %2 %3 %4")
				.arg(iConnectorItem->attachedToInstanceTitle())
				.arg(iConnectorItem->attachedToID())
				.arg(iConnectorItem->attachedTo()->title())
				.arg(iConnectorItem->connectorSharedName()));
		foreach(ConnectorItem * jConnectorItem, m_routingStatus.m_unroutedConnectors.values(iConnectorItem)) {
			jConnectorItem->showEqualPotential(show);
			if (show) 
				DebugDialog::debug(QString("     %1 %2 %3 %4")
					.arg(jConnectorItem->attachedToInstanceTitle())
					.arg(jConnectorItem->attachedToID())
					.arg(jConnectorItem->attachedTo()->title())
					.arg(jConnectorItem->connectorSharedName()));
		}
	}
}


