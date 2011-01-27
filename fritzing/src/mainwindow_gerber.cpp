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

#include <QMessageBox>
#include <QFileDialog>
#include <QWebPage>
#include <QWebFrame>
#include <QSvgRenderer>

#include "mainwindow.h"
#include "debugdialog.h"
#include "fsvgrenderer.h"
#include "svg/svg2gerber.h"
#include "sketch/pcbsketchwidget.h"
#include "svg/svgfilesplitter.h"
#include "svg/groundplanegenerator.h"
#include "utils/graphicsutils.h"
#include "utils/textutils.h"


static QRegExp AaCc("[aAcC]");

////////////////////////////////////////////

void MainWindow::exportToGerber() {

    //NOTE: this assumes just one board per sketch

    // grab the list of parts
    ItemBase * board = m_pcbGraphicsView->findBoard();
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

	if (exportDir.isEmpty()) return;

	exportToGerber(exportDir, board, true);
}

void MainWindow::exportToGerber(const QString & exportDir, ItemBase * board, bool displayMessageBoxes) 
{
	if (board == NULL) {
		board = m_pcbGraphicsView->findBoard();
	}
	if (board == NULL) {
		DebugDialog::debug("board not found");
		return;
	}

	// TODO:  need copper1 (.gbl) mask (.gbs) 

	LayerList viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Bottom);
	doCopper(board, viewLayerIDs, "Copper0", "_copperTop.gtl", "_maskTop.gts", true, exportDir, displayMessageBoxes);

	if (m_pcbGraphicsView->boardLayers() == 2) {
		viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Top);
		doCopper(board, viewLayerIDs, "Copper1", "_copperBottom.gbl", "_maskBottom.gbs", false, exportDir, displayMessageBoxes);
	}

    LayerList silkLayerIDs;
    silkLayerIDs << ViewLayer::Silkscreen1  << ViewLayer::Silkscreen1Label;
	doSilk(silkLayerIDs, "Silk1", "_silkBottom.gbo", board, exportDir, displayMessageBoxes);
    silkLayerIDs.clear();
    silkLayerIDs << ViewLayer::Silkscreen0  << ViewLayer::Silkscreen0Label;
	doSilk(silkLayerIDs, "Silk0", "_silkTop.gto", board, exportDir, displayMessageBoxes);

    // now do it for the outline/contour
    LayerList outlineLayerIDs;
    outlineLayerIDs << ViewLayer::Board;
	QSizeF imageSize;
	bool empty;
	QString svgOutline = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), outlineLayerIDs, outlineLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgOutline.isEmpty()) {
        // tell the user something reasonable
        return;
    }

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
    bool result = domDocument.setContent(svgOutline, &errorStr, &errorLine, &errorColumn);
    if (!result) {
		displayMessage(tr("outline file export failure (1)"), displayMessageBoxes);
        return;
    }

    // create copper0 gerber from svg
    SVG2gerber outlineGerber;
	outlineGerber.convert(svgOutline, m_pcbGraphicsView->boardLayers() == 2, "contour", "Mask");

    // contour / board outline
    QString contourFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_contour.gm1";
    QFile contourOut(contourFile);
	if (!contourOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(tr("outline file export failure (2)"), displayMessageBoxes);
		return;
	}

    QTextStream contourStream(&contourOut);
    contourStream << outlineGerber.getContour();
}

void MainWindow::doCopper(ItemBase * board, LayerList & viewLayerIDs, const QString & copperName, const QString & copperSuffix, const QString & solderMaskSuffix, bool doDrill, const QString & exportDir, bool displayMessageBoxes) 
{
	QSizeF imageSize;
	bool empty;
	QString svg = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
	if (svg.isEmpty()) {
		displayMessage(tr("%1 file export failure (1)").arg(copperName), displayMessageBoxes);
		return;
	}

	svg = clipToBoard(svg, board);
	if (svg.isEmpty()) {
		displayMessage(tr("%1 file export failure (3)").arg(copperName), displayMessageBoxes);
		return;
	}

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		displayMessage(tr("%1 file export failure (2)").arg(copperName), displayMessageBoxes);
		return;
	}

    // create copper gerber from svg
    SVG2gerber copperGerber;
	QString maskName = QString("Mask%1").arg(copperName[copperName.length() - 1]);
	copperGerber.convert(svg, m_pcbGraphicsView->boardLayers() == 2, copperName, maskName);

    QString copperFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + copperSuffix;
    QFile copperOut(copperFile);
	if (!copperOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(tr("%1 file export failure (3)").arg(copperName), displayMessageBoxes);
		return;
	}

    QTextStream copperStream(&copperOut);
    copperStream << copperGerber.getGerber();
	copperStream.flush();
	copperOut.close();

    // soldermask
    QString soldermaskFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + solderMaskSuffix;
    QFile maskOut(soldermaskFile);
	if (!maskOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(tr("%1 mask file export failure (4)").arg(copperName), displayMessageBoxes);
		return;
	}

    QTextStream maskStream(&maskOut);
    maskStream << copperGerber.getSolderMask();
	maskStream.flush();
	maskOut.close();

	if (doDrill) {
		// drill file
		QString drillFile = exportDir + "/" +
							  QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
							  + "_drill.txt";
		QFile drillOut(drillFile);
		if (!drillOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
			displayMessage(tr("%1 drill file export failure (5)").arg(copperName), displayMessageBoxes);
			return;
		}

		QTextStream drillStream(&drillOut);
		drillStream << copperGerber.getNCDrill();
		drillStream.flush();
		drillOut.close();
	}
}

void MainWindow::doSilk(LayerList silkLayerIDs, const QString & silkName, const QString & gerberSuffix, ItemBase * board, const QString & exportDir, bool displayMessageBoxes ) 
{
	QSizeF imageSize;
	bool empty;
	QString svgSilk = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, silkLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgSilk.isEmpty()) {
		displayMessage(tr("silk file export failure (1)"), displayMessageBoxes);
        return;
    }

	if (empty) {
		// don't bother with file
		return;
	}

	svgSilk = clipToBoard(svgSilk, board);
	if (svgSilk.isEmpty()) {
		displayMessage(tr("silk export failure"), displayMessageBoxes);
		return;
	}

#ifndef QT_NO_DEBUG
	// for debugging silkscreen svg
    //QFile silkout(QDir::temp().absoluteFilePath(gerberSuffix + ".svg"));
	//if (silkout.open(QIODevice::WriteOnly | QIODevice::Text)) {
		//QTextStream silkStream(&silkout);
		//silkStream << svgSilk;
		//silkout.close();
	//}
#endif

    // create silk gerber from svg
    SVG2gerber silkGerber;
	silkGerber.convert(svgSilk, m_pcbGraphicsView->boardLayers() == 2, silkName, "Mask");

    QString silkFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + gerberSuffix;
    QFile silkOut(silkFile);
	if (!silkOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(tr("silk file export failure (2)"), displayMessageBoxes);
		return;
	}

    QTextStream silkStream(&silkOut);
    silkStream << silkGerber.getGerber();
	silkStream.flush();
	silkOut.close();
}

void MainWindow::displayMessage(const QString & message, bool displayMessageBoxes) {
	// don't use QMessageBox if running conversion as a service
	if (displayMessageBoxes) {
		QMessageBox::warning(this, tr("Fritzing"), message);
		return;
	}

	DebugDialog::debug(message);
}

QString MainWindow::clipToBoard(QString svgString, ItemBase * board) {
	QDomDocument domDocument1;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument1.setContent(svgString, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		return "";
	}

	QDomDocument domDocument2;
	domDocument2.setContent(svgString, &errorStr, &errorLine, &errorColumn);

	bool anyConverted = false;
    if (TextUtils::squashElement(domDocument1, "text", "", QRegExp())) {
        anyConverted = true; 
	}

	// gerber can't handle ellipses that are rotated, so cull them all
    if (TextUtils::squashElement(domDocument1, "ellipse", "", QRegExp())) {
		anyConverted = true;
    }

	// gerber can't handle paths with curves
    if (TextUtils::squashElement(domDocument1, "path", "d", AaCc)) {
		anyConverted = true;
    }

	QVector <QDomElement> leaves1;
	int transformCount1 = 0;
        QDomElement e1 = domDocument1.documentElement();
        TextUtils::collectLeaves(e1, transformCount1, leaves1);

	QVector <QDomElement> leaves2;
	int transformCount2 = 0;
        QDomElement e2 = domDocument2.documentElement();
        TextUtils::collectLeaves(e2, transformCount2, leaves2);

	qreal res = GraphicsUtils::StandardFritzingDPI;
	QRectF source = board->boundingRect();
	int twidth = res * source.width() / FSvgRenderer::printerScale();
	int theight = res * source.height() / FSvgRenderer::printerScale();

	svgString = TextUtils::removeXMLEntities(domDocument1.toString());
	QXmlStreamReader reader(svgString);
	QSvgRenderer renderer(&reader);
	bool anyClipped = false;
	for (int i = 0; i < transformCount1; i++) {
		QString n = QString::number(i);
		QRectF bounds = renderer.boundsOnElement(n);
		QMatrix m = renderer.matrixForElement(n);
		QDomElement element = leaves1.at(i);
		QString ms = element.attribute("transform");
		if (!ms.isEmpty()) {
			m *= TextUtils::transformStringToMatrix(ms);
		}
		QRectF mBounds = m.mapRect(bounds);
		if (mBounds.left() < 0 || mBounds.top() < 0 || bounds.right() > twidth || bounds.bottom() > theight) {
			// element is outside of bounds, squash it so it will be clipped
			element.setTagName("g");
			anyClipped = anyConverted = true;
		}	
	}

	if (anyClipped) {
		svgString = TextUtils::removeXMLEntities(domDocument1.toString());
	}

    if (anyConverted) {
		for (int i = 0; i < transformCount1; i++) {
			QDomElement element1 = leaves1.at(i);
			if (element1.tagName() != "g") {
				QDomElement element2 = leaves2.at(i);
				element2.setTagName("g");
			}
		}
		
		QString svg = TextUtils::removeXMLEntities(domDocument2.toString());

		QSize imgSize(twidth, theight);

		// expand the svg to fill the space of the image
		QRegExp widthFinder("width=[^i]+in.");
		int ix = widthFinder.indexIn(svg);
		if (ix >= 0) {
			svg.replace(ix, widthFinder.cap(0).length(), QString("width=\"%1px\"").arg(twidth));
		}
		QRegExp heightFinder("height=[^i]+in.");
		ix = heightFinder.indexIn(svg);
		if (ix > 0) {
			svg.replace(ix, heightFinder.cap(0).length(), QString("height=\"%1px\"").arg(theight));
		}

		QStringList exceptions;
		exceptions << "none" << "";
		QString toColor("#000000");
		QByteArray svgByteArray;
		SvgFileSplitter::changeColors(svg, toColor, exceptions, svgByteArray);

		QImage image(imgSize, QImage::Format_RGB32);
		image.fill(0);
		image.setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
		image.setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);
		QRectF target(0, 0, twidth, theight);

		//QString simple = "<html>"
			//"<body><font color='#ff0000'>hello world</font>"
			//"</body></html>";

		QWebPage webpage;
		webpage.setViewportSize(image.size());
		webpage.mainFrame()->setContent(svgByteArray, "image/svg+xml");
		//webpage.mainFrame()->setContent(simple.toUtf8());
		QPainter painter;
		painter.begin(&image);
		webpage.mainFrame()->render(&painter);
		painter.end();
		image.invertPixels();				// need white pixels on a black background for GroundPlaneGenerator
		//image.save("output.png");

		GroundPlaneGenerator gpg;
		gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#ffffff", "silkscreen", false, 1);
		
		QDomDocument doc;
		TextUtils::mergeSvg(doc, svgString, "");
		foreach (QString gsvg, gpg.newSVGs()) {
			TextUtils::mergeSvg(doc, gsvg, "");
		}
		svgString = TextUtils::mergeSvgFinish(doc);
	}

	return svgString;
}
