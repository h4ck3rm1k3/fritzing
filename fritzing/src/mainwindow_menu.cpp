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
#include <QSvgGenerator>
#include <QColor>

#include "mainwindow.h"
#include "debugdialog.h"
#include "waitpushundostack.h"
//#include "partseditor/mainpartseditorwindow.h"
#include "partseditor/partseditormainwindow.h"
#include "aboutbox.h"
#include "autoroute/autorouter1.h"
#include "autoroute/autorouteprogressdialog.h"
#include "items/virtualwire.h"
#include "fsvgrenderer.h"
#include "labels/note.h"
#include "fapplication.h"
#include "svg/svg2gerber.h"
#include "eagle/fritzing2eagle.h"
#include "breadboardsketchwidget.h"
#include "schematicsketchwidget.h"
#include "pcbsketchwidget.h"
#include "partsbinpalette/binmanager/binmanager.h"
#include "expandinglabel.h"
#include "htmlinfoview.h"
#include "utils/bendpointaction.h"
#include "fgraphicsscene.h"
#include "utils/fileprogressdialog.h"
#include "svg/svgfilesplitter.h"

static QString eagleActionType = ".eagle";
static QString gerberActionType = ".gerber";
static QString jpgActionType = ".jpg";
static QString psActionType = ".ps";
static QString pdfActionType = ".pdf";
static QString pngActionType = ".png";
static QString svgActionType = ".svg";
static QString bomActionType = ".txt";

static QHash<QString, QPrinter::OutputFormat> filePrintFormats;
static QHash<QString, QImage::Format> fileExportFormats;
static QHash<QString, QString> fileExtFormats;


bool sortPartList(ItemBase * b1, ItemBase * b2){
    return b1->instanceTitle().toLower() < b2->instanceTitle().toLower();
}

void MainWindow::initExportConstants()
{
	filePrintFormats[pdfActionType] = QPrinter::PdfFormat;
	filePrintFormats[psActionType] = QPrinter::PostScriptFormat;

	fileExportFormats[pngActionType] = QImage::Format_ARGB32;
	fileExportFormats[jpgActionType] = QImage::Format_RGB32;

	fileExtFormats[pdfActionType] = tr("PDF (*.pdf)");
	fileExtFormats[psActionType] = tr("PostScript (*.ps)");
	fileExtFormats[pngActionType] = tr("PNG Image (*.png)");
	fileExtFormats[jpgActionType] = tr("JPEG Image (*.jpg)");
	fileExtFormats[svgActionType] = tr("SVG Image (*.svg)");
	fileExtFormats[bomActionType] = tr("BoM Text File (*.txt)");
}

void MainWindow::print() {
	#ifndef QT_NO_PRINTER
		QPrinter printer(QPrinter::HighResolution);

		QPrintDialog *printDialog = new QPrintDialog(&printer, this);
		if (printDialog->exec() == QDialog::Accepted) {
			printAux(printer,tr("Printing..."));
			m_statusBar->showMessage(tr("Ready"), 2000);
		} else {
			return;
		}
	#endif
}

void MainWindow::exportEtchable() {
	exportEtchable(true, false);
}

void MainWindow::exportEtchableSvg() {
	exportEtchable(false, true);
}

void MainWindow::exportEtchable(bool wantPDF, bool wantSVG)
{
	if (!m_pcbGraphicsView->ratsAllRouted()) {
		QMessageBox msgBox(this);
		msgBox.setText(tr("All traces have not yet been routed."));
		msgBox.setInformativeText(tr("Do you want to proceed anyway?"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.button(QMessageBox::Yes)->setText(tr("Proceed"));
		msgBox.button(QMessageBox::No)->setText(tr("Cancel"));
		msgBox.setDefaultButton(QMessageBox::Yes);
		int ret = msgBox.exec();
		if (ret != QMessageBox::Yes) return;
	}

	QString path = defaultSaveFolder();

	QString fileExt;
	QString extFmt = (wantPDF) ? fileExtFormats.value(pdfActionType) : fileExtFormats.value(svgActionType);
	QString fileName = FApplication::getSaveFileName(this,
		tr("Export Etchable for DIY..."),
		path+"/"+QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)+getExtFromFileDialog(extFmt),
		extFmt,
		&fileExt
	);

	if (fileName.isEmpty()) {
		return; //Cancel pressed
	}

	FileProgressDialog * fileProgressDialog = exportProgress();

	DebugDialog::debug(fileExt+" selected to export");
	fileExt = getExtFromFileDialog(fileExt);
	#ifdef Q_WS_X11
		if(!alreadyHasExtension(fileName)) {
			fileName += fileExt;
		}
	#endif

	QList<ViewLayer::ViewLayerID> viewLayerIDs;
	viewLayerIDs << ViewLayer::Copper0 << ViewLayer::Copper0Trace;
	QSizeF imageSize;
	QString svg = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, NULL);
	if (wantSVG) {
		QString svgFileName = fileName;
		svgFileName.replace(fileExt, ".svg");
		QFile file(svgFileName);
		file.open(QIODevice::WriteOnly);
		QTextStream out(&file);
		out << svg;
		file.close();
	}
	else {
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(filePrintFormats[fileExt]);
		printer.setOutputFileName(fileName);
		QPainter painter;
		if (painter.begin(&printer))
		{
			// now convert to pdf
			QSvgRenderer svgRenderer;
			svgRenderer.load(svg.toLatin1());
			qreal trueWidth = imageSize.width() / FSvgRenderer::printerScale();
			qreal trueHeight = imageSize.height() / FSvgRenderer::printerScale();
			int res = printer.resolution();
			QRectF bounds(0, 0, trueWidth * res, trueHeight * res);
			svgRenderer.render(&painter, bounds);
		}

		painter.end();
	}

	m_statusBar->showMessage(tr("Sketch exported"), 2000);
	delete fileProgressDialog;

/*

	int width = m_pcbGraphicsView->width();
	if (m_pcbGraphicsView->verticalScrollBar()->isVisible()) {
		width -= m_pcbGraphicsView->verticalScrollBar()->width();
	}
	int height = m_pcbGraphicsView->height();
	if (m_pcbGraphicsView->horizontalScrollBar()->isVisible()) {
		height -= m_pcbGraphicsView->horizontalScrollBar()->height();
	}

	double trueWidth = width / m_printerScale;
	double trueHeight = height / m_printerScale;

	// set everything to a 1200 dpi resolution
	QSize imgSize(trueWidth * 1200, trueHeight * 1200);
	QImage image(imgSize, QImage::Format_RGB32);
	image.setDotsPerMeterX(1200 * 39.3700787);
	image.setDotsPerMeterY(1200 * 39.3700787);
	QPainter painter;

	QColor color;
	color = m_pcbGraphicsView->background();
	m_pcbGraphicsView->setBackground(QColor::fromRgb(255,255,255,255));

	m_pcbGraphicsView->scene()->clearSelection();
	m_pcbGraphicsView->saveLayerVisibility();
	m_pcbGraphicsView->setAllLayersVisible(false);
	m_pcbGraphicsView->setLayerVisible(ViewLayer::Copper0, true);
	m_pcbGraphicsView->hideConnectors(true);

	painter.begin(&image);
	m_pcbGraphicsView->render(&painter);
	painter.end();


	QSvgGenerator svgGenerator;
	svgGenerator.setFileName("c:/fritzing2/testsvggenerator.svg");
    svgGenerator.setSize(QSize(width * 8, height * 8));
	QPainter svgPainter(&svgGenerator);
	m_pcbGraphicsView->render(&svgPainter);
	svgPainter.end();


	m_pcbGraphicsView->hideConnectors(false);
	m_pcbGraphicsView->setBackground(color);
	m_pcbGraphicsView->restoreLayerVisibility();
	// TODO: restore the selection

	QRgb black = 0;
	for (int x = 0; x < imgSize.width(); x++) {
		for (int y = 0; y < imgSize.height(); y++) {
			QRgb p = image.pixel(x, y);
			if (p != 0xffffffff) {
				image.setPixel(x, y, black);
			}
		}
	}

	bool result = image.save(fileName);
	if (!result) {
		QMessageBox::warning(this, tr("Fritzing"), tr("Unable to save %1").arg(fileName) );
	}

*/


}

void MainWindow::doExport() {
	QAction * action = qobject_cast<QAction *>(sender());
	if (action == NULL) return;

	QString actionType = action->data().toString();
	QString path = defaultSaveFolder();

	if (actionType.compare(eagleActionType) == 0) {
		exportToEagle();
		return;
	}

	if (actionType.compare(gerberActionType) == 0) {
		exportToGerber();
		return;
	}

	if (actionType.compare(bomActionType) == 0) {
		exportBOM();
		return;
	}

	if (actionType.compare(svgActionType) == 0) {
		exportSvg();
		return;
	}

	#ifndef QT_NO_PRINTER
		QString fileExt;
		QString extFmt = fileExtFormats.value(actionType);
		DebugDialog::debug(QString("file export string %1").arg(extFmt));
		QString fileName = FApplication::getSaveFileName(this,
			tr("Export..."),
			path+"/"+QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)+getExtFromFileDialog(extFmt),
			extFmt,
			&fileExt
		);

		if (fileName.isEmpty()) {
			return; //Cancel pressed
		} else {
			FileProgressDialog * fileProgressDialog = exportProgress();
			fileExt = getExtFromFileDialog(extFmt);
			DebugDialog::debug(fileExt+" selected to export");
			#ifdef Q_WS_X11
				if(!alreadyHasExtension(fileName)) {
					fileName += fileExt;
				}
			#endif

			if(filePrintFormats.contains(fileExt)) { // PDF or PS
				QPrinter printer(QPrinter::HighResolution);
				printer.setOutputFormat(filePrintFormats[fileExt]);
				printer.setOutputFileName(fileName);
				printAux(printer,tr("Exporting..."));
				m_statusBar->showMessage(tr("Sketch exported"), 2000);
			} else { // PNG...
				DebugDialog::debug(QString("format: %1 %2").arg(fileExt).arg(fileExportFormats[fileExt]));
				exportAux(fileName,fileExportFormats[fileExt]);
			}
			delete fileProgressDialog;

		}
	#endif
}

void MainWindow::exportAux(QString fileName, QImage::Format format) {
	int width = m_currentGraphicsView->width();
	if (m_currentGraphicsView->verticalScrollBar()->isVisible()) {
		width -= m_currentGraphicsView->verticalScrollBar()->width();
	}
	int height = m_currentGraphicsView->height();
	if (m_currentGraphicsView->horizontalScrollBar()->isVisible()) {
		height -= m_currentGraphicsView->horizontalScrollBar()->height();
	}
	QSize imgSize(width, height);
	QImage image(imgSize,format);
	image.setDotsPerMeterX(1200*254);
	image.setDotsPerMeterY(1200*254);
	QPainter painter;

	//QColor color;
	//if(true) {
		//color = m_currentWidget->background();
		//m_currentWidget->setBackground(QColor::fromRgb(255,255,255,255));
	//}

	painter.begin(&image);
	m_currentGraphicsView->render(&painter);
	painter.end();

	//if (true) {
		//m_currentWidget->setBackground(color);
	//}

	//QImage bw = image->createHeuristicMask();  // image->createMaskFromColor (Wire::getRgb("trace"), Qt::MaskOutColor );
	//bool result = bw.save(fileName);
	bool result = image.save(fileName);
	if (!result) {
		QMessageBox::warning(this, tr("Fritzing"), tr("Unable to save %1").arg(fileName) );
	}
}

void MainWindow::printAux(QPrinter &printer, const QString & message, bool removeBackground) {
	m_statusBar->showMessage(message);

	QPainter painter;
	if (painter.begin(&printer)) {
		// scale the output
		int res = printer.resolution();

		qreal scale2 = res / FSvgRenderer::printerScale();

		DebugDialog::debug(QString("p.w:%1 p.h:%2 pager.w:%3 pager.h:%4 paperr.w:%5 paperr.h:%6 source.w:%7 source.h:%8")
			.arg(printer.width()).arg(printer.height()).arg(printer.pageRect().width())
			.arg(printer.pageRect().height())
			.arg(printer.paperRect().width()).arg(printer.paperRect().height())
			.arg(printer.width() / scale2)
			.arg(printer.height() / scale2) );

		//QRectF target(0, 0, printer.width(), printer.height());
		//QRectF target = printer.pageRect();
		//QRectF target = printer.paperRect();


		QPointF sceneStart = m_currentGraphicsView->mapToScene(QPoint(0,0));
		QPointF sceneEnd = m_currentGraphicsView->mapToScene(QPoint(m_currentGraphicsView->viewport()->width(), m_currentGraphicsView->viewport()->height()));

		QRectF source(sceneStart, sceneEnd);
		QRectF target(0, 0, source.width() * scale2, source.height() * scale2);

		QColor color;
		if(removeBackground) {
			color = m_currentGraphicsView->background();
			m_currentGraphicsView->setBackground(QColor::fromRgb(255,255,255,255));
		}

		QList<QGraphicsItem*> selItems = m_currentGraphicsView->scene()->selectedItems();
		foreach(QGraphicsItem *item, selItems) {
			item->setSelected(false);
		}

		// render to printer:
		m_currentGraphicsView->scene()->render(&painter, target, source, Qt::KeepAspectRatio);

		foreach(QGraphicsItem *item, selItems) {
			item->setSelected(true);
		}

		if(removeBackground) {
			m_currentGraphicsView->setBackground(color);
		}

		DebugDialog::debug(QString("source w:%1 h:%2 target w:%5 h:%6 pres:%3 screenres:%4")
			.arg(source.width())
			.arg(source.height()).arg(res).arg(this->physicalDpiX())
			.arg(target.width()).arg(target.height()) );

		//#ifndef QT_NO_CONCURRENT
			//QProgressDialog dialog;
			//dialog.setLabelText(message);
	 	//
			// Create a QFutureWatcher and conncect signals and slots.
			//QFutureWatcher<void> futureWatcher;
			//QObject::connect(&futureWatcher, SIGNAL(finished()), &dialog, SLOT(reset()));
			//QObject::connect(&dialog, SIGNAL(canceled()), &futureWatcher, SLOT(cancel()));
			//QObject::connect(&futureWatcher, SIGNAL(progressRangeChanged(int, int)), &dialog, SLOT(setRange(int, int)));
			//QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), &dialog, SLOT(setValue(int)));
		//
			// Start the computation.
			//futureWatcher.setFuture(QtConcurrent::run(painter,&QPainter::end));
			//dialog.exec();
		//
			//futureWatcher.waitForFinished();
		//#endif

		//#ifdef QT_NO_CONCURRENT
			painter.end();
		//#endif
	} else {
		QMessageBox::warning(this, tr("Fritzing"),
			tr("Cannot print to %1").arg("print.pdf"));
	}
}

void MainWindow::saveAsAux(const QString & fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    file.close();

    setReadOnly(false);
    //FritzingWindow::saveAsAux(fileName);

    QApplication::setOverrideCursor(Qt::WaitCursor);
	m_sketchModel->save(fileName);
    QApplication::restoreOverrideCursor();

    m_statusBar->showMessage(tr("Saved '%1'").arg(fileName), 2000);
    setCurrentFile(fileName);

   // mark the stack clean so we update the window dirty flag
    m_undoStack->setClean();
}


void MainWindow::closeIfEmptySketch(MainWindow* mw) {
	int cascFactorX; int cascFactorY;
	// close empty sketch window if user opens from a file
	if (isEmptyFileName(m_fileName, untitledFileName()) && this->undoStackIsEmpty()) {
		QTimer::singleShot(0, this, SLOT(close()) );
		cascFactorX = 0;
		cascFactorY = 0;
	} else {
		cascFactorX = CascadeFactorX;
		cascFactorY = CascadeFactorY;
	}
	mw->move(x()+cascFactorX,y()+cascFactorY);
	mw->show();
}

void MainWindow::load() {
	QString path;
	// if it's the first time load is called use Documents folder
	if(m_firstOpen){
		path = defaultSaveFolder();
		m_firstOpen = false;
	}
	else {
		path = "";
	}

	QString fileName = FApplication::getOpenFileName(
			this,
			tr("Select a Fritzing File to Open"),
			path,
			tr("Fritzing Files (*%1 *%2);;Fritzing (*%1);;Fritzing Shareable (*%2)").arg(FritzingSketchExtension).arg(FritzingBundleExtension)
		);
	if (fileName.isNull()) return;

	if (alreadyOpen(fileName)) return;

	QFile file(fileName);
	if (!file.exists()) {
       QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot find file %1.")
                             .arg(fileName));


		return;
	}



    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fritzing"),
                             tr("Cannot read file  1 %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    file.close();

    MainWindow* mw = newMainWindow(m_paletteModel, m_refModel, fileName, true);
	mw->loadWhich(fileName);
    mw->clearFileProgressDialog();
	closeIfEmptySketch(mw);
}

bool MainWindow::loadWhich(const QString & fileName, bool setAsLastOpened, bool addToRecent)
{
	bool result = false;
    if(fileName.endsWith(FritzingSketchExtension)) {
    	load(fileName, setAsLastOpened, addToRecent);
		result = true;
    } else if(fileName.endsWith(FritzingBundleExtension)) {
    	loadBundledSketch(fileName);
		result = true;
    } else if (fileName.endsWith(FritzingBinExtension)) {
		m_paletteWidget->load(fileName);
		result = true;
	} else if (fileName.endsWith(FritzingPartExtension)) {
		notYetImplemented(tr("directly loading parts"));
	} else if (fileName.endsWith(FritzingModuleExtension)) {
		load(fileName, false, false);
	}  else if (fileName.endsWith(FritzingBundledPartExtension)) {
		loadBundledPart(fileName);
		result = true;
	}

	if (result) {
		this->show();
	}

	return result;
}

void MainWindow::load(const QString & fileName, bool setAsLastOpened, bool addToRecent) {

	if (m_fileProgressDialog) {
		m_fileProgressDialog->setMaximum(100);
		m_fileProgressDialog->setValue(2);
	}
	this->show();
	showAllFirstTimeHelp(false);
	QApplication::processEvents();

	QFileInfo fileInfo(fileName);

	if (m_fileProgressDialog) {
		m_fileProgressDialog->setMessage(tr("loading %1 (model)").arg(fileInfo.fileName()));
		m_fileProgressDialog->setMaximum(100);
		m_fileProgressDialog->setValue(10);
	}
	QApplication::processEvents();


	QList<ModelPart *> modelParts;
	m_sketchModel->load(fileName, m_paletteModel, modelParts);

	QApplication::processEvents();
	if (m_fileProgressDialog) {
		m_fileProgressDialog->setValue(55);
		m_fileProgressDialog->setMessage(tr("loading %1 (breadboard)").arg(fileInfo.fileName()));
	}

	m_breadboardGraphicsView->loadFromModel(modelParts, BaseCommand::SingleView, NULL, false, false);

	QApplication::processEvents();
	if (m_fileProgressDialog) {
		m_fileProgressDialog->setValue(70);
		m_fileProgressDialog->setMessage(tr("loading %1 (pcb)").arg(fileInfo.fileName()));
	}

	m_pcbGraphicsView->loadFromModel(modelParts, BaseCommand::SingleView, NULL, false, false);

	QApplication::processEvents();
	if (m_fileProgressDialog) {
		m_fileProgressDialog->setValue(85);
		m_fileProgressDialog->setMessage(tr("loading %1 (schematic)").arg(fileInfo.fileName()));
	}

	m_schematicGraphicsView->loadFromModel(modelParts, BaseCommand::SingleView, NULL, false, false);

	QApplication::processEvents();
	if (m_fileProgressDialog) {
		m_fileProgressDialog->setValue(98);
	}

	if(setAsLastOpened) {
		QSettings settings;
		settings.setValue("lastOpenSketch",fileName);
	}

	setCurrentFile(fileName, addToRecent);

	UntitledSketchIndex--;
}

void MainWindow::copy() {
	if (m_currentGraphicsView == NULL) return;
	m_currentGraphicsView->copy();
}

void MainWindow::cut() {
	if (m_currentGraphicsView == NULL) return;
	m_currentGraphicsView->cut();
}

void MainWindow::paste() {
	if (m_currentGraphicsView == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard == NULL) {
		// shouldn't happen
		return;
	}

	const QMimeData* mimeData = clipboard->mimeData(QClipboard::Clipboard);
	if (mimeData == NULL) return;

   	if (!mimeData->hasFormat("application/x-dnditemsdata")) return;

    QByteArray itemData = mimeData->data("application/x-dnditemsdata");
	QList<ModelPart *> modelParts;
	if (((ModelBase *) m_sketchModel)->paste(m_paletteModel, itemData, modelParts, NULL)) {
		QUndoCommand * parentCommand = new QUndoCommand("Paste");

		m_breadboardGraphicsView->loadFromModel(modelParts, BaseCommand::SingleView, parentCommand, true, true);
		m_pcbGraphicsView->loadFromModel(modelParts, BaseCommand::SingleView, parentCommand, true, true);
		m_schematicGraphicsView->loadFromModel(modelParts, BaseCommand::SingleView, parentCommand, true, true);

		m_undoStack->push(parentCommand);
	}
}

void MainWindow::duplicate() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->copy();
	paste();

	//m_currentGraphicsView->duplicate();
}

void MainWindow::doDelete() {
	//DebugDialog::debug(QString("invoking do delete") );

	if (m_currentGraphicsView != NULL) {
		m_currentGraphicsView->deleteItem();
	}
}

void MainWindow::selectAll() {
	if (m_currentGraphicsView != NULL) {
		m_currentGraphicsView->selectDeselectAllCommand(true);
	}
}

void MainWindow::deselect() {
	if (m_currentGraphicsView != NULL) {
		m_currentGraphicsView->selectDeselectAllCommand(false);
	}
}

void MainWindow::about()
{
	AboutBox::showAbout();
}

void MainWindow::createActions()
{
	m_raiseWindowAct = new QAction(m_fileName, this);
	m_raiseWindowAct->setCheckable(true);
	connect( m_raiseWindowAct, SIGNAL(triggered()), this, SLOT(raiseAndActivate()));
	updateRaiseWindowAction();

    createFileMenuActions();
    createEditMenuActions();
    createPartMenuActions();
    createViewMenuActions();
    createWindowMenuActions();
    createHelpMenuActions();
	createTraceMenuActions();
}

void MainWindow::createFileMenuActions() {
	m_newAct = new QAction(tr("&New"), this);
	m_newAct->setShortcut(tr("Ctrl+N"));
	m_newAct->setStatusTip(tr("Create a new sketch"));
	connect(m_newAct, SIGNAL(triggered()), this, SLOT(createNewSketch()));

	m_openAct = new QAction(tr("&Open..."), this);
	m_openAct->setShortcut(tr("Ctrl+O"));
	m_openAct->setStatusTip(tr("Open a sketch"));
	connect(m_openAct, SIGNAL(triggered()), this, SLOT(load()));

	createOpenRecentMenu();
	createOpenExampleMenu();
	createCloseAction();

	m_saveAct = new QAction(tr("&Save"), this);
	m_saveAct->setShortcut(tr("Ctrl+S"));
	m_saveAct->setStatusTip(tr("Save the current sketch"));
	connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

	m_saveAsAct = new QAction(tr("&Save As..."), this);
	m_saveAsAct->setShortcut(tr("Shift+Ctrl+S"));
	m_saveAsAct->setStatusTip(tr("Save the current sketch"));
	connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	m_saveAsBundledAct = new QAction(tr("Save As Shareable..."), this);
	m_saveAsBundledAct->setShortcut(tr("Alt+Ctrl+S"));
	m_saveAsBundledAct->setStatusTip(tr("Export current sketch and its non-core parts"));
	connect(m_saveAsBundledAct, SIGNAL(triggered()), this, SLOT(saveBundledSketch()));

	m_saveAsModuleAct = new QAction(tr("Save As Module..."), this);
	m_saveAsModuleAct->setStatusTip(tr("Export current sketch as a standalone module"));
	connect(m_saveAsModuleAct, SIGNAL(triggered()), this, SLOT(saveAsModule()));

	m_editModuleAct = new QAction(tr("Open Module as Sketch"), this);
	m_editModuleAct->setStatusTip(tr("Open selected module as a sketch (for editing)"));
	connect(m_editModuleAct, SIGNAL(triggered()), this, SLOT(editModule()));

	m_exportJpgAct = new QAction(tr("to &JPG..."), this);
	m_exportJpgAct->setData(jpgActionType);
	m_exportJpgAct->setStatusTip(tr("Export the visible area of the current sketch as a JPG image"));
	connect(m_exportJpgAct, SIGNAL(triggered()), this, SLOT(doExport()));

	m_exportPngAct = new QAction(tr("to P&NG..."), this);
	m_exportPngAct->setData(pngActionType);
	m_exportPngAct->setStatusTip(tr("Export the visible area of the current sketch as a PNG image"));
	connect(m_exportPngAct, SIGNAL(triggered()), this, SLOT(doExport()));

	m_exportPsAct = new QAction(tr("to Post&Script..."), this);
	m_exportPsAct->setData(psActionType);
	m_exportPsAct->setStatusTip(tr("Export the visible area of the current sketch as a PostScript image"));
	connect(m_exportPsAct, SIGNAL(triggered()), this, SLOT(doExport()));

	m_exportPdfAct = new QAction(tr("to &PDF..."), this);
	m_exportPdfAct->setData(pdfActionType);
	m_exportPdfAct->setStatusTip(tr("Export the visible area of the current sketch as a PDF image"));
	connect(m_exportPdfAct, SIGNAL(triggered()), this, SLOT(doExport()));

	m_exportSvgAct = new QAction(tr("to &SVG..."), this);
	m_exportSvgAct->setData(svgActionType);
	m_exportSvgAct->setStatusTip(tr("Export the current sketch as an SVG image"));
	connect(m_exportSvgAct, SIGNAL(triggered()), this, SLOT(doExport()));

    m_exportBomAct = new QAction(tr("List of parts (&Bill of Materials)"), this);
    m_exportBomAct->setData(bomActionType);
    m_exportBomAct->setStatusTip(tr("Save a Bill of Materials (BoM)/Shopping List as text"));
    connect(m_exportBomAct, SIGNAL(triggered()), this, SLOT(doExport()));

	m_exportEagleAct = new QAction(tr("to &Eagle..."), this);
	m_exportEagleAct->setData(eagleActionType);
	m_exportEagleAct->setStatusTip(tr("Export the current sketch to Eagle CAD"));
	connect(m_exportEagleAct, SIGNAL(triggered()), this, SLOT(doExport()));

	m_exportGerberAct = new QAction(tr("to &Gerber..."), this);
	m_exportGerberAct->setData(gerberActionType);
	m_exportGerberAct->setStatusTip(tr("Export the current sketch to Gerber"));
	connect(m_exportGerberAct, SIGNAL(triggered()), this, SLOT(doExport()));

	m_exportEtchableAct = new QAction(tr("Etchable PDF..."), this);
	m_exportEtchableAct->setStatusTip(tr("Export the current sketch to PDF for DIY production"));
	connect(m_exportEtchableAct, SIGNAL(triggered()), this, SLOT(exportEtchable()));

	m_exportEtchableSvgAct = new QAction(tr("Etchable SVG..."), this);
	m_exportEtchableSvgAct->setStatusTip(tr("Export the current sketch to SVG for DIY production"));
	connect(m_exportEtchableSvgAct, SIGNAL(triggered()), this, SLOT(exportEtchableSvg()));

	/*m_pageSetupAct = new QAction(tr("&Page Setup..."), this);
	m_pageSetupAct->setShortcut(tr("Shift+Ctrl+P"));
	m_pageSetupAct->setStatusTip(tr("Setup the current sketch page"));
	connect(m_pageSetupAct, SIGNAL(triggered()), this, SLOT(pageSetup()));*/

	m_printAct = new QAction(tr("&Print..."), this);
	m_printAct->setShortcut(tr("Ctrl+P"));
	m_printAct->setStatusTip(tr("Print the current view"));
	connect(m_printAct, SIGNAL(triggered()), this, SLOT(print()));

	m_quitAct = new QAction(tr("&Quit"), this);
	m_quitAct->setShortcut(tr("Ctrl+Q"));
	m_quitAct->setStatusTip(tr("Quit the application"));
	connect(m_quitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	m_quitAct->setMenuRole(QAction::QuitRole);

}

void MainWindow::createOpenExampleMenu() {
	m_openExampleMenu = new QMenu(tr("&Open Example"), this);
	QString folderPath = getApplicationSubFolderPath("sketches")+"/";
	populateMenuFromXMLFile(m_openExampleMenu, m_openExampleActions, folderPath, "index.xml"/*, "fritzing-sketches", "sketch", "category"*/);
}

void MainWindow::populateMenuFromXMLFile(
		QMenu *parentMenu, QStringList &actionsTracker,
		const QString &folderPath, const QString &indexFileName/*,
		const QString &rootNode, const QString &indexNode,
		const QString &submenuNode*/
) {
	QDomDocument dom;
	QFile file(folderPath+indexFileName);
	dom.setContent(&file);
	file.close();

	QDomElement domElem = dom.documentElement();
	QDomElement indexDomElem = domElem.firstChild().toElement();
	QDomElement taxonomyDomElem = indexDomElem.nextSiblingElement("categories");

	SketchIndex index = indexAvailableElements(indexDomElem,folderPath);
	populateMenuWithIndex(index,parentMenu,actionsTracker,taxonomyDomElem);
	foreach (SketchDescriptor * sketchDescriptor, index.values()) {
		delete sketchDescriptor;
	}
}

SketchIndex MainWindow::indexAvailableElements(QDomElement &domElem, const QString &srcPrefix) {
	SketchIndex retval;
	QDomNode n = domElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement();
		if(!e.isNull() && e.tagName() == "sketch") {
			const QString id = e.attribute("id");
			const QString name = e.attribute("name");
			QString srcAux = e.attribute("src");
			// if it's an absolute path, don't prefix it
			const QString src = QFileInfo(srcAux).exists()? srcAux: srcPrefix+srcAux;
			retval[id] = new SketchDescriptor(id,name,src);
		}
		n = n.nextSibling();
	}
	return retval;
}

void MainWindow::populateMenuWithIndex(const SketchIndex &index, QMenu * parentMenu, QStringList &actionsTracker, QDomElement &domElem) {
	QDomNode n = domElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if(!e.isNull()) {
			if(e.nodeName() == "sketch") {
				QString id = e.attribute("id");
				if(!id.isNull() && !id.isEmpty()) {
					SketchDescriptor elem = *index[id];
					if(QFileInfo(elem.src).exists()) {
						actionsTracker << elem.name;
						QAction * currAction = new QAction(elem.name, this);
						currAction->setData(elem.src);
						connect(currAction,SIGNAL(triggered()),this,SLOT(openRecentOrExampleFile()));
						parentMenu->addAction(currAction);
					}
				}
			} else if(e.nodeName() == "category") {
				QString name = e.attribute("name");
				QMenu * currMenu = new QMenu(name, parentMenu);
				parentMenu->addMenu(currMenu);
				populateMenuWithIndex(index, currMenu, actionsTracker, e);
			}
		}
		n = n.nextSibling();
	}
}

void MainWindow::populateMenuFromFolderContent(QMenu * parentMenu, const QString &path) {
	QDir *currDir = new QDir(path);
	QStringList content = currDir->entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	if(content.size() > 0) {
		for(int i=0; i < content.size(); i++) {
			QString currFile = content.at(i);
			QString currFilePath = currDir->absolutePath()+"/"+currFile;
			if(QFileInfo(currFilePath).isDir()) {
				QMenu * currMenu = new QMenu(currFile, parentMenu);
				parentMenu->addMenu(currMenu);
				populateMenuFromFolderContent(currMenu, currFilePath);
			} else {
				QString actionText = currFile.remove(FritzingSketchExtension);
				m_openExampleActions << actionText;
				QAction * currAction = new QAction(actionText, this);
				currAction->setData(currFilePath);
				connect(currAction,SIGNAL(triggered()),this,SLOT(openRecentOrExampleFile()));
				parentMenu->addAction(currAction);
			}
		}
	} else {
		parentMenu->setEnabled(false);
	}
	delete currDir;
}

void MainWindow::createOpenRecentMenu() {
	m_openRecentFileMenu = new QMenu(tr("&Open Recent Files"), this);

	for (int i = 0; i < MaxRecentFiles; ++i) {
		m_openRecentFileActs[i] = new QAction(this);
		m_openRecentFileActs[i]->setVisible(false);
		connect(m_openRecentFileActs[i], SIGNAL(triggered()),this, SLOT(openRecentOrExampleFile()));
	}


    for (int i = 0; i < MaxRecentFiles; ++i) {
    	m_openRecentFileMenu->addAction(m_openRecentFileActs[i]);
    }
    updateRecentFileActions();
}

void MainWindow::updateFileMenu() {
	updateRecentFileActions();
	bool enabled = false;
	if (m_currentGraphicsView && m_currentGraphicsView->scene()) {
		foreach (QGraphicsItem * item, m_currentGraphicsView->scene()->items()) {
			if (dynamic_cast<ItemBase *>(item) != NULL) {
				enabled = true;
				break;
			}
		}
	}
	m_saveAsModuleAct->setEnabled(enabled);

	enabled = false;
	QList<QGraphicsItem *> items = m_currentGraphicsView->scene()->selectedItems();
	if (items.count() == 1) {
		ItemBase * item = dynamic_cast<ItemBase *>(items[0]);
		if (item != NULL) {
			enabled = (item->itemType() == ModelPart::Module);
		}
	}

	m_editModuleAct->setEnabled(enabled);
}

void MainWindow::updateRecentFileActions() {
	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();
	int ix = 0;
	for (int i = 0; i < files.size(); ++i) {
		QFileInfo finfo(files[i]);
		if (!finfo.exists()) continue;

		QString text = tr("&%1 %2").arg(ix + 1).arg(finfo.fileName());
		m_openRecentFileActs[ix]->setText(text);
		m_openRecentFileActs[ix]->setData(files[i]);
		m_openRecentFileActs[ix]->setVisible(true);
		if (++ix >= (int) MaxRecentFiles) {
			break;
		}
	}

	for (int j = ix; j < MaxRecentFiles; ++j) {
		m_openRecentFileActs[j]->setVisible(false);
	}

	m_openRecentFileMenu->setEnabled(ix > 0);
}

void MainWindow::createEditMenuActions() {
	m_undoAct = m_undoGroup->createUndoAction(this, tr("Undo"));
	m_undoAct->setShortcuts(QKeySequence::Undo);
	m_undoAct->setText(tr("Undo"));

	m_redoAct = m_undoGroup->createRedoAction(this, tr("Redo"));
	m_redoAct->setShortcuts(QKeySequence::Redo);
	m_redoAct->setText(tr("Redo"));

	m_cutAct = new QAction(tr("&Cut"), this);
	m_cutAct->setShortcut(tr("Ctrl+X"));
	m_cutAct->setStatusTip(tr("Cut selection"));
	connect(m_cutAct, SIGNAL(triggered()), this, SLOT(cut()));

	m_copyAct = new QAction(tr("&Copy"), this);
	m_copyAct->setShortcut(tr("Ctrl+C"));
	m_copyAct->setStatusTip(tr("Copy selection"));
	connect(m_copyAct, SIGNAL(triggered()), this, SLOT(copy()));

	m_pasteAct = new QAction(tr("&Paste"), this);
	m_pasteAct->setShortcut(tr("Ctrl+V"));
	m_pasteAct->setStatusTip(tr("Paste clipboard contents"));
	connect(m_pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

	m_duplicateAct = new QAction(tr("&Duplicate"), this);
	m_duplicateAct->setShortcut(tr("Ctrl+D"));
	m_duplicateAct->setStatusTip(tr("Duplicate selection"));
	connect(m_duplicateAct, SIGNAL(triggered()), this, SLOT(duplicate()));

	m_deleteAct = new QAction(tr("&Delete"), this);

	#ifdef Q_WS_MAC
		m_deleteAct->setShortcut(Qt::Key_Backspace);
	#else
		m_deleteAct->setShortcut(QKeySequence::Delete);
	#endif

	m_deleteAct->setStatusTip(tr("Delete selection"));
	connect(m_deleteAct, SIGNAL(triggered()), this, SLOT(doDelete()));

	m_selectAllAct = new QAction(tr("&Select All"), this);
	m_selectAllAct->setShortcut(tr("Ctrl+A"));
	m_selectAllAct->setStatusTip(tr("Select all elements"));
	connect(m_selectAllAct, SIGNAL(triggered()), this, SLOT(selectAll()));

	m_deselectAct = new QAction(tr("&Deselect"), this);
	m_deselectAct->setStatusTip(tr("Deselect"));
	connect(m_deselectAct, SIGNAL(triggered()), this, SLOT(deselect()));

	m_addNoteAct = new QAction(tr("Add Note"), this);
	m_addNoteAct->setStatusTip(tr("Add a note"));
	connect(m_addNoteAct, SIGNAL(triggered()), this, SLOT(addNote()));

	m_preferencesAct = new QAction(tr("&Preferences..."), this);
	m_preferencesAct->setStatusTip(tr("Show the application's about box"));
	m_preferencesAct->setMenuRole(QAction::PreferencesRole);						// make sure this is added to the correct menu on mac
	connect(m_preferencesAct, SIGNAL(triggered()), QApplication::instance(), SLOT(preferences()));
}

void MainWindow::createPartMenuActions() {
	// TODO PARTS EDITOR REMOVE
    /*m_createNewPartActInOldEditor = new QAction(tr("&Create New Part in Old Editor..."), this);
	connect(m_createNewPartActInOldEditor, SIGNAL(triggered()), this, SLOT(createNewPartInOldEditor()));*/

	m_createNewPart = new QAction(tr("&New"), this);
	m_createNewPart->setShortcut(tr("Alt+Ctrl+N"));
	m_createNewPart->setStatusTip(tr("Create new part"));
	connect(m_createNewPart, SIGNAL(triggered()), this, SLOT(createNewPart()));

	m_openInPartsEditorAct = new QAction(tr("&Edit"), this);
	m_openInPartsEditorAct->setShortcut(tr("Ctrl+Return"));
	m_openInPartsEditorAct->setStatusTip(tr("Open the old parts editor"));
	connect(m_openInPartsEditorAct, SIGNAL(triggered()), this, SLOT(openInPartsEditor()));

	m_addToBinMenu = new QMenu(tr("&Add to bin..."), this);
	m_addToBinMenu->setStatusTip(tr("Add selected part to bin"));

	// TODO PARTS EDITOR REMOVE
	/*m_openInOldPartsEditorAct = new QAction(tr("&Open in Old Parts Editor"), this);
	connect(m_openInOldPartsEditorAct, SIGNAL(triggered()), this, SLOT(openInOldPartsEditor()));*/

#ifndef QT_NO_DEBUG
	m_infoViewOnHoverAction = new QAction(tr("Update InfoView on hover"), this);
	m_infoViewOnHoverAction->setCheckable(true);
	bool infoViewOnHover = false;
	m_infoViewOnHoverAction->setChecked(infoViewOnHover);
	setInfoViewOnHover(infoViewOnHover);
	connect(m_infoViewOnHoverAction, SIGNAL(toggled(bool)), this, SLOT(setInfoViewOnHover(bool)));
#endif

	m_rotate90cwAct = new QAction(tr("&Rotate 90\x00B0 Clockwise"), this);
	m_rotate90cwAct->setShortcut(tr("Ctrl+R"));
	m_rotate90cwAct->setStatusTip(tr("Rotate the selected parts by 90 degrees clockwise"));
	connect(m_rotate90cwAct, SIGNAL(triggered()), this, SLOT(rotate90cw()));

	m_rotate180Act = new QAction(tr("&Rotate 180\x00B0"), this);
	m_rotate180Act->setStatusTip(tr("Rotate the selected parts by 180 degrees"));
	connect(m_rotate180Act, SIGNAL(triggered()), this, SLOT(rotate180()));

	m_rotate90ccwAct = new QAction(tr("&Rotate 90\x00B0 Counter Clockwise"), this);
	m_rotate90ccwAct->setShortcut(tr("Alt+Ctrl+R"));
	m_rotate90ccwAct->setStatusTip(tr("Rotate current selection 90 degrees counter clockwise"));
	connect(m_rotate90ccwAct, SIGNAL(triggered()), this, SLOT(rotate90ccw()));

	m_flipHorizontalAct = new QAction(tr("&Flip Horizontal"), this);
	m_flipHorizontalAct->setStatusTip(tr("Flip current selection horizontally"));
	connect(m_flipHorizontalAct, SIGNAL(triggered()), this, SLOT(flipHorizontal()));

	m_flipVerticalAct = new QAction(tr("&Flip Vertical"), this);
	m_flipVerticalAct->setStatusTip(tr("Flip current selection vertically"));
	connect(m_flipVerticalAct, SIGNAL(triggered()), this, SLOT(flipVertical()));

	m_bringToFrontAct = new QAction(tr("Bring to Front"), this);
	m_bringToFrontAct->setShortcut(tr("Shift+Ctrl+]"));
    m_bringToFrontAct->setStatusTip(tr("Bring selected object(s) to front of their layer"));
    connect(m_bringToFrontAct, SIGNAL(triggered()), this, SLOT(bringToFront()));

	m_bringForwardAct = new QAction(tr("Bring Forward"), this);
	m_bringForwardAct->setShortcut(tr("Ctrl+]"));
    m_bringForwardAct->setStatusTip(tr("Bring selected object(s) forward in their layer"));
    connect(m_bringForwardAct, SIGNAL(triggered()), this, SLOT(bringForward()));

	m_sendBackwardAct = new QAction(tr("Send Backward"), this);
	m_sendBackwardAct->setShortcut(tr("Ctrl+["));
    m_sendBackwardAct->setStatusTip(tr("Send selected object(s) back in their layer"));
    connect(m_sendBackwardAct, SIGNAL(triggered()), this, SLOT(sendBackward()));

	m_sendToBackAct = new QAction(tr("Send to Back"), this);
	m_sendToBackAct->setShortcut(tr("Shift+Ctrl+["));
    m_sendToBackAct->setStatusTip(tr("Send selected object(s) to the back of their layer"));
    connect(m_sendToBackAct, SIGNAL(triggered()), this, SLOT(sendToBack()));

    /*m_deleteItemAct = new QAction(tr("&Delete"), this);
    m_deleteItemAct->setShortcut(tr("Delete"));
    m_deleteItemAct->setStatusTip(tr("Delete item from sketch"));
    connect(m_deleteItemAct, SIGNAL(triggered()), this, SLOT(deleteItem()));*/

    m_groupAct = new QAction(tr("&Group"), this);
	m_groupAct->setShortcut(tr("Ctrl+G"));
	m_groupAct->setStatusTip(tr("Group multiple items"));
	connect(m_groupAct, SIGNAL(triggered()), this, SLOT(group()));

	m_showAllLayersAct = new QAction(tr("&Show All Layers"), this);
	m_showAllLayersAct->setStatusTip(tr("Show all the available layers for the current view"));
	connect(m_showAllLayersAct, SIGNAL(triggered()), this, SLOT(showAllLayers()));

	m_hideAllLayersAct = new QAction(tr("&Hide All Layers"), this);
	m_hideAllLayersAct->setStatusTip(tr("Hide all the layers of the current view"));
	connect(m_hideAllLayersAct, SIGNAL(triggered()), this, SLOT(hideAllLayers()));

	m_showPartLabelAct = new QAction(tr("&Show part label"), this);
	m_showPartLabelAct->setStatusTip(tr("Show or hide the label for the selected parts"));
	m_showPartLabelAct->setCheckable(true);
	connect(m_showPartLabelAct, SIGNAL(triggered()), this, SLOT(showPartLabels()));

	m_loadBundledPart = new QAction(tr("&Import..."), this);
	m_loadBundledPart->setStatusTip(tr("Import a part"));
	connect(m_loadBundledPart, SIGNAL(triggered()), this, SLOT(loadBundledPart()));

	m_saveBundledPart = new QAction(tr("&Export..."), this);
	m_saveBundledPart->setStatusTip(tr("Export selected part"));
	connect(m_saveBundledPart, SIGNAL(triggered()), this, SLOT(saveBundledPart()));

	m_addBendpointAct = new BendpointAction(tr("Add Bendpoint"), this);
	m_addBendpointAct->setStatusTip(tr("Add a bendpoint to the selected wire"));
	connect(m_addBendpointAct, SIGNAL(triggered()), this, SLOT(addBendpoint()));
}

void MainWindow::createViewMenuActions() {
	m_zoomInAct = new QAction(tr("&Zoom In"), this);
	m_zoomInAct->setShortcut(tr("Ctrl++"));
	m_zoomInAct->setStatusTip(tr("Zoom in"));
	connect(m_zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

	// instead of creating a filter to grab the shortcut, let's create a new action
	// and append it to the window
	QAction *zoomInAux = new QAction(this);
	zoomInAux->setShortcut(tr("Ctrl+="));
	connect(zoomInAux, SIGNAL(triggered()), this, SLOT(zoomIn()));
	this->addAction(zoomInAux);

	m_zoomOutAct = new QAction(tr("&Zoom Out"), this);
	m_zoomOutAct->setShortcut(tr("Ctrl+-"));
	m_zoomOutAct->setStatusTip(tr("Zoom out"));
	connect(m_zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

	m_fitInWindowAct = new QAction(tr("&Fit in Window"), this);
	m_fitInWindowAct->setShortcut(tr("Ctrl+0"));
	m_fitInWindowAct->setStatusTip(tr("Fit in window"));
	connect(m_fitInWindowAct, SIGNAL(triggered()), this, SLOT(fitInWindow()));

	m_actualSizeAct = new QAction(tr("&Actual Size"), this);
	m_actualSizeAct->setShortcut(tr("Shift+Ctrl+0"));
	m_actualSizeAct->setStatusTip(tr("Actual size"));
	connect(m_actualSizeAct, SIGNAL(triggered()), this, SLOT(actualSize()));

	m_showBreadboardAct = new QAction(tr("&Show Breadboard"), this);
	m_showBreadboardAct->setShortcut(tr("Ctrl+1"));
	m_showBreadboardAct->setStatusTip(tr("Show the breadboard view"));
	connect(m_showBreadboardAct, SIGNAL(triggered()), this, SLOT(showBreadboardView()));

	m_showSchematicAct = new QAction(tr("&Show Schematic"), this);
	m_showSchematicAct->setShortcut(tr("Ctrl+2"));
	m_showSchematicAct->setStatusTip(tr("Show the schematic view"));
	connect(m_showSchematicAct, SIGNAL(triggered()), this, SLOT(showSchematicView()));

	m_showPCBAct = new QAction(tr("&Show PCB"), this);
	m_showPCBAct->setShortcut(tr("Ctrl+3"));
	m_showPCBAct->setStatusTip(tr("Show the PCB view"));
	connect(m_showPCBAct, SIGNAL(triggered()), this, SLOT(showPCBView()));
}

void MainWindow::createWindowMenuActions() {
	m_minimizeAct = new QAction(tr("&Minimize"), this);
	m_minimizeAct->setShortcut(tr("Ctrl+M"));
	m_minimizeAct->setStatusTip(tr("Minimize current window"));
	connect(m_minimizeAct, SIGNAL(triggered(bool)), this, SLOT(minimize()));

	/*
	m_toggleToolbarAct = new QAction(tr("&Toolbar"), this);
	m_toggleToolbarAct->setShortcut(tr("Shift+Ctrl+T"));
	m_toggleToolbarAct->setCheckable(true);
	m_toggleToolbarAct->setChecked(true);
	m_toggleToolbarAct->setStatusTip(tr("Toggle Toolbar visibility"));
	connect(m_toggleToolbarAct, SIGNAL(triggered(bool)), this, SLOT(toggleToolbar(bool)));
	*/

    m_toggleDebuggerOutputAct = new QAction(tr("Debugger Output"), this);
    m_toggleDebuggerOutputAct->setCheckable(true);
   	connect(m_toggleDebuggerOutputAct, SIGNAL(triggered(bool)), this, SLOT(toggleDebuggerOutput(bool)));
}

void MainWindow::createHelpMenuActions() {
	m_openHelpAct = new QAction(tr("Online Tutorials"), this);
	m_openHelpAct->setShortcut(tr("Ctrl+?"));
	m_openHelpAct->setStatusTip(tr("Open Fritzing help"));
	connect(m_openHelpAct, SIGNAL(triggered(bool)), this, SLOT(openHelp()));

	m_examplesAct = new QAction(tr("Online Projects Gallery"), this);
	m_examplesAct->setStatusTip(tr("Open Fritzing examples"));
	connect(m_examplesAct, SIGNAL(triggered(bool)), this, SLOT(openExamples()));

	m_partsRefAct = new QAction(tr("Online Parts Reference"), this);
	m_partsRefAct->setStatusTip(tr("Open Parts Reference"));
	connect(m_partsRefAct, SIGNAL(triggered(bool)), this, SLOT(openPartsReference()));

	m_showInViewHelpAct = new QAction(tr("First Time Help"), this);
	m_showInViewHelpAct->setStatusTip(tr("Show or Hide First Time Help"));
	m_showInViewHelpAct->setCheckable(true);
	m_showInViewHelpAct->setChecked(true);
	connect(m_showInViewHelpAct, SIGNAL(triggered(bool)), this, SLOT(showInViewHelp()));

	/*m_visitFritzingDotOrgAct = new QAction(tr("Visit fritzing.org"), this);
	m_visitFritzingDotOrgAct->setStatusTip(tr("www.fritzing.org"));
	connect(m_visitFritzingDotOrgAct, SIGNAL(triggered(bool)), this, SLOT(visitFritzingDotOrg()));*/

	m_checkForUpdatesAct = new QAction(tr("Check for updates..."), this);
	m_checkForUpdatesAct->setStatusTip(tr("Check whether a newer version of Fritzing is available for download"));
	connect(m_checkForUpdatesAct, SIGNAL(triggered()), QApplication::instance(), SLOT(checkForUpdates()));

	m_aboutAct = new QAction(tr("&About"), this);
	m_aboutAct->setStatusTip(tr("Show the application's about box"));
	connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));
	m_aboutAct->setMenuRole(QAction::AboutRole);

	m_aboutQtAct = new QAction(tr("&About Qt"), this);
	m_aboutQtAct->setStatusTip(tr("Show Qt's about box"));
	connect(m_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	m_reportBugAct = new QAction(tr("&Report a bug..."), this);
	m_reportBugAct->setStatusTip(tr("Report a but you've found in Fritzing"));
	connect(m_reportBugAct, SIGNAL(triggered()), this, SLOT(reportBug()));

	m_importFilesFromPrevInstallAct = new QAction(tr("&Import parts and bins from old version..."), this);
	m_importFilesFromPrevInstallAct->setStatusTip(tr("Import parts and bins from previous installation"));
	connect(m_importFilesFromPrevInstallAct, SIGNAL(triggered()), this, SLOT(importFilesFromPrevInstall()));
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAct);
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addMenu(m_openRecentFileMenu);
    m_fileMenu->addMenu(m_openExampleMenu);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_closeAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);
    m_fileMenu->addAction(m_saveAsBundledAct);
#ifndef QT_NO_DEBUG
    m_fileMenu->addAction(m_saveAsModuleAct);
    m_fileMenu->addAction(m_editModuleAct);
#endif
    m_fileMenu->addSeparator();
	m_exportMenu = m_fileMenu->addMenu(tr("&Export"));
    //m_fileMenu->addAction(m_pageSetupAct);
    m_fileMenu->addAction(m_printAct);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_quitAct);
    connect(m_fileMenu, SIGNAL(aboutToShow()), this, SLOT(updateFileMenu()));


	m_exportMenu->addAction(m_exportPdfAct);
	m_exportMenu->addAction(m_exportPsAct);
	m_exportMenu->addAction(m_exportSvgAct);
	m_exportMenu->addAction(m_exportPngAct);
	m_exportMenu->addAction(m_exportJpgAct);
	m_exportMenu->addSeparator();
	m_exportMenu->addAction(m_exportBomAct);
	m_exportMenu->addAction(m_exportEtchableAct);
	m_exportMenu->addAction(m_exportEtchableSvgAct);
	m_exportMenu->addAction(m_exportEagleAct);
	m_exportMenu->addAction(m_exportGerberAct);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_undoAct);
    m_editMenu->addAction(m_redoAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_cutAct);
    m_editMenu->addAction(m_copyAct);
    m_editMenu->addAction(m_pasteAct);
    m_editMenu->addAction(m_duplicateAct);
    m_editMenu->addAction(m_deleteAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_selectAllAct);
    m_editMenu->addAction(m_deselectAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_addNoteAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_preferencesAct);
    updateEditMenu();
    connect(m_editMenu, SIGNAL(aboutToShow()), this, SLOT(updateEditMenu()));

    m_partMenu = menuBar()->addMenu(tr("&Part"));
    connect(m_partMenu, SIGNAL(aboutToShow()), this, SLOT(updatePartMenu()));

    m_partMenu->addAction(m_createNewPart);
	m_partMenu->addAction(m_loadBundledPart);
	m_partMenu->addSeparator();
	m_partMenu->addAction(m_openInPartsEditorAct);
	m_partMenu->addAction(m_saveBundledPart);
	m_partMenu->addSeparator();
	m_partMenu->addAction(m_rotate90cwAct);
	m_partMenu->addAction(m_rotate180Act);
	m_partMenu->addAction(m_rotate90ccwAct);
	m_partMenu->addAction(m_flipHorizontalAct);
	m_partMenu->addAction(m_flipVerticalAct);
	m_partMenu->addSeparator();
	m_zOrderMenu = m_partMenu->addMenu(tr("Raise and Lower"));
	m_partMenu->addSeparator();
	m_partMenu->addMenu(m_addToBinMenu);
	m_partMenu->addAction(m_showPartLabelAct);

#ifndef QT_NO_DEBUG
	m_partMenu->addSeparator();
	m_partMenu->addAction(m_groupAct);
#endif

	m_zOrderMenu->addAction(m_bringToFrontAct);
	m_zOrderMenu->addAction(m_bringForwardAct);
	m_zOrderMenu->addAction(m_sendBackwardAct);
	m_zOrderMenu->addAction(m_sendToBackAct);

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_zoomInAct);
    m_viewMenu->addAction(m_zoomOutAct);
    m_viewMenu->addAction(m_fitInWindowAct);
    m_viewMenu->addAction(m_actualSizeAct);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_showBreadboardAct);
    m_viewMenu->addAction(m_showSchematicAct);
    m_viewMenu->addAction(m_showPCBAct);
    m_viewMenu->addSeparator();
    connect(m_viewMenu, SIGNAL(aboutToShow()), this, SLOT(updateLayerMenu()));
    m_numFixedActionsInViewMenu = m_viewMenu->actions().size();

    m_windowMenu = menuBar()->addMenu(tr("&Window"));
	m_windowMenu->addAction(m_minimizeAct);
	m_windowMenu->addSeparator();
	//m_windowMenu->addAction(m_toggleToolbarAct);
	updateWindowMenu();
	connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

	m_pcbTraceMenu = menuBar()->addMenu(tr("&Trace"));
	m_pcbTraceMenu->addAction(m_autorouteAct);
	m_pcbTraceMenu->addAction(m_createTraceAct);
	m_pcbTraceMenu->addAction(m_createJumperAct);
	m_pcbTraceMenu->addAction(m_excludeFromAutorouteAct);
	m_pcbTraceMenu->addAction(m_selectAllTracesAct);
	m_pcbTraceMenu->addAction(m_selectAllExcludedTracesAct);
	m_pcbTraceMenu->addAction(m_selectAllJumpersAct);

#ifndef QT_NO_DEBUG
	m_pcbTraceMenu->addAction(m_groundFillAct);
#endif

	m_schematicTraceMenu = menuBar()->addMenu(tr("&Diagram"));
	m_schematicTraceMenu->addAction(m_autorouteAct);
	m_schematicTraceMenu->addAction(m_createTraceAct);
	m_schematicTraceMenu->addAction(m_excludeFromAutorouteAct);
	m_schematicTraceMenu->addAction(m_selectAllTracesAct);
	m_schematicTraceMenu->addAction(m_selectAllExcludedTracesAct);
	m_schematicTraceMenu->addAction(m_selectAllJumpersAct);

#ifndef QT_NO_DEBUG
	m_schematicTraceMenu->addAction(m_tidyWiresAct);
#endif

	updateTraceMenu();
	connect(m_pcbTraceMenu, SIGNAL(aboutToShow()), this, SLOT(updateTraceMenu()));
	connect(m_schematicTraceMenu, SIGNAL(aboutToShow()), this, SLOT(updateTraceMenu()));


    menuBar()->addSeparator();

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_showInViewHelpAct);
    m_helpMenu->addAction(m_openHelpAct);
    m_helpMenu->addAction(m_examplesAct);
    m_helpMenu->addAction(m_partsRefAct);
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_checkForUpdatesAct);
	m_helpMenu->addAction(m_importFilesFromPrevInstallAct);
	m_helpMenu->addAction(m_reportBugAct);
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_aboutAct);
#ifndef QT_NO_DEBUG
	m_helpMenu->addAction(m_aboutQtAct);
#endif
}

void MainWindow::updateLayerMenu() {
	removeActionsStartingAt(m_viewMenu, m_numFixedActionsInViewMenu);
    m_viewMenu->addAction(m_showAllLayersAct);
    m_viewMenu->addAction(m_hideAllLayersAct);

	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->updateLayerMenu(m_viewMenu, m_showAllLayersAct, m_hideAllLayersAct );
}

void MainWindow::updateWireMenu() {
	if (m_currentGraphicsView == m_breadboardGraphicsView) return;

	QList<QGraphicsItem *> items = m_currentGraphicsView->scene()->selectedItems();
	m_addBendpointAct->setEnabled(false);
	if (items.count() == 1) {
		enableAddBendpointAct(items[0]);
	}
	Wire * wire = NULL;
	bool enableAll = false;
	bool deleteOK = false;
	bool createTraceOK = false;
	bool createJumperOK = false;
	bool excludeOK = false;
	bool enableZOK = true;
	if (items.count() == 1) {
		enableAll = true;
		wire = dynamic_cast<Wire *>(items[0]);
		if (wire != NULL) {
			if (wire->getRatsnest()) {
				QList<ConnectorItem *> ends;
				Wire * jt = wire->findJumperOrTraced(ViewGeometry::JumperFlag | ViewGeometry::TraceFlag, ends);
				createJumperOK = (jt == NULL) || (!jt->getJumper());
				createTraceOK = (jt == NULL) || (!jt->getTrace());
			}
			else if (wire->getJumper()) {
				deleteOK = true;
				createTraceOK = true;
				excludeOK = true;
				m_excludeFromAutorouteAct->setChecked(!wire->getAutoroutable());
			}
			else if (wire->getTrace()) {
				deleteOK = true;
				createJumperOK = true;
				excludeOK = true;
				m_excludeFromAutorouteAct->setChecked(!wire->getAutoroutable());
			}
		}
	}

	m_bringToFrontAct->setEnabled(enableZOK);
	m_bringForwardAct->setEnabled(enableZOK);
	m_sendBackwardAct->setEnabled(enableZOK);
	m_sendToBackAct->setEnabled(enableZOK);
	m_createTraceAct->setEnabled(enableAll && createTraceOK);
	m_createJumperAct->setEnabled(enableAll && createJumperOK);
	m_deleteAct->setEnabled(enableAll && deleteOK);
	m_excludeFromAutorouteAct->setEnabled(enableAll && excludeOK);

}

void MainWindow::updatePartMenu() {
	if (m_currentGraphicsView == NULL) return;

	ItemCount itemCount = m_currentGraphicsView->calcItemCount();

	bool enable = true;

	m_groupAct->setEnabled(itemCount.selCount > 1);

	if (itemCount.selCount <= 0) {
		enable = false;
	}
	else {
		if (itemCount.itemsCount == itemCount.selCount) {
			// if all items are selected
			// z-reordering is a no-op
			enable = false;
		}
	}

	//DebugDialog::debug(QString("enable layer actions %1").arg(enable));
	m_bringToFrontAct->setEnabled(enable);
	m_bringForwardAct->setEnabled(enable);
	m_sendBackwardAct->setEnabled(enable);
	m_sendToBackAct->setEnabled(enable);

	m_showPartLabelAct->setEnabled((itemCount.selCount > 0)  && (itemCount.selCount > itemCount.noteCount));
	m_showPartLabelAct->setChecked(itemCount.labelCount == itemCount.selCount);

	enable = (itemCount.selRotatable > 0);

	//DebugDialog::debug(QString("enable rotate (2) %1").arg(enable));
	m_rotate90cwAct->setEnabled(enable);
	m_rotate180Act->setEnabled(enable);
	m_rotate90ccwAct->setEnabled(enable);

	m_flipHorizontalAct->setEnabled((itemCount.selHFlipable > 0) && (m_currentGraphicsView != m_pcbGraphicsView));
	m_flipVerticalAct->setEnabled((itemCount.selVFlipable > 0) && (m_currentGraphicsView != m_pcbGraphicsView));

	updateItemMenu();
	updateEditMenu();

	m_addBendpointAct->setEnabled(false);
	if (itemCount.selCount == 1) {
		enableAddBendpointAct(m_currentGraphicsView->scene()->selectedItems()[0]);
	}
}

void MainWindow::updateTransformationActions() {
	if (m_currentGraphicsView == NULL) return;

	ItemCount itemCount = m_currentGraphicsView->calcItemCount();
	bool enable = (itemCount.selRotatable > 0);

	//DebugDialog::debug(QString("enable rotate (1) %1").arg(enable));
	m_rotate90cwAct->setEnabled(enable);
	m_rotate180Act->setEnabled(enable);
	m_rotate90ccwAct->setEnabled(enable);
	foreach(SketchToolButton* rotateButton, m_rotateButtons) {
		rotateButton->setEnabled(enable);
	}

	m_flipHorizontalAct->setEnabled((itemCount.selHFlipable > 0) && (m_currentGraphicsView != m_pcbGraphicsView));
	m_flipVerticalAct->setEnabled((itemCount.selVFlipable > 0) && (m_currentGraphicsView != m_pcbGraphicsView));

	enable = m_flipHorizontalAct->isEnabled() || m_flipVerticalAct->isEnabled();
	foreach(SketchToolButton* flipButton, m_flipButtons) {
		flipButton->setEnabled(enable);
	}
}

void MainWindow::updateItemMenu() {
	if (m_currentGraphicsView == NULL) return;

	QList<QGraphicsItem *> items = m_currentGraphicsView->scene()->selectedItems();

	if (m_currentGraphicsView == m_pcbGraphicsView) {
		bool enabled = true;
		int count = 0;
		foreach (QGraphicsItem * item, items) {
			VirtualWire * vw = dynamic_cast<VirtualWire *>(item);
			if (vw == NULL) {
				enabled = false;
				break;
			}

			if (!vw->getRatsnest()) {
				enabled = false;
				break;
			}

			count++;
		}

		// TODO: if there's already a trace or jumper, disable appropriately
		m_createTraceAct->setEnabled(enabled && count > 0);
		m_createJumperAct->setEnabled(enabled && count > 0);
	}

	int selCount = 0;
	ItemBase * itemBase = NULL;
	foreach(QGraphicsItem * item, items) {
		ItemBase * ib = ItemBase::extractTopLevelItemBase(item);
		if (ib == NULL) continue;

		selCount++;
		if (selCount == 1) itemBase = ib;
		else if (selCount > 1) break;
	}

	PaletteItem *selected = dynamic_cast<PaletteItem *>(itemBase);
	bool enabled = (selCount == 1) && (selected != NULL);
	m_addToBinMenu->setEnabled(enabled);
	m_addToBinMenu->clear();
	if(enabled) {
		QList<QAction*> acts = m_paletteWidget->openedBinsActions(selectedModuleID());
		m_addToBinMenu->addActions(acts);
	}
	m_saveBundledPart->setEnabled(enabled && !selected->modelPart()->isCore());

	//TODO PARTS EDITOR REMOVE
	//m_openInOldPartsEditorAct->setEnabled(enabled);
	// can't open wire in parts editor
	enabled &= selected != NULL && selected->itemType() == ModelPart::Part;
	m_openInPartsEditorAct->setEnabled(enabled);

}

void MainWindow::updateEditMenu() {
	QClipboard *clipboard = QApplication::clipboard();
	m_pasteAct->setEnabled(false);
	if (clipboard != NULL) {
		const QMimeData *mimeData = clipboard->mimeData(QClipboard::Clipboard);
		if (mimeData != NULL) {
			if (mimeData->hasFormat("application/x-dnditemsdata")) {
				m_pasteAct->setEnabled(true);
				//DebugDialog::debug(QString("paste enabled: true"));
			}
		}
	}

	if (m_currentGraphicsView != NULL) {
		const QList<QGraphicsItem *> items =  m_currentGraphicsView->scene()->selectedItems();
		bool copyActsEnabled = false;
		bool deleteActsEnabled = false;
		foreach (QGraphicsItem * item, items) {
			if (m_currentGraphicsView->canDeleteItem(item)) {
				deleteActsEnabled = true;
			}
			if (m_currentGraphicsView->canCopyItem(item)) {
				copyActsEnabled = true;
			}
		}

		//DebugDialog::debug(QString("enable cut/copy/duplicate/delete %1 %2 %3").arg(copyActsEnabled).arg(deleteActsEnabled).arg(m_currentWidget->viewIdentifier()) );
		m_deleteAct->setEnabled(deleteActsEnabled);
		m_cutAct->setEnabled(deleteActsEnabled && copyActsEnabled);
		m_copyAct->setEnabled(copyActsEnabled);
		m_duplicateAct->setEnabled(copyActsEnabled);
	}
}

void MainWindow::updateTraceMenu() {
	bool rEnabled = false;
	bool jEnabled = false;
	bool tEnabled = false;
	bool ctEnabled = false;
	bool cjEnabled = false;
	bool exEnabled = false;
	bool exChecked = true;
	bool twEnabled = false;
	bool gfEnabled = false;

	if (m_currentGraphicsView != NULL) {
		if (m_currentGraphicsView != this->m_breadboardGraphicsView) {
			QList<QGraphicsItem *> items = m_currentGraphicsView->scene()->items();
			foreach (QGraphicsItem * item, items) {
				Wire * wire = dynamic_cast<Wire *>(item);
				if (wire == NULL) {
					if (m_currentGraphicsView != m_pcbGraphicsView) continue;
					if (gfEnabled) continue;

					ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
					if (itemBase == NULL) continue;

					if (itemBase->itemType() == ModelPart::Board ||  itemBase->itemType() == ModelPart::ResizableBoard) {
						gfEnabled = true;
					}

					continue;
				}

				if (wire->getRatsnest()) {
					rEnabled = true;
					if (wire->isSelected()) {
						ctEnabled = true;
						cjEnabled = true;
					}
				}
				else if (wire->getJumper()) {
					jEnabled = true;
					if (wire->isSelected()) {
						ctEnabled = true;
						exEnabled = true;
						if (wire->getAutoroutable()) {
							exChecked = false;
						}
					}
				}
				else if (wire->getTrace()) {
					tEnabled = true;
					twEnabled = true;
					if (wire->isSelected()) {
						cjEnabled = true;
						exEnabled = true;
						if (wire->getAutoroutable()) {
							exChecked = false;
						}
					}
				}
			}
		}
	}

	m_createTraceAct->setEnabled(ctEnabled);
	m_createJumperAct->setEnabled(cjEnabled && (m_currentGraphicsView == m_pcbGraphicsView));
	m_excludeFromAutorouteAct->setEnabled(exEnabled);
	m_excludeFromAutorouteAct->setChecked(exChecked);
	m_autorouteAct->setEnabled(rEnabled);
	m_exportEtchableAct->setEnabled(true);
	m_exportEtchableSvgAct->setEnabled(true);
	m_selectAllTracesAct->setEnabled(tEnabled);
	m_selectAllExcludedTracesAct->setEnabled(tEnabled);
	m_selectAllJumpersAct->setEnabled(jEnabled);
	m_tidyWiresAct->setEnabled(twEnabled);
	m_groundFillAct->setEnabled(gfEnabled);

}


void MainWindow::group() {
	if (m_currentGraphicsView == NULL) return;

	ModelPart * mp = m_pcbGraphicsView->group(NULL);
	m_breadboardGraphicsView->group(mp);
	m_schematicGraphicsView->group(mp);
}

void MainWindow::zoomIn() {
	// To zoom throw the combobox options
	zoomIn(1);

	/*if (m_currentWidget == NULL) return;
	m_currentWidget->relativeZoom(ZoomComboBox::ZoomStep);*/
}

void MainWindow::zoomIn(int steps) {
	for(int i=0; i < steps; i++) {
		currentSketchArea()->zoomComboBox()->zoomIn();
	}
}

void MainWindow::zoomOut() {
	// To zoom throw the combobox options
	zoomOut(1);

	/*if (m_currentWidget == NULL) return;
	m_currentWidget->relativeZoom(-ZoomComboBox::ZoomStep);*/
}

void MainWindow::zoomOut(int steps) {
	for(int i=0; i < steps; i++) {
		currentSketchArea()->zoomComboBox()->zoomOut();
	}
}

void MainWindow::fitInWindow() {
	if (m_currentGraphicsView == NULL) return;
	m_currentGraphicsView->fitInWindow();
}

void MainWindow::actualSize() {
	if (m_currentGraphicsView == NULL) return;
	m_currentGraphicsView->absoluteZoom(100);
}

void MainWindow::showBreadboardView() {
	this->m_tabWidget->setCurrentIndex(0);
}

void MainWindow::showSchematicView() {
	this->m_tabWidget->setCurrentIndex(1);

}

void MainWindow::showPCBView() {
	this->m_tabWidget->setCurrentIndex(2);
}

void MainWindow::openHelp() {
	QDesktopServices::openUrl(QString("http://new.fritzing.org/learning"));
}

void MainWindow::openExamples() {
	QDesktopServices::openUrl(QString("http://new.fritzing.org/projects"));
}

void MainWindow::openPartsReference() {
	QDesktopServices::openUrl(QString("http://new.fritzing.org/parts"));
}

void MainWindow::visitFritzingDotOrg() {
	 QDesktopServices::openUrl(QString("http://www.fritzing.org"));
}

void MainWindow::reportBug() {
	 QDesktopServices::openUrl(QString("http://code.google.com/p/fritzing/issues"));
}

void MainWindow::createNewPartInOldEditor() {
	openOldPartsEditor(NULL);
}

void MainWindow::createNewPart() {
	openPartsEditor(NULL);
}

void MainWindow::openOldPartsEditor(PaletteItem *paletteItem){
	Q_UNUSED(paletteItem);
	/*static long nextId = -1;
	ModelPart *modelPart = NULL;
	long id = nextId--;

	if(paletteItem != NULL) {
		modelPart = paletteItem->modelPart();
		id = paletteItem->id();
	}

	MainPartsEditorWindow * mainPartsEditorWindow = new MainPartsEditorWindow(id,this,0,modelPart,modelPart!=NULL);
	connect(mainPartsEditorWindow, SIGNAL(partUpdated(QString)), this, SLOT(loadPart(QString)));
	connect(mainPartsEditorWindow, SIGNAL(closed(long)), this, SLOT(partsEditorClosed(long)));

	mainPartsEditorWindow->show();
	mainPartsEditorWindow->raise();*/
}

// TODO PARTS EDITOR REMOVE
void MainWindow::openPartsEditor(PaletteItem * paletteItem) {
	ModelPart* modelPart = paletteItem? paletteItem->modelPart(): NULL;
	long id = paletteItem? paletteItem->id(): -1;
	QWidget *partsEditor = getPartsEditor(modelPart, id);
	partsEditor->show();
	partsEditor->raise();
}

PartsEditorMainWindow* MainWindow::getPartsEditor(ModelPart *modelPart, long _id, class PartsBinPaletteWidget* requester) {
	static long nextId = -1;
	long id = _id==-1? nextId--: _id;

	PartsEditorMainWindow *mainPartsEditorWindow = new PartsEditorMainWindow(id, this, modelPart, (modelPart!=NULL));

	connect(mainPartsEditorWindow, SIGNAL(partUpdated(const QString&, long)), this, SLOT(loadPart(const QString&, long)));
	connect(mainPartsEditorWindow, SIGNAL(closed(long)), this, SLOT(partsEditorClosed(long)));

	connect(this, SIGNAL(aboutToClose()), mainPartsEditorWindow, SLOT(parentAboutToClose()));
	connect(mainPartsEditorWindow, SIGNAL(changeActivationSignal(bool)), this, SLOT(changeActivation(bool)));

	m_partsEditorWindows.insert(id, mainPartsEditorWindow);
	if(requester) m_binsWithPartsEditorRequests.insert(id,requester);

	return mainPartsEditorWindow;
}

void MainWindow::partsEditorClosed(long id) {
	m_partsEditorWindows.remove(id);
	m_binsWithPartsEditorRequests.remove(id);
}

void MainWindow::openInOldPartsEditor() {
	// TODO: check to see if part is already open in a part editor window
	if (m_currentGraphicsView == NULL) return;
	PaletteItem *selectedPart = m_currentGraphicsView->getSelectedPart();

	openOldPartsEditor(selectedPart);
}

// TODO PARTS EDITOR REMOVE
void MainWindow::openInPartsEditor() {
	if (m_currentGraphicsView == NULL) return;

	PaletteItem *selectedPart = m_currentGraphicsView->getSelectedPart();
	PartsEditorMainWindow * window = m_partsEditorWindows.value(selectedPart->id());

	if(window != NULL) {
		window->raise();
	} else {
		openPartsEditor(selectedPart);
	}
}

void MainWindow::createNewSketch() {
    MainWindow* mw = newMainWindow(m_paletteModel, m_refModel, "new sketch", true);
    mw->move(x()+CascadeFactorX,y()+CascadeFactorY);
	QApplication::processEvents();

	mw->addBoard();
    mw->show();

    QSettings settings;
    settings.remove("lastOpenSketch");
    mw->clearFileProgressDialog();
}

void MainWindow::minimize() {
	this->showMinimized();
}

void MainWindow::toggleToolbar(bool toggle) {
	Q_UNUSED(toggle);
	/*if(toggle) {
		this->m_fileToolBar->show();
		this->m_editToolBar->show();
	} else {
		this->m_fileToolBar->hide();
		this->m_editToolBar->hide();
	}*/
}

void MainWindow::togglePartLibrary(bool toggle) {
	if(toggle) {
		m_paletteWidget->show();
	} else {
		m_paletteWidget->hide();
	}
}

void MainWindow::toggleInfo(bool toggle) {
	if(toggle) {
		((QDockWidget*)m_infoView->parent())->show();
	} else {
		((QDockWidget*)m_infoView->parent())->hide();
	}
}

void MainWindow::toggleNavigator(bool toggle) {
	if(toggle) {
		((QDockWidget*)m_miniViewContainerBreadboard->parent())->show();
		((QDockWidget*)m_miniViewContainerSchematic->parent())->show();
		((QDockWidget*)m_miniViewContainerPCB->parent())->show();
	} else {
		((QDockWidget*)m_miniViewContainerBreadboard->parent())->hide();
		((QDockWidget*)m_miniViewContainerSchematic->parent())->hide();
		((QDockWidget*)m_miniViewContainerPCB->parent())->hide();
	}
}

void MainWindow::toggleUndoHistory(bool toggle) {
	if(toggle) {
		((QDockWidget*)m_undoView->parent())->show();
	} else {
		((QDockWidget*)m_undoView->parent())->hide();
	}
}

void MainWindow::toggleDebuggerOutput(bool toggle) {
	if (toggle) {
		DebugDialog::showDebug();
	} else {
		DebugDialog::hideDebug();
	}
}

void MainWindow::updateWindowMenu() {
	m_toggleDebuggerOutputAct->setChecked(DebugDialog::visible());
	foreach (QWidget * widget, QApplication::topLevelWidgets()) {
		MainWindow * mainWindow = qobject_cast<MainWindow *>(widget);
		if (mainWindow == NULL) continue;

		QAction *action = mainWindow->raiseWindowAction();
		action->setChecked(action == m_raiseWindowAct);
		m_windowMenu->addAction(action);
	}
}

void MainWindow::pageSetup() {
	notYetImplemented(tr("Page Setup"));
}

void MainWindow::notYetImplemented(QString action) {
	QMessageBox::warning(this, tr("Fritzing"),
				tr("Sorry, \"%1\" has not been implemented yet").arg(action));
}


void MainWindow::rotate90cw() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->rotateX(90);
}

void MainWindow::rotate90ccw() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->rotateX(-90);
}

void MainWindow::rotate180() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->rotateX(180);
}

void MainWindow::flipHorizontal() {
	m_currentGraphicsView->flip(Qt::Horizontal);
}

void MainWindow::flipVertical() {
	m_currentGraphicsView->flip(Qt::Vertical);
}

void MainWindow::sendToBack() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->sendToBack();
}

void MainWindow::sendBackward() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->sendBackward();
}

void MainWindow::bringForward() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->bringForward();
}

void MainWindow::bringToFront() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->bringToFront();
}

void MainWindow::showAllLayers() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->setAllLayersVisible(true);
}

void MainWindow::hideAllLayers() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->setAllLayersVisible(false);
}

void MainWindow::openRecentOrExampleFile() {
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		if (alreadyOpen(action->data().toString())) {
			return;
		}

		MainWindow* mw = newMainWindow(m_paletteModel, m_refModel, action->data().toString(), true);
		bool readOnly = m_openExampleActions.contains(action->text());
		mw->setReadOnly(readOnly);
		mw->load(action->data().toString(),!readOnly,!readOnly);
		mw->clearFileProgressDialog();
		closeIfEmptySketch(mw);
	}
}

void MainWindow::removeActionsStartingAt(QMenu * menu, int start) {
	QList<QAction*> actions = menu->actions();

	if(start == 0) {
		menu->clear();
	} else {
		for(int i=start; i < actions.size(); i++) {
			menu->removeAction(actions.at(i));
		}
	}
}

//TODO: this whole thing should probably be cleaned up and moved to another file
void MainWindow::exportToGerber() {

    //NOTE: this assumes just one board per sketch
    //TODO: should deal with crazy multi-board setups someday...

    // grab the list of parts
    ItemBase * board = NULL;
    foreach (QGraphicsItem * childItem, m_pcbGraphicsView->items()) {
        board = dynamic_cast<ItemBase *>(childItem);
        if (board == NULL) continue;

        //for now take the first board you find
        if (board->itemType() == ModelPart::ResizableBoard || board->itemType() == ModelPart::Board) {
            break;
        }
        board = NULL;
    }

    // barf an error if there's no board
    if (!board) {
        QMessageBox::critical(this, tr("Fritzing"),
                   tr("Your sketch does not have a board yet!  Please add a PCB in order to export to Gerber."));
        return;
    }

    QString exportDir = QFileDialog::getExistingDirectory(this, tr("Choose a folder for exporting"),
                                             defaultSaveFolder(),
                                             QFileDialog::ShowDirsOnly
                                             | QFileDialog::DontResolveSymlinks);

	QList<ViewLayer::ViewLayerID> viewLayerIDs;
	viewLayerIDs << ViewLayer::Copper0 << ViewLayer::Copper0Trace;
	QSizeF imageSize;
    QString svg = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, board);
	if (svg.isEmpty()) {
		// tell the user something reasonable
		return;
	}

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		// tell the user something reasonable
		return;
	}

    // create copper0 gerber from svg
    SVG2gerber copper0Gerber(svg, "copper0");

    QString copper0File = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_copperTop.gtl";
    QFile copper0Out(copper0File);
    if (!copper0Out.open(QIODevice::WriteOnly | QIODevice::Text))
        DebugDialog::debug("gerber export: cannot open output file");

    QTextStream copperStream(&copper0Out);
    copperStream << copper0Gerber.getGerber();

    // soldermask
    QString soldermaskFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_maskTop.gts";
    QFile maskOut(soldermaskFile);
    if (!maskOut.open(QIODevice::WriteOnly | QIODevice::Text))
        DebugDialog::debug("gerber export: cannot open output file");

    QTextStream maskStream(&maskOut);
    maskStream << copper0Gerber.getSolderMask();

    // drill file
    QString drillFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_drill.txt";
    QFile drillOut(drillFile);
    if (!drillOut.open(QIODevice::WriteOnly | QIODevice::Text))
        DebugDialog::debug("gerber export: cannot open output file");

    QTextStream drillStream(&drillOut);
    drillStream << copper0Gerber.getNCDrill();

    // now do it for silk
    QList<ViewLayer::ViewLayerID> silkLayerIDs;
    silkLayerIDs << ViewLayer::Silkscreen;
    QString svgSilk = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, silkLayerIDs, true, imageSize, board);
    if (svgSilk.isEmpty()) {
        // tell the user something reasonable
        return;
    }

	/*
	// for debugging silkscreen svg
    QFile silkout("silk.svg");
	if (silkout.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream silkStream(&silkout);
		silkStream << svgSilk;
		silkout.close();
	}
	*/

    result = domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn);
    if (!result) {
            // tell the user something reasonable
            return;
    }

    // create copper0 gerber from svg
    SVG2gerber silk0Gerber(svgSilk, "silk");

    QString silk0File = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_silkTop.gto";
    QFile silk0Out(silk0File);
    if (!silk0Out.open(QIODevice::WriteOnly | QIODevice::Text))
        DebugDialog::debug("gerber export: cannot open output file");

    QTextStream silkStream(&silk0Out);
    silkStream << silk0Gerber.getGerber();
}

void MainWindow::exportToEagle() {

	QString text =
		tr("This will soon provide an export of your Fritzing sketch to the EAGLE layout "
		"software. If you'd like to have more exports to your favourite EDA tool, please let "
		"us know, or contribute.");
/*
	QString text =
		tr("The Eagle export module is very experimental.  If anything breaks or behaves "
		"strangely, please let us know.");
*/

	QMessageBox::information(this, tr("Fritzing"), text);

	Fritzing2Eagle eagle = Fritzing2Eagle(m_pcbGraphicsView);

	/*
	QList <ItemBase*> partList;

	// bail out if something is wrong
	// TODO: show an error in QMessageBox
    if(m_currentWidget == NULL) {
		return;
	}

    m_currentGraphicsView->collectParts(partList);

	QString exportInfoString = tr("parts include:\n");
	QString exportString = tr("GRID INCH 0.005\n");


	for(int i=0; i < partList.size(); i++){
		QString label = partList.at(i)->instanceTitle();
		QString desc = partList.at(i)->modelPartShared()->title();

		QHash<QString,QString> properties = partList.at(i)->modelPartShared()->properties();
		QString package = properties["package"];
		if (package == NULL) {
			package = tr("*** package not specified ***");
		}

		exportInfoString += label + tr(" which is a ") + desc + tr(" in a ") + package + tr(" package.\n");
	}
	QMessageBox::information(this, tr("Fritzing"), exportInfoString);
	*/

	/*
	QFile fp( fileName );
	fp.open(QIODevice::WriteOnly);
	fp.write(bom.toUtf8(),bom.length());
	fp.close();
	*/


	/*
	GRID INCH 0.005
	USE '/Applications/eclipse/eagle/lbr/fritzing.lbr';
	ADD RESISTOR@fritzing 'R_1' R0.000 (2.3055117 2.1307087);
	ADD LED@fritzing 'L_2' R0.000 (5.423622 2.425197);
	GRID LAST;
	*/
}

void MainWindow::exportSvg() {
	QString path = defaultSaveFolder();
	QString fileExt;
	QString fileName = FApplication::getSaveFileName(this,
		tr("Export SVG..."),
		path+"/"+QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)+svgActionType,
		fileExtFormats[svgActionType],
		&fileExt
		);

	if (fileName.isEmpty()) return;

	FileProgressDialog * fileProgressDialog = exportProgress();
	QList<ViewLayer::ViewLayerID> viewLayerIDs;
	foreach (ViewLayer * viewLayer, m_currentGraphicsView->viewLayers()) {
		viewLayerIDs << viewLayer->viewLayerID();
	}

	QSizeF imageSize;
    QString svg = m_currentGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, false, imageSize, NULL);
	if (svg.isEmpty()) {
		// tell the user something reasonable
		return;
	}

	QFile file(fileName);
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);
	out << svg;
	file.close();
	delete fileProgressDialog;

}

void MainWindow::exportBOM() {
    QList <ItemBase*> partList;
    QMap<QString, int> shoppingList;

    int maxLabelWidth = 0;

    QString bom = tr("Fritzing Bill of Materials\n\n");

    bom += tr("Sketch: \t") + QFileInfo(m_fileName).fileName() + "\n";
    //TODO:  add the Author ID - system specific crap
    //bom += tr("Printed by: ") +

    bom += tr("Date: \t") + QDateTime::currentDateTime().toString() + "\n\n";

    // bail out if something is wrong
    // TODO: show an error in QMessageBox
    if(m_currentWidget == NULL) {
        return;
    }

    m_currentGraphicsView->collectParts(partList);

    qSort(partList.begin(), partList.end(), sortPartList);

    for(int i=0; i < partList.size(); i++){
        QString label = partList.at(i)->instanceTitle();
        QString desc = partList.at(i)->modelPartShared()->title();
        if(label.length() > maxLabelWidth) {
            maxLabelWidth = label.length();
        }
        if(!shoppingList.contains(desc)){
            shoppingList.insert(desc, 1);
        }
        else {
            shoppingList[desc]++;
        }
    }

    for(int i=0; i < partList.size(); i++){
        QString spacer = "   ";
        QString label = partList.at(i)->instanceTitle();

        spacer += QString(maxLabelWidth - label.length(), QChar(' '));
        bom += label + spacer +
               partList.at(i)->modelPartShared()->title() + "\n";
    }

    bom += tr("\n\nShopping List\n\nQuantity\tPart\n\n");
    QMapIterator<QString, int> it(shoppingList);
    while (it.hasNext()) {
        it.next();
        bom += QString::number(it.value()) + "\t\t" + it.key() + "\n";
    }

    QString path = defaultSaveFolder();

    QString fileExt;
    QString extFmt = fileExtFormats.value(bomActionType);
    QString fname = path+"/"+QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)+".bom"+getExtFromFileDialog(extFmt);
    DebugDialog::debug(QString("fname %1\n%2").arg(fname).arg(extFmt));

    QString fileName = FApplication::getSaveFileName(this,
            tr("Export Bill of Materials (BoM)..."),
            fname,
            extFmt,
            &fileExt
    );

    if (fileName.isEmpty()) {
            return; //Cancel pressed
    }

	FileProgressDialog * fileProgressDialog = exportProgress();
    DebugDialog::debug(fileExt+" selected to export");
    //fileExt = getExtFromFileDialog(fileExt);
    //#ifdef Q_WS_X11
//                if(!alreadyHasExtension(fileName)) {
//                        fileName += fileExt;
//                }
//        #endif

    QFile fp( fileName );
    fp.open(QIODevice::WriteOnly);
    fp.write(bom.toUtf8(),bom.length());
    fp.close();

	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard != NULL) {
		clipboard->setText(bom);
	}
	delete fileProgressDialog;
}

void MainWindow::hideShowTraceMenu() {
	m_pcbTraceMenu->menuAction()->setVisible(m_currentGraphicsView == m_pcbGraphicsView);
	m_schematicTraceMenu->menuAction()->setVisible(m_currentGraphicsView == m_schematicGraphicsView);
}

void MainWindow::createTraceMenuActions() {
	m_autorouteAct = new QAction(tr("&Autoroute"), this);
	m_autorouteAct->setStatusTip(tr("Autoroute..."));
	connect(m_autorouteAct, SIGNAL(triggered()), this, SLOT(autoroute()));

	m_createTraceAct = new QAction(tr("&Create Trace from Selected Wire(s)"), this);
	m_createTraceAct->setStatusTip(tr("Create a trace from the selected wire"));
	connect(m_createTraceAct, SIGNAL(triggered()), this, SLOT(createTrace()));

	m_createJumperAct = new QAction(tr("&Create Jumper from Selected Wire(s)"), this);
	m_createJumperAct->setStatusTip(tr("Create a jumper wire from the selected wire"));
	connect(m_createJumperAct, SIGNAL(triggered()), this, SLOT(createJumper()));

	m_excludeFromAutorouteAct = new QAction(tr("&Don't Autoroute This Trace"), this);
	m_excludeFromAutorouteAct->setStatusTip(tr("When autorouting, do not rip up this wire"));
	connect(m_excludeFromAutorouteAct, SIGNAL(triggered()), this, SLOT(excludeFromAutoroute()));
	m_excludeFromAutorouteAct->setCheckable(true);

	m_selectAllTracesAct = new QAction(tr("Select All Traces"), this);
	m_selectAllTracesAct->setStatusTip(tr("Select all trace wires"));
	connect(m_selectAllTracesAct, SIGNAL(triggered()), this, SLOT(selectAllTraces()));

	m_selectAllExcludedTracesAct = new QAction(tr("Select All Traces Marked \"Don't Autoroute\""), this);
	m_selectAllExcludedTracesAct->setStatusTip(tr("Select all trace wires excluded from autorouting"));
	connect(m_selectAllExcludedTracesAct, SIGNAL(triggered()), this, SLOT(selectAllExcludedTraces()));

	m_selectAllJumpersAct = new QAction(tr("Select All Jumper Wires"), this);
	m_selectAllJumpersAct->setStatusTip(tr("Select all jumper wires"));
	connect(m_selectAllJumpersAct, SIGNAL(triggered()), this, SLOT(selectAllJumpers()));

	m_tidyWiresAct = new QAction(tr("Tidy Wires"), this);
	m_tidyWiresAct->setStatusTip(tr("Tidy selected wires"));
	connect(m_tidyWiresAct, SIGNAL(triggered()), this, SLOT(tidyWires()));

	m_groundFillAct = new QAction(tr("Ground Fill"), this);
	m_groundFillAct->setStatusTip(tr("Fill up the ground plane"));
	connect(m_groundFillAct, SIGNAL(triggered()), this, SLOT(groundFill()));
}

void MainWindow::autoroute() {
	PCBSketchWidget * pcbSketchWidget = qobject_cast<PCBSketchWidget *>(m_currentGraphicsView);
	if (pcbSketchWidget == NULL) return;

	dynamic_cast<SketchAreaWidget *>(pcbSketchWidget->parent())->routingStatusLabel()->setText(tr("Autorouting..."));

	AutorouteProgressDialog progress(this);
	progress.setModal(true);
	progress.show();

	pcbSketchWidget->scene()->clearSelection();
	pcbSketchWidget->setIgnoreSelectionChangeEvents(true);
	Autorouter1 * autorouter1 = new Autorouter1(pcbSketchWidget);

	connect(&progress, SIGNAL(cancel()), autorouter1, SLOT(cancel()), Qt::DirectConnection);
	connect(&progress, SIGNAL(skip()), autorouter1, SLOT(cancelTrace()), Qt::DirectConnection);
	connect(&progress, SIGNAL(stop()), autorouter1, SLOT(stopTrace()), Qt::DirectConnection);
	connect(autorouter1, SIGNAL(setMaximumProgress(int)), &progress, SLOT(setMaximum(int)), Qt::DirectConnection);
	connect(autorouter1, SIGNAL(setProgressValue(int)), &progress, SLOT(setValue(int)), Qt::DirectConnection);
	QApplication::processEvents();

	autorouter1->start();
	pcbSketchWidget->setIgnoreSelectionChangeEvents(false);
	delete autorouter1;
}

void MainWindow::createTrace() {
	m_currentGraphicsView->createTrace();
}

void MainWindow::createJumper() {
	m_pcbGraphicsView->createJumper();
}

void MainWindow::excludeFromAutoroute() {
	m_pcbGraphicsView->excludeFromAutoroute(m_excludeFromAutorouteAct->isChecked());
}

void MainWindow::selectAllTraces() {
	m_currentGraphicsView->selectAllWires(ViewGeometry::TraceFlag);
}

void MainWindow::selectAllExcludedTraces() {
	m_pcbGraphicsView->selectAllExcludedTraces();
}

void MainWindow::selectAllJumpers() {
	m_currentGraphicsView->selectAllWires(ViewGeometry::JumperFlag);
}

void MainWindow::notClosableForAWhile() {
	m_dontClose = true;

	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	connect(timer,SIGNAL(timeout()),this,SLOT(ensureClosable()));
	timer->start(500);
}

void MainWindow::ensureClosable() {
	m_dontClose = false;
}

void MainWindow::showPartLabels() {
	if (m_currentGraphicsView == NULL) return;

	m_currentGraphicsView->showPartLabels(m_showPartLabelAct->isChecked());
}

void MainWindow::addNote() {
	if (m_currentGraphicsView == NULL) return;

	ViewGeometry vg;
	vg.setRect(0, 0, Note::initialMinWidth, Note::initialMinHeight);
	QPointF tl = m_currentGraphicsView->mapToScene(QPoint(0, 0));
	QSizeF vpSize = m_currentGraphicsView->viewport()->size();
	tl.setX(tl.x() + ((vpSize.width() - Note::initialMinWidth) / 2.0));
	tl.setY(tl.y() + ((vpSize.height() - Note::initialMinHeight) / 2.0));
	vg.setLoc(tl);

	QUndoCommand * parentCommand = new QUndoCommand(tr("Add Note"));
	m_currentGraphicsView->stackSelectionState(false, parentCommand);
	m_currentGraphicsView->scene()->clearSelection();
	new AddItemCommand(m_currentGraphicsView, BaseCommand::SingleView, ItemBase::noteModuleIDName, vg, ItemBase::getNextID(), false, -1, -1, parentCommand);
	m_undoStack->push(parentCommand);
}

bool MainWindow::alreadyOpen(const QString & fileName) {
    foreach (QWidget * widget, QApplication::topLevelWidgets()) {
        MainWindow * mainWindow = qobject_cast<MainWindow *>(widget);
        if (mainWindow == NULL) continue;

		// don't load two copies of the same file
		if (mainWindow->fileName().compare(fileName) == 0) {
			mainWindow->raise();
			return true;
		}
    }

	return false;
}

void MainWindow::enableAddBendpointAct(QGraphicsItem * graphicsItem) {
	// assumes act is disabled already

	Wire * wire = dynamic_cast<Wire *>(graphicsItem);
	if (wire == NULL) return;
	if (wire->getRatsnest()) return;

	BendpointAction * bendpointAction = qobject_cast<BendpointAction *>(m_addBendpointAct);
	FGraphicsScene * scene = qobject_cast<FGraphicsScene *>(graphicsItem->scene());
	if (scene != NULL) {
		bendpointAction->setLastLocation(scene->lastContextMenuPos());
	}

	bool enabled = false;
	if (m_currentGraphicsView->lastHoverEnterConnectorItem()) {
		bendpointAction->setText(tr("Remove Bendpoint"));
		bendpointAction->setLastHoverEnterConnectorItem(m_currentGraphicsView->lastHoverEnterConnectorItem());
		bendpointAction->setLastHoverEnterItem(NULL);
		enabled = true;
	}
	else if (m_currentGraphicsView->lastHoverEnterItem()) {
		bendpointAction->setText(tr("Add Bendpoint"));
		bendpointAction->setLastHoverEnterItem(m_currentGraphicsView->lastHoverEnterItem());
		bendpointAction->setLastHoverEnterConnectorItem(NULL);
		enabled = true;
	}
	else {
		bendpointAction->setLastHoverEnterItem(NULL);
		bendpointAction->setLastHoverEnterConnectorItem(NULL);
	}

	m_addBendpointAct->setEnabled(enabled);
}

void MainWindow::addBendpoint()
{
	BendpointAction * bendpointAction = qobject_cast<BendpointAction *>(m_addBendpointAct);

	m_currentGraphicsView->addBendpoint(bendpointAction->lastHoverEnterItem(),
										bendpointAction->lastHoverEnterConnectorItem(),
										bendpointAction->lastLocation());
}

FileProgressDialog * MainWindow::exportProgress() {
	return (new FileProgressDialog("Exporting...", 0, this));
}

void MainWindow::importFilesFromPrevInstall() {
	QString prevInstallPath = QFileDialog::getExistingDirectory(
			this,
			tr("Please choose the previous Fritzing folder..."),
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(prevInstallPath.isNull()) return;
	if(!QFileInfo(prevInstallPath+"/parts").exists()) {
		QMessageBox::critical(
			this, QObject::tr("Fritzing"),
			tr("The folder \"%1\" isn't a Friting installation folder").arg(prevInstallPath));
		return;
	}

	QString userDataPath = getUserDataStorePath();

	// replicate dirs
	QStringList foldersToCopy = getUserDataStoreFolders();
	foreach(QString folder, foldersToCopy) {
		replicateDir(prevInstallPath+folder, userDataPath+folder);
	}

	// cleanup old bins
	QDir dataStoreBins(userDataPath);
	dataStoreBins.cd("bins");
	QStringList binsToRemove;
	binsToRemove
		<< "allParts.fzb" << "artreenoBin.fzb"
		<< "E6SetBin.fzb" << "pin_headers.fzb";
	foreach(QString binToRemove, binsToRemove) {
		dataStoreBins.remove(binToRemove);
	}

	// make sure to add the old my_parts.fzp to the folder
	QString myPartsBinRelPath = "/bins/my_parts.fzb";
	QFile myOldPartsBinFile(prevInstallPath+myPartsBinRelPath);
	if(myOldPartsBinFile.exists()) {
		QDateTime now = QDateTime::currentDateTime();
		QString newNamePostfix = QString("__imported_on__%1.fzb").arg(now.toString("yyyy-MM-dd_hh-mm-ss"));
		myOldPartsBinFile.copy(userDataPath+myPartsBinRelPath.replace(".fzb",newNamePostfix));
	}

	QMessageBox::information(
		this, QObject::tr("Fritzing"),
		tr("You will have to restart Fritzing in order to use the imported parts"));
}

void MainWindow::tidyWires() {
	m_currentGraphicsView->tidyWires();
}

#define MINYSECTION 4

void MainWindow::groundFill()
{
	
	ItemBase * board = NULL;
    foreach (QGraphicsItem * childItem, m_pcbGraphicsView->items()) {
        board = dynamic_cast<ItemBase *>(childItem);
        if (board == NULL) continue;

        //for now take the first board you find
        if (board->itemType() == ModelPart::ResizableBoard || board->itemType() == ModelPart::Board) {
            break;
        }
        board = NULL;
    }

    // barf an error if there's no board
    if (!board) {
        QMessageBox::critical(this, tr("Fritzing"),
                   tr("Your sketch does not have a board yet!  Please add a PCB in order to fill the ground plane."));
        return;
    }

	//FileProgressDialog * fileProgressDialog = exportProgress();
	QList<ViewLayer::ViewLayerID> viewLayerIDs;
	viewLayerIDs << ViewLayer::Copper0 << ViewLayer::Copper0Trace;
	QSizeF imageSize;
    QString svg = m_currentGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, board);
	if (svg.isEmpty()) {
		// tell the user something reasonable
		return;
	}

	QByteArray byteArray;
	if (!SvgFileSplitter::changeStrokeWidth(svg, 5, byteArray)) {
		QMessageBox::warning(this, tr("Fritzing"), tr("Unable to create ground fill"));
		return;
	}

	QFile file("testGroundFill.svg");
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);
	out << byteArray;
	file.close();

	int res = 1000 / 10;
	qreal iWidth = res * imageSize.width() / FSvgRenderer::printerScale();
	qreal iHeight = res * imageSize.height() / FSvgRenderer::printerScale();
	QSvgRenderer renderer(byteArray);
	QImage image(iWidth, iHeight, QImage::Format_Indexed8);
	//image.setColor(0, Qt::black);
	//for (int i = 0; i < 256; i++) {
		//image.setColor(i, Qt::white);
	//}

	QPainter painter;
	painter.begin(&image);
	renderer.render(&painter);
	painter.end();

	image.save("testGroundFill.png");

	qreal useWidth = qMin(iWidth, res * board->boundingRect().width() / FSvgRenderer::printerScale());
	qreal useHeight = qMin(iHeight, res * board->boundingRect().height() / FSvgRenderer::printerScale());

	QList<QRect> rects;
	// TODO deal with irregular board outline
	for (int y = 0; y < useHeight; y++) {
		bool inWhite = true;
		int whiteStart = 0;
		uchar * scanLine = image.scanLine(y);
		for (int x = 0; x < useWidth; x++) {
			uchar current = *(scanLine + x);
			if (inWhite) {
				if (current != 0) {
					// another white pixel, keep moving
					continue;
				}

				// got black: close up this segment;
				inWhite = false;
				if (x - whiteStart < MINYSECTION) {
					// not a big enough section
					continue;
				}

				rects.append(QRect(whiteStart, y, x - whiteStart + 1, 1));
			}
			else {
				if (current == 0) {
					// another black pixel, keep moving
					continue;
				}

				inWhite = true;
				whiteStart = x;
			}
		}
		if (inWhite) {
			// close up the last segment
			if (floor(useWidth) - whiteStart + 1 >= MINYSECTION) {
				rects.append(QRect(whiteStart, y, floor(useWidth) - whiteStart + 1, 1));
			}
		}
	}




	// render it back as svgs by figuring out the beginning and end of each separate horizontal line
	//		skip lines that are too small
	//		designate each line as a connector, and attach each line that intersects to a bus
	// call the whole thing a part
	// stick it on its own layer
}
