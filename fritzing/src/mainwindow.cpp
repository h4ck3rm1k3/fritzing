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
#include <QDesktopServices>

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

MainWindow::MainWindow(PaletteModel * paletteModel, ReferenceModel *refModel) :
	FritzingWindow(untitledFileName(), untitledFileCount(), fileExtension())
{
	QFile styleSheet(":/resources/styles/fritzing.qss");

	if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/fritzing.qss");
	} else {
		setStyleSheet(styleSheet.readAll()+___MacStyle___);
	}

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

	m_tabWidget = new FTabWidget(this);
	m_tabWidget->setObjectName("sketch_tabs");
	setCentralWidget(m_tabWidget);

	// all this belongs in viewLayer.xml
	m_breadboardGraphicsView = new SketchWidget(ItemBase::BreadboardView, this);
	m_breadboardWidget = new SketchAreaWidget(m_breadboardGraphicsView,this);
	m_breadboardGraphicsView->setPaletteModel(m_paletteModel);
	m_breadboardGraphicsView->setSketchModel(m_sketchModel);
	m_breadboardGraphicsView->setRefModel(m_refModel);
	m_breadboardGraphicsView->setUndoStack(m_undoStack);
	m_breadboardGraphicsView->setChainDrag(true);			// enable bend points
	m_breadboardGraphicsView->addBreadboardViewLayers();
	m_tabWidget->addTab(m_breadboardWidget, tr("breadboard"));


	m_schematicGraphicsView = new SketchWidget(ItemBase::SchematicView, this);
	m_schematicWidget = new SketchAreaWidget(m_schematicGraphicsView, this);
	m_schematicGraphicsView->setPaletteModel(m_paletteModel);
	m_schematicGraphicsView->setRefModel(m_refModel);
	m_schematicGraphicsView->setSketchModel(m_sketchModel);
	m_schematicGraphicsView->setUndoStack(m_undoStack);
	m_schematicGraphicsView->setChainDrag(true);			// enable bend points
	m_schematicGraphicsView->addSchematicViewLayers();
	m_tabWidget->addTab(m_schematicWidget, tr("schematic"));

	m_pcbGraphicsView = new SketchWidget(ItemBase::PCBView, this);
	m_pcbWidget = new SketchAreaWidget(m_pcbGraphicsView, this);
	m_pcbGraphicsView->setPaletteModel(m_paletteModel);
	m_pcbGraphicsView->setRefModel(m_refModel);
	m_pcbGraphicsView->setSketchModel(m_sketchModel);
	m_pcbGraphicsView->setUndoStack(m_undoStack);
	m_pcbGraphicsView->setChainDrag(true);				// enable bend points
	m_pcbGraphicsView->addPcbViewLayers();
	m_tabWidget->addTab(m_pcbWidget, tr("pcb"));

    m_undoView = new QUndoView();
    m_undoGroup = new QUndoGroup(this);
    m_undoView->setGroup(m_undoGroup);
    m_undoGroup->setActiveStack(m_breadboardGraphicsView->undoStack());

    createActions();
    createZoomOptions();
    createSketchButtons();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();

	m_itemMenu = new QMenu(QObject::tr("Part"), this);
	m_itemMenu->addAction(m_openInPartsEditorAct);
	//m_itemMenu->addAction(m_infoViewOnHoverAction);
	//m_itemMenu->addAction(m_swapPartAction);

    connect(
    	m_itemMenu,
    	SIGNAL(aboutToShow()),
    	this,
    	SLOT(updateItemMenu())
    );

    m_breadboardGraphicsView->setItemMenu(m_itemMenu);
    m_pcbGraphicsView->setItemMenu(m_itemMenu);
    m_schematicGraphicsView->setItemMenu(m_itemMenu);

    m_breadboardGraphicsView->setInfoView(m_infoView);
    m_pcbGraphicsView->setInfoView(m_infoView);
    m_schematicGraphicsView->setInfoView(m_infoView);

    m_breadboardGraphicsView->setBackground(QColor(204,204,204));
    m_schematicGraphicsView->setBackground(QColor(255,255,255));
    m_pcbGraphicsView->setBackground(QColor(137,144,153));

	// make sure to set the connections after the views have been created
	connect(m_tabWidget, SIGNAL(currentChanged ( int )),
			this, SLOT(tabWidget_currentChanged( int )));

	connectPairs();

	// do this the first time, since the current_changed signal wasn't sent
	tabWidget_currentChanged(0);

	this->installEventFilter(this);

	m_comboboxChanged = false;

	QSettings settings("Fritzing","Fritzing");
	if(!settings.value("main/state").isNull()) {
		restoreState(settings.value("main/state").toByteArray());
		restoreGeometry(settings.value("main/geometry").toByteArray());
	}

	setMinimumSize(0,0);
	m_tabWidget->setMinimumWidth(500);
	m_tabWidget->setMinimumWidth(0);
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

	qreal divisor = 1.0;
	if (stringWidth.endsWith("cm", Qt::CaseInsensitive)) {
		divisor = 2.54;
		stringWidth.chop(2);
	}
	else if (stringWidth.endsWith("mm", Qt::CaseInsensitive)) {
		divisor = 25.4;
		stringWidth.chop(2);
	}
	else if (stringWidth.endsWith("in", Qt::CaseInsensitive)) {
		divisor = 1.0;
		stringWidth.chop(2);
	}
	else {
		// units not understood
		return 0;
	}

	bool ok;
	qreal result = stringWidth.toDouble(&ok);
	if (!ok) return 0;

	return result / divisor;
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
	succeeded = succeeded && connect(signaller, SIGNAL(tooltipAppliedToItem(long, const QString&)),
										 slotter, SLOT(sketchWidget_tooltipAppliedToItem(long, const QString&)));
	succeeded = succeeded && connect(signaller, SIGNAL(wireDisconnectedSignal(long, QString)),
									 slotter, SLOT(sketchWidget_wireDisconnected(long,  QString)));
	succeeded = succeeded && connect(signaller, SIGNAL(wireConnectedSignal(long,  QString, long,  QString)),
									 slotter, SLOT(sketchWidget_wireConnected(long, QString, long, QString)));
	succeeded = succeeded && connect(signaller, SIGNAL(changeConnectionSignal(long,  QString, long,  QString, bool, bool, bool)),
									 slotter, SLOT(sketchWidget_changeConnection(long, QString, long, QString, bool, bool, bool)));
	succeeded = succeeded && connect(signaller, SIGNAL(initializeBusConnectorItemSignal(long, const QString &)),
									 slotter, SLOT(sketchWidget_initializeBusConnectorItem(long, const QString &)));
	succeeded = succeeded && connect(signaller, SIGNAL(mergeBusesSignal(long, const QString &, QPointF, long, const QString &, QPointF, bool)),
									 slotter, SLOT(sketchWidget_mergeBuses(long, const QString &, QPointF, long, const QString &, QPointF, bool)));
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


	if (!succeeded) {
		DebugDialog::debug("connectPair failed");
	}

}

void MainWindow::setCurrentFile(const QString &fileName) {
	m_fileName = fileName;
	setTitle();

    QSettings settings("Fritzing","Fritzing");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}


void MainWindow::createZoomOptions() {
	m_zoomOptsComboBox = new ZoomComboBox(this);

	connect(m_zoomOptsComboBox, SIGNAL(zoomChanged(qreal)), this, SLOT(updateViewZoom(qreal)));

    connect(m_breadboardGraphicsView, SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));
    connect(m_schematicGraphicsView, SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));
    connect(m_pcbGraphicsView, SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));

    connect(m_breadboardGraphicsView, SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));
	connect(m_schematicGraphicsView, SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));
	connect(m_pcbGraphicsView, SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));

	connect(m_breadboardGraphicsView, SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));
	connect(m_schematicGraphicsView, SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));
	connect(m_pcbGraphicsView, SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));

	connect(m_breadboardGraphicsView, SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));
	connect(m_schematicGraphicsView, SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));
	connect(m_pcbGraphicsView, SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));
}

void MainWindow::createToolBars() {
	/* TODO: Mariano this is too hacky and requires some styling
	 * around here and some else in the qss file
	 */
	QToolBar *tb = new QToolBar(this);
	tb->setObjectName("fake_tabbar");
	tb->setFloatable(false);
	tb->setMovable(false);
	int height = m_tabWidget->tabBar()->height();
	tb->layout()->setMargin(0);
	tb->setFixedHeight(height+10);
	tb->setMinimumWidth(400); // connect to tabwidget resize event
	tb->toggleViewAction()->setVisible(false);
	m_tabWidget->tabBar()->setParent(tb);
	addToolBar(tb);

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
	m_exportToPdfButton = new QToolButton(this);
	m_exportToPdfButton->setDefaultAction(m_exportPdfAct);
	m_exportToPdfButton->setIcon(QIcon(":/resources/images/toolbar_icons/toolbarExport_pdf_icon.png"));

	/*QMenu *exportMenu = new QMenu(m_exportToPdfButton);
	exportMenu->addAction(QIcon(":/resources/images/toolbar_icons/toolbarExport_ps_icon.png"),"");
	m_exportToPdf

	Button->setMenu(exportMenu);*/

	m_autorouteButton = new QPushButton(this);
	m_autorouteButton->setIcon(QIcon(":/resources/images/toolbar_icons/toolbarAutorouteEnabled_icon.png"));
	connect(m_autorouteButton, SIGNAL(clicked()), this, SLOT(autoroute()));

	m_exportDiyButton = new QPushButton(this);
	m_exportDiyButton->setIcon(QIcon(":/resources/images/toolbar_icons/toolbarExport_diy_icon.png"));
	connect(m_exportDiyButton, SIGNAL(clicked()), this, SLOT(exportDiy()));
}

QList<QWidget*> MainWindow::getButtonsForView(ItemBase::ViewIdentifier viewId) {
	QList<QWidget*> retval;
	retval << m_exportToPdfButton;
	if(viewId == ItemBase::PCBView) {
		retval << m_autorouteButton << m_exportDiyButton;
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

void MainWindow::updateZoomOptionsNoMatterWhat(qreal zoom) {
	m_zoomOptsComboBox->setEditText(tr("%1%").arg(zoom));
}

void MainWindow::updateViewZoom(qreal newZoom) {
	m_comboboxChanged = true;
	m_currentWidget->absoluteZoom(newZoom);
}


void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::tabWidget_currentChanged(int index) {
	SketchAreaWidget * widgetParent = dynamic_cast<SketchAreaWidget *>(m_tabWidget->currentWidget());
	if (widgetParent == NULL) return;
	widgetParent->setContent(getButtonsForView(widgetParent->viewIdentifier()),m_zoomOptsComboBox);

	SketchWidget *widget = widgetParent->graphicsView();

	m_currentWidget = widget;
	if (widget == NULL) return;

	m_miniViewContainer->setView(widget);

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

	if(!beforeClosing() || !whatToDoWithFilesAddedFromBundled() ||!m_paletteWidget->beforeClosing()) {
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

bool MainWindow::whatToDoWithFilesAddedFromBundled() {
	if (m_filesAddedFromBundled.size() > 0) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, tr("Save %1").arg(QFileInfo(m_fileName).baseName()),
									 tr("Do you want to keep the parts that were loaded with this bundled sketch %1?")
									 .arg(QFileInfo(m_fileName).baseName()),
									 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (reply == QMessageBox::Yes) {
			return true;
		} else if (reply == QMessageBox::No) {
			foreach(QString pathToRemove, m_filesAddedFromBundled) {
				QFile::remove(pathToRemove);
			}
			m_filesAddedFromBundled.clear();
			emit partsFromBundledDiscarded();
			return true;
		}
		else {
			return false;
		}
	} else {
		return true;
	}
}


void MainWindow::setZoomComboBoxValue(qreal value) {
	m_zoomOptsComboBox->setEditText(tr("%1%").arg(value,0,'f',2));
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

void MainWindow::createDockWindows()
{
	m_infoView = new HtmlInfoView(m_refModel);

	m_paletteWidget = new PartsBinPaletteWidget(m_infoView, this);
	connect(m_paletteWidget, SIGNAL(saved(bool)), this, SLOT(binSaved(bool)));
	if (m_paletteModel->loadedFromFile()) {
		m_paletteWidget->loadFromModel(m_paletteModel);
	}
	else
	{
		m_paletteWidget->setPaletteModel(m_paletteModel);
	}
	dockIt(m_paletteWidget, PartsBinMinHeight, PartsBinDefaultHeight);

    makeDock(tr("Part Inspector"), m_infoView, InfoViewMinHeight, InfoViewDefaultHeight);

    m_miniViewContainer = new MiniViewContainer(this);
    makeDock(tr("Navigator"), m_miniViewContainer, NavigatorMinHeight, NavigatorDefaultHeight);

    makeDock(tr("Undo History"), m_undoView, UndoHistoryMinHeight, UndoHistoryDefaultHeight)->hide();
    m_undoView->setMinimumSize(DockMinWidth, UndoHistoryMinHeight);

    m_consoleView = new Console();
    QDockWidget * dock = makeDock(tr("Console"), m_consoleView, DockMinHeight, DockDefaultHeight, Qt::BottomDockWidgetArea);
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
		updateEditMenu();
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
	DebugDialog::debug(tr("default save location: %1").arg(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
	return QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
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
}

void MainWindow::swapSelected() {
	PaletteItem *selInParts = m_paletteWidget->selected();
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
		QList<StringTriple> views = mp->getAvailableViewFiles();
		foreach(StringTriple view, views) {
			if(view.second != "core") {
				QFile file(view.concat());
				file.copy(destFolder.path()+"/"+ZIP_SVG+view.third.replace("/","."));
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

void MainWindow::loadBundledSketch() {
	QString path;
	// if it's the first time load is called use Documents folder
	if(m_firstOpen){
		path = defaultSaveFolder();
		m_firstOpen = false;
	}
	else {
		path = "";
	}
	QString fileName = QFileDialog::getOpenFileName( this, "Select a bundled sketch to Open", path, tr("Fritzing (*%1)").arg(FritzingExtension+"z") );
	if (fileName.isNull()) return;

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

	QString destFilePath = getApplicationSubFolderPath("parts")+"/svg/"+destFolder+"/"+viewFolder+"/"+fileName;
	if(svgfile.copy(destFilePath)) {
		// TODO Mariano: make a backup if it already exists
		m_filesAddedFromBundled << destFilePath;
	}
}

void MainWindow::copyToPartsFolder(const QFileInfo& file, const QString &destFolder) {
	QFile partfile(file.filePath());
	// let's make sure that we remove just the suffix
	QString destFilePath = getApplicationSubFolderPath("parts")+"/"+destFolder+"/"+file.fileName().remove(QRegExp("^"+ZIP_PART));

	if(partfile.copy(destFilePath)) {
		// TODO Mariano: make a backup if it already exists
		m_filesAddedFromBundled << destFilePath;
	}
	ModelPart *mp = m_refModel->loadPart(destFilePath, true);
	m_paletteWidget->addPart(mp, true);
}

void MainWindow::binSaved(bool hasPartsFromBundled) {
	if(hasPartsFromBundled) {
		// the bin will need those parts, so just keep them
		m_filesAddedFromBundled.clear();
	}
}

#undef ZIP_PART
#undef ZIP_SVG
