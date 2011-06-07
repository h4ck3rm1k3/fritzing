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
#include "utils/folderutils.h"


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

	FolderUtils::setOpenSaveFolder(exportDir);
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

	LayerList viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Bottom);
	int copperInvalidCount = doCopper(board, viewLayerIDs, "Copper0", "_copperBottom.gbl", "_maskBottom.gbs", true, exportDir, displayMessageBoxes);

	if (m_pcbGraphicsView->boardLayers() == 2) {
		viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Top);
		copperInvalidCount += doCopper(board, viewLayerIDs, "Copper1", "_copperTop.gtl", "_maskTop.gts", false, exportDir, displayMessageBoxes);
	}

    LayerList silkLayerIDs;
    silkLayerIDs << ViewLayer::Silkscreen1  << ViewLayer::Silkscreen1Label;
	int silkInvalidCount = doSilk(silkLayerIDs, "Silk1", "_silkTop.gto", board, exportDir, displayMessageBoxes);
    silkLayerIDs.clear();
    silkLayerIDs << ViewLayer::Silkscreen0  << ViewLayer::Silkscreen0Label;
	silkInvalidCount += doSilk(silkLayerIDs, "Silk0", "_silkBottom.gbo", board, exportDir, displayMessageBoxes);

    // now do it for the outline/contour
    LayerList outlineLayerIDs;
    outlineLayerIDs << ViewLayer::Board;
	QSizeF imageSize;
	bool empty;
	QString svgOutline = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), outlineLayerIDs, outlineLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgOutline.isEmpty()) {
        displayMessage(tr("outline is empty"), displayMessageBoxes);
        return;
    }

	QXmlStreamReader streamReader(svgOutline);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

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
	int outlineInvalidCount = outlineGerber.convert(svgOutline, m_pcbGraphicsView->boardLayers() == 2, "contour", "Mask", true, svgSize * GraphicsUtils::StandardFritzingDPI);
	if (outlineInvalidCount > 0) {
		outlineInvalidCount = 0;
		svgOutline = clipToBoard(svgOutline, board, "board");
		outlineInvalidCount = outlineGerber.convert(svgOutline, m_pcbGraphicsView->boardLayers() == 2, "contour", "Mask", true, svgSize * GraphicsUtils::StandardFritzingDPI);
	}

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
    contourStream << outlineGerber.getGerber();

	if (outlineInvalidCount > 0 || silkInvalidCount > 0 || copperInvalidCount > 0) {
		QString s;
		if (outlineInvalidCount > 0) s += tr("the board outline layer, ");
		if (silkInvalidCount > 0) s += tr("silkscreen layer(s), ");
		if (copperInvalidCount > 0) s += tr("copper layer(s), ");
		s.chop(2);
		displayMessage(tr("Unable to translate svg curves in ").arg(s), displayMessageBoxes);
	}

}

int MainWindow::doCopper(ItemBase * board, LayerList & viewLayerIDs, const QString & copperName, const QString & copperSuffix, const QString & solderMaskSuffix, bool doDrill, const QString & exportDir, bool displayMessageBoxes) 
{
	QSizeF imageSize;
	bool empty;
	QString svg = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
	if (svg.isEmpty()) {
		displayMessage(tr("%1 file export failure (1)").arg(copperName), displayMessageBoxes);
		return 0;
	}

	QXmlStreamReader streamReader(svg);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svg = clipToBoard(svg, board, copperName);
	if (svg.isEmpty()) {
		displayMessage(tr("%1 file export failure (3)").arg(copperName), displayMessageBoxes);
		return 0;
	}

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		displayMessage(tr("%1 file export failure (2)").arg(copperName), displayMessageBoxes);
		return 0;
	}

    // create copper gerber from svg
    SVG2gerber copperGerber;
	QString maskName = QString("Mask%1").arg(copperName[copperName.length() - 1]);
	int copperInvalidCount = copperGerber.convert(svg, m_pcbGraphicsView->boardLayers() == 2, copperName, maskName, false, svgSize * GraphicsUtils::StandardFritzingDPI);

    QString copperFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + copperSuffix;
    QFile copperOut(copperFile);
	if (!copperOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(tr("%1 file export failure (3)").arg(copperName), displayMessageBoxes);
		return 0;
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
		return 0;
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
			return 0;
		}

		QTextStream drillStream(&drillOut);
		drillStream << copperGerber.getNCDrill();
		drillStream.flush();
		drillOut.close();
	}

	return copperInvalidCount;
}

int MainWindow::doSilk(LayerList silkLayerIDs, const QString & silkName, const QString & gerberSuffix, ItemBase * board, const QString & exportDir, bool displayMessageBoxes ) 
{
	QSizeF imageSize;
	bool empty;
	QString svgSilk = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, silkLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgSilk.isEmpty()) {
		displayMessage(tr("silk file export failure (1)"), displayMessageBoxes);
        return 0;
    }

	if (empty) {
		// don't bother with file
		return 0;
	}

	QXmlStreamReader streamReader(svgSilk);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svgSilk = clipToBoard(svgSilk, board, silkName);
	if (svgSilk.isEmpty()) {
		displayMessage(tr("silk export failure"), displayMessageBoxes);
		return 0;
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
	int silkInvalidCount = silkGerber.convert(svgSilk, m_pcbGraphicsView->boardLayers() == 2, silkName, "Mask", false, svgSize * GraphicsUtils::StandardFritzingDPI);

    QString silkFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + gerberSuffix;
    QFile silkOut(silkFile);
	if (!silkOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(tr("silk file export failure (2)"), displayMessageBoxes);
		return 0;
	}

    QTextStream silkStream(&silkOut);
    silkStream << silkGerber.getGerber();
	silkStream.flush();
	silkOut.close();

	return silkInvalidCount;
}

void MainWindow::displayMessage(const QString & message, bool displayMessageBoxes) {
	// don't use QMessageBox if running conversion as a service
	if (displayMessageBoxes) {
		QMessageBox::warning(this, tr("Fritzing"), message);
		return;
	}

	DebugDialog::debug(message);
}

QString MainWindow::clipToBoard(QString svgString, ItemBase * board, const QString & layerName) {
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
		image.fill(0xffffffff);
		image.setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
		image.setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);
		QRectF target(0, 0, twidth, theight);

		QSvgRenderer renderer(svgByteArray);
		QPainter painter;
		painter.begin(&image);
		renderer.render(&painter, target);
		painter.end();
		image.invertPixels();				// need white pixels on a black background for GroundPlaneGenerator
		//image.save("output.png");

		GroundPlaneGenerator gpg;
		gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#ffffff", layerName, false, 1);
		
		QDomDocument doc;
		TextUtils::mergeSvg(doc, svgString, "");
		foreach (QString gsvg, gpg.newSVGs()) {
			TextUtils::mergeSvg(doc, gsvg, "");
		}
		svgString = TextUtils::mergeSvgFinish(doc);
	}

	return svgString;
}
