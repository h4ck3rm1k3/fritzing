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

#include "mainpartseditorwindow.h"
#include "pcbxml.h"
#include "../debugdialog.h"
#include "../fdockwidget.h"
#include "../waitpushundostack.h"

#include "editabletextwidget.h"
#include "partseditorspecificationsview.h"
#include "partsymbolswidget.h"


#include <QtGui>
#include <QCryptographicHash>

// TODO: delete svgview stuff. no longer needed as we are using SketchWidget

const QString MainPartsEditorWindow::templatePath = "/docs/templates/";

MainPartsEditorWindow::MainPartsEditorWindow(long id, QWidget * parent, Qt::WFlags f, ModelPart *modelPart, bool fromTemplate)
	: QMainWindow(parent, f)
{
	Q_UNUSED(fromTemplate);
	m_id = id;

	m_comboboxChanged = false;
	setWindowTitle(tr("Parts Editor"));
	setAttribute(Qt::WA_DeleteOnClose, true);

	m_paletteModel = new PaletteModel(false, false);

	if(modelPart == NULL){
		if (fromTemplate && !createTemplate()){
			QMessageBox::critical(this, tr("Parts Editor"),
	                           tr("Error! Cannot create part file.\n"),
	                           QMessageBox::Close);
	        // this seems hacky but maybe it's ok
	        QTimer::singleShot(60, this, SLOT(close()));
			return;
		}
	}
	else {
		m_partPath =  modelPart->modelPartStuff()->path();
	}

	//if(!fromTemplate) {
		m_sketchModel = new SketchModel(true);
	//} else {
	//	m_partInfoWidget = new PartInfoWidget(this);
	//	m_connWidget = new ConnectorsWidget();
	//	connect(m_paletteModel, SIGNAL(newPartLoaded(ModelPart*)), m_connWidget, SLOT(updateInfo(ModelPart*)));
	//	connect(m_paletteModel, SIGNAL(newPartLoaded(ModelPart*)), m_partInfoWidget, SLOT(updateInfo(ModelPart*)));
	//	modelPart = m_paletteModel->loadPart(m_partPath);
	//	m_sketchModel = new SketchModel(modelPart);
	//}

	m_undoStack = new WaitPushUndoStack(this);

	m_pcbView = new PartsEditorSketchWidget(ItemBase::PCBView, this);
	m_pcbView->setSketchModel(m_sketchModel);
	m_pcbView->setUndoStack(m_undoStack);
	ViewLayer *pcbViewLayer = new ViewLayer(ViewLayer::Copper0, true, 2.5);
	m_pcbView->addViewLayer(pcbViewLayer);

	m_schemSvgFile = new StringPair;
	m_schemView = new PartsEditorSketchWidget(ItemBase::SchematicView, this);
	m_schemView->setSketchModel(m_sketchModel);
	m_schemView->setUndoStack(m_undoStack);
	ViewLayer *schemViewLayer = new ViewLayer(ViewLayer::Schematic, true, 2.5);
	m_schemView->addViewLayer(schemViewLayer);

	m_breadSvgFile = new StringPair;
	m_breadView = new PartsEditorSketchWidget(ItemBase::BreadboardView, this);
	m_breadView->setSketchModel(m_sketchModel);
	m_breadView->setUndoStack(m_undoStack);
	ViewLayer *breadViewLayer = new ViewLayer(ViewLayer::Breadboard, true, 2.5);
	m_breadView->addViewLayer(breadViewLayer);

	m_iconSvgFile = new StringPair;
	m_iconView = new PartsEditorSketchWidget(ItemBase::IconView, this);
	m_iconView->setSketchModel(m_sketchModel);
	m_iconView->setUndoStack(m_undoStack);
	ViewLayer *iconViewLayer = new ViewLayer(ViewLayer::Icon, true, 2.5);
	m_iconView->addViewLayer(iconViewLayer);


	//m_mainFrame = new QFrame(this);
	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	m_tabWidget->addTab(m_breadView, tr("breadboard"));
	m_tabWidget->addTab(m_schemView, tr("schematic"));
	m_tabWidget->addTab(m_pcbView, tr("pcb"));
	m_tabWidget->addTab(m_iconView,tr("icon"));

	createZoomOptions();
	createMenus();
	createDockWindows();

	m_tabWidget->connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabWidget_currentChanged(int)));
	tabWidget_currentChanged(0);

    statusBar()->showMessage(QString(" "), 2000);

    if(fromTemplate) {
		loadSketchWidgetFromModel(m_breadView, m_breadSvgFile, m_sketchModel->root(), ItemBase::BreadboardView);
		loadSketchWidgetFromModel(m_schemView, m_schemSvgFile, m_sketchModel->root(), ItemBase::SchematicView);
		loadSketchWidgetFromModel(m_iconView, m_iconSvgFile, m_sketchModel->root(), ItemBase::IconView);

		m_pcbView->setPaletteModel(m_paletteModel);
		ViewGeometry viewGeometry;
		m_pcbView->loadFromModel(modelPart, viewGeometry);

    }
}

void MainPartsEditorWindow::loadSketchWidgetFromModel(PartsEditorSketchWidget *widget, StringPair *svgFilePath, ModelPart * modelPart, ItemBase::ViewIdentifier viewId) {
	ViewGeometry viewGeometry;
	widget->setPaletteModel(m_paletteModel);
	PartsEditorPaletteItem* item = (PartsEditorPaletteItem*)widget->loadFromModel(modelPart, viewGeometry);
	StringPair *sp = item->svgFilePath();
	QString svgFilePathOrig = sp->first+"/"+sp->second;
	delete sp;
	copyToTempAndRename(svgFilePathOrig, svgFilePath, viewId);
	item->setSvgFilePath(svgFilePath);
}

void MainPartsEditorWindow::tabWidget_currentChanged(int index) {
	m_settingsStack->setCurrentIndex(index);
	updateZoomText(index);
}

// creates a temp directory and copies all template .fz and .svg files
// returns false if directory can't be created
bool MainPartsEditorWindow::createTemplate(){
	QDir srcDir = QDir(":/resources/part-template");
	DebugDialog::debug("template source: " + srcDir.path());

	QDir randDir = createTempFolderIfNecessary();

	replicateDir(srcDir,randDir);

	QFile tempFile(QCoreApplication::applicationDirPath() + templatePath);
	tempFile.copy(randDir.path() + "/core/template" + FritzingPartExtension);

	m_partPath = randDir.path() + "/core/template" + FritzingPartExtension;
	DebugDialog::debug("created temp part: " + m_partPath);

	return true;
}

QDir MainPartsEditorWindow::createTempFolderIfNecessary() {
	if(m_tempDir.path() == ".") {
		// generate unique temp dir
		QString randext = getRandText();
		QDir randDir = QDir(QDir::tempPath());
		if(!randDir.mkdir(randext)) return randDir;
		if(!randDir.cd(randext)) return randDir;

		m_tempDir = randDir;
	}
	return m_tempDir;
}

void MainPartsEditorWindow::replicateDir(QDir srcDir, QDir targDir) {
	// copy all files from template source to new random temp dir
	QStringList files = srcDir.entryList();
	for(int i=0; i < files.size(); i++) {
		QFile tempFile(srcDir.path() + "/" +files.at(i));
		DebugDialog::debug("copying " + tempFile.fileName());
		QFileInfo fi(files.at(i));
		QString newFilePath = targDir.path() + "/" + fi.fileName();
		if(QFileInfo(tempFile.fileName()).isDir()) {
			QDir newTargDir = QDir(newFilePath);
			newTargDir.mkpath(newTargDir.absolutePath());
			newTargDir.cd(files.at(i));
			replicateDir(QDir(tempFile.fileName()),newTargDir);
		} else {
			tempFile.copy(newFilePath);
		}
	}
}

void MainPartsEditorWindow::createDockWindows()
{
	// layer info and tools pane contains a stack with a QFormLayout for each layer
    QDockWidget *dock = new QDockWidget(tr("Layer Settings"), this);
    m_settingsStack = new QStackedWidget(this);
	dock->setWidget(m_settingsStack);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    addDockWidget(Qt::RightDockWidgetArea, dock);

	// breadboard layer
	QWidget *breadSettings = new QWidget(dock);
    QFormLayout *breadForm = new QFormLayout;
    QPushButton *breadLoadSvgButton = new QPushButton("&Select Image", breadSettings);
    breadLoadSvgButton->connect(breadLoadSvgButton, SIGNAL(clicked()), this, SLOT(loadBreadFile()));
    breadForm->addRow(tr("&SVG Image:") + m_breadSvgFile->second, breadLoadSvgButton);
    breadSettings->setLayout(breadForm);
    m_settingsStack->addWidget(breadSettings);;

	// schem layer
	QWidget *schemSettings = new QWidget(dock);
    QFormLayout *schemForm = new QFormLayout;
    QPushButton *schemLoadSvgButton = new QPushButton("&Select Image", schemSettings);
    schemLoadSvgButton->connect(schemLoadSvgButton, SIGNAL(clicked()), this, SLOT(loadSchemFile()));
    schemForm->addRow(tr("&SVG Image:") + m_schemSvgFile->second, schemLoadSvgButton);
    schemSettings->setLayout(schemForm);
    m_settingsStack->addWidget(schemSettings);

	// pcb layer
	QWidget *pcbSettings = new QWidget(dock);
    QFormLayout *pcbForm = new QFormLayout;
    QPushButton *pcbLoadFootprintButton = new QPushButton("&Choose File", pcbSettings);
    pcbLoadFootprintButton->connect(pcbLoadFootprintButton, SIGNAL(clicked()), this, SLOT(loadPcbFootprint()));
    pcbForm->addRow(tr("&Import Footprint:") + m_pcbSvgFile, pcbLoadFootprintButton);
    pcbSettings->setLayout(pcbForm);
    m_settingsStack->addWidget(pcbSettings);

	// icon layer
	QWidget *iconSettings = new QWidget(dock);
    QFormLayout *iconForm = new QFormLayout;
    QPushButton *iconLoadSvgButton = new QPushButton("&Select Image", iconSettings);
    iconLoadSvgButton->connect(iconLoadSvgButton, SIGNAL(clicked()), this, SLOT(loadIconFile()));
    iconForm->addRow(tr("&SVG Image:") + m_iconSvgFile->second, iconLoadSvgButton);
    iconSettings->setLayout(iconForm);
    m_settingsStack->addWidget(iconSettings);

    // General part info
	m_partInfoWidget = new PartInfoWidget(this);
	makeDock(tr("General Info"), m_partInfoWidget, Qt::RightDockWidgetArea);

    // connector stuff
    m_connWidget = new ConnectorsWidget();
    makeDock(tr("Connectors"), m_connWidget, Qt::RightDockWidgetArea);
    connectViewToConnectorsDock(m_breadView);
	connectViewToConnectorsDock(m_schemView);
	connectViewToConnectorsDock(m_pcbView);

	//makeDock(tr("NEW WIDGETS TEST"), new EditableTextWidget(this), Qt::TopDockWidgetArea);
	//makeDock(tr("NEW WIDGETS TEST"), new PartsEditorViewImageWidget(ItemBase::BreadboardView,0,this), Qt::TopDockWidgetArea);
	//makeDock(tr("NEW WIDGETS TEST"), new PartSymbolsWidget(this), Qt::TopDockWidgetArea);
}

void MainPartsEditorWindow::loadPcbFootprint(){
	QString pcbFootprintFile;
	//this->loadFileIntoView(m_pcbSvgFile, m_pcbView, ItemBase::PCBView, "copper0");
	pcbFootprintFile = QFileDialog::getOpenFileName(this,
       tr("Open Image"), QApplication::applicationFilePath(), tr("SVG Files (*.fzfp)"));

    QFile file(pcbFootprintFile);
    QDomDocument doc("footprint");
    if (!file.open(QIODevice::ReadOnly)) {
    	 DebugDialog::debug("cannot open fzfp file");
         return;
    }
    if (!doc.setContent(&file)) {
    	DebugDialog::debug("cannot parse fzfp xml");
        file.close();
        return;
    }
    file.close();
    PcbXML *footprint = new PcbXML(doc.documentElement());
    DebugDialog::debug(footprint->getSvgFile());
}

void MainPartsEditorWindow::loadSchemFile(){
	this->loadFileIntoView(m_schemSvgFile, m_schemView, ItemBase::SchematicView, "schematic");
}

void MainPartsEditorWindow::loadIconFile(){
	this->loadFileIntoView(m_iconSvgFile, m_iconView, ItemBase::IconView, "icon");
}

void MainPartsEditorWindow::loadBreadFile(){
	this->loadFileIntoView(m_breadSvgFile, m_breadView, ItemBase::BreadboardView, "breadboard");
}

void MainPartsEditorWindow::loadFileIntoView(StringPair *svgFilePath, PartsEditorSketchWidget * view, ItemBase::ViewIdentifier viewIdentifier, QString layer) {
	QString origPath = QFileDialog::getOpenFileName(this,
       tr("Open Image"),
       (svgFilePath->first.isEmpty() || svgFilePath->second.isEmpty()) ? QDir::currentPath()+"/parts/svg/" : svgFilePath->first+"/"+svgFilePath->second,
       tr("SVG Files (*.svg)"));

	if(origPath.isEmpty()) {
		return; // Cancel pressed
	} else {
		ModelPart * mp = static_cast<ModelPart *>(m_sketchModel->root());

		copyToTempAndRename(origPath, svgFilePath, viewIdentifier);
		view->loadSvgFile(svgFilePath, mp, viewIdentifier, layer);
	}
}

void MainPartsEditorWindow::copyToTempAndRename(QString filePathOrig, StringPair* filePathDest, ItemBase::ViewIdentifier viewId) {
	DebugDialog::debug(QString("---- copying from %1").arg(filePathOrig));
	QDir randDir = createTempFolderIfNecessary();
	QString viewFolder = ItemBase::viewIdentifierNaturalName(viewId);

	if(!randDir.mkdir(viewFolder)) return;
	if(!randDir.cd(viewFolder)) return;

	QString destFilePath = getRandText()+".svg";
	DebugDialog::debug(QString("dest file: %1").arg(randDir.path()+"/"+destFilePath));
	QFile tempFile(filePathOrig);
	tempFile.copy(randDir.path()+"/"+destFilePath);

	if(!randDir.cd("..")) return; // out of view folder
	filePathDest->first = randDir.path();

	DebugDialog::debug(QString("path in fz file: %1").arg(viewFolder+"/"+destFilePath));
	filePathDest->second = viewFolder+"/"+destFilePath;

	DebugDialog::debug(QString("++++ copying to %1").arg(filePathDest->first+"/"+filePathDest->second));
}

QString MainPartsEditorWindow::getRandText() {
	QString rand = QUuid::createUuid().toString();
	QString randext = QCryptographicHash::hash(rand.toAscii(),QCryptographicHash::Md4).toHex();
	return randext;
}

void MainPartsEditorWindow::connectViewToConnectorsDock(PartsEditorSketchWidget * view) {
	connect(
		view,SIGNAL(connectorsFound(ItemBase::ViewIdentifier,QStringList)),
		m_connWidget,SLOT(connectorsFound(ItemBase::ViewIdentifier,QStringList))
	);
}

void MainPartsEditorWindow::createMenus(){

	QAction *closeAct = new QAction(tr("&Close"), this);
	closeAct->setShortcut(tr("Ctrl+W"));
	closeAct->setStatusTip(tr("Close the Parts Editor Window"));
	connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));

	QAction *saveAct = new QAction(tr("&Save"), this);
	saveAct->setShortcut(tr("Ctrl+S"));
	saveAct->setStatusTip(tr("Save"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	QAction *saveAsAct = new QAction(tr("&Close"), this);
	saveAsAct->setStatusTip(tr("Save as..."));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(closeAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
}

void MainPartsEditorWindow::createZoomOptions() {
	m_zoomOptsComboBox = new ZoomComboBox(m_tabWidget);
	QRect rect = m_zoomOptsComboBox->geometry();
	rect.moveTo(400,1);
	rect.setHeight(22);
	m_zoomOptsComboBox->setGeometry(rect);

	if (connect(m_zoomOptsComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(updateViewZoom(QString))))
		DebugDialog::debug("connected updateViewZoom");
	else DebugDialog::debug("connect failed on zoom");

    connect(m_breadView, SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));
    connect(m_schemView, SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));
    connect(m_pcbView, SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));
    connect(m_iconView, SIGNAL(zoomChanged(qreal)), this, SLOT(updateZoomOptions(qreal)));

    connect(m_breadView, SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));
	connect(m_schemView, SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));
	connect(m_pcbView, SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));
	connect(m_iconView, SIGNAL(zoomOutOfRange(qreal)), this, SLOT(updateZoomOptionsNoMatterWhat(qreal)));

	connect(m_breadView, SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));
	connect(m_schemView, SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));
	connect(m_pcbView, SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));
	connect(m_iconView, SIGNAL(zoomIn(int)), this, SLOT(zoomIn(int)));

	connect(m_breadView, SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));
	connect(m_schemView, SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));
	connect(m_pcbView, SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));
	connect(m_iconView, SIGNAL(zoomOut(int)), this, SLOT(zoomOut(int)));
}

void MainPartsEditorWindow::zoomIn(int steps) {
	for(int i=0; i < steps; i++) {
		m_zoomOptsComboBox->zoomIn();
	}
}

void MainPartsEditorWindow::zoomOut(int steps) {
	for(int i=0; i < steps; i++) {
		m_zoomOptsComboBox->zoomOut();
	}
}

void MainPartsEditorWindow::updateZoomOptions(qreal zoom) {
	if(!m_comboboxChanged) {
		m_zoomOptsComboBox->setEditText(tr("%1%").arg(zoom));
	} else {
		m_comboboxChanged = false;
	}
}

void MainPartsEditorWindow::updateZoomOptionsNoMatterWhat(qreal zoom) {
	m_zoomOptsComboBox->setEditText(tr("%1%").arg(zoom));
}

void MainPartsEditorWindow::updateViewZoom(QString zoomComboBoxText) {
	if(zoomComboBoxText != "" ) {
		m_comboboxChanged = true;
		QString temptext(zoomComboBoxText);
		PartsEditorSketchWidget * temp = dynamic_cast<PartsEditorSketchWidget *>(m_tabWidget->currentWidget());
		if(temp != NULL) temp->absoluteZoom(temptext.remove("%").toDouble());
	}
}

void MainPartsEditorWindow::updateZoomText(int /*current*/){
	PartsEditorSketchWidget * temp = dynamic_cast<PartsEditorSketchWidget *>(m_tabWidget->currentWidget());
	m_zoomOptsComboBox->setEditText(tr("%1%").arg((int)temp->currentZoom()));
}

void MainPartsEditorWindow::makeDock(const QString & title, QWidget * widget,  Qt::DockWidgetArea area)
{
	static int docMinHeight = 30;
	static int docWidth = 120;

    FDockWidget * dock = new FDockWidget(title, this);
    dock->setStyle(new QCleanlooksStyle());
	dock->setAllowedAreas(area);
    dock->setWidget(widget);
    widget->setParent(dock);

    addDockWidget(area, dock);
    dock->setMinimumSize(dock->minimumSize().width(), docMinHeight);
    dock->resize(docWidth, dock->size().height());
    //connect(dock, SIGNAL(dockChangeActivationSignal(FDockWidget *)), this, SLOT(dockChangeActivation(class FDockWidget *)));
}

void MainPartsEditorWindow::closeEvent(QCloseEvent *event) {
	if(m_tempDir.path() != ".") {
		rmdir(m_tempDir);
		m_tempDir = QDir();
	}
	QMainWindow::closeEvent(event);
	emit closed(m_id);
}

void MainPartsEditorWindow::rmdir(QDir & dir) {
	DebugDialog::debug(QString("removing temp folder: %1").arg(dir.path()));

	QStringList files = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	for(int i=0; i < files.size(); i++) {
		QFile tempFile(dir.path() + "/" +files.at(i));
		DebugDialog::debug(QString("removing temp folder inside original: %1").arg(tempFile.fileName()));
		if(QFileInfo(tempFile.fileName()).isDir()) {
			QDir dir = QDir(tempFile.fileName());
			rmdir(dir);
		} else {
			tempFile.remove(tempFile.fileName());
		}
	}
	dir.rmdir(dir.path());
}

