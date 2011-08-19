/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 5396 $:
$Author: cohen@irascible.com $:
$Date: 2011-08-15 01:36:25 +0200 (Mon, 15 Aug 2011) $

********************************************************************/

#include <QMessageBox>
#include <QFileDialog>
#include <QSvgRenderer>

#include "gerbergenerator.h"
#include "../debugdialog.h"
#include "../fsvgrenderer.h"
#include "../sketch/pcbsketchwidget.h"
#include "svgfilesplitter.h"
#include "groundplanegenerator.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../utils/folderutils.h"
#include "../items/logoitem.h"

static QRegExp AaCc("[aAcCqQtTsS]");

////////////////////////////////////////////

void GerberGenerator::exportToGerber(const QString & filename, const QString & exportDir, ItemBase * board, PCBSketchWidget * sketchWidget, bool displayMessageBoxes) 
{
	if (board == NULL) {
		board = sketchWidget->findBoard();
	}
	if (board == NULL) {
		DebugDialog::debug("board not found");
		return;
	}

	LayerList viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Bottom);
	int copperInvalidCount = doCopper(board, sketchWidget, viewLayerIDs, "Copper0", "_copperBottom.gbl", filename, exportDir, displayMessageBoxes);

	if (sketchWidget->boardLayers() == 2) {
		viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Top);
		copperInvalidCount += doCopper(board, sketchWidget, viewLayerIDs, "Copper1", "_copperTop.gtl", filename, exportDir, displayMessageBoxes);
	}

	LayerList maskLayerIDs = ViewLayer::maskLayers(ViewLayer::Bottom);
	int maskInvalidCount = doMask(maskLayerIDs, "Mask0", "_maskBottom.gbs", board, sketchWidget, filename, exportDir, displayMessageBoxes);

	if (sketchWidget->boardLayers() == 2) {
		maskLayerIDs = ViewLayer::maskLayers(ViewLayer::Top);
		maskInvalidCount += doMask(maskLayerIDs, "Mask1", "_maskTop.gts", board, sketchWidget, filename, exportDir, displayMessageBoxes);
	}

    LayerList silkLayerIDs;
    silkLayerIDs << ViewLayer::Silkscreen1  << ViewLayer::Silkscreen1Label;
	int silkInvalidCount = doSilk(silkLayerIDs, "Silk1", "_silkTop.gto", board, sketchWidget, filename, exportDir, displayMessageBoxes);
    silkLayerIDs.clear();
    silkLayerIDs << ViewLayer::Silkscreen0  << ViewLayer::Silkscreen0Label;
	silkInvalidCount += doSilk(silkLayerIDs, "Silk0", "_silkBottom.gbo", board, sketchWidget, filename, exportDir, displayMessageBoxes);

    // now do it for the outline/contour
    LayerList outlineLayerIDs;
    outlineLayerIDs << ViewLayer::Board;
	QSizeF imageSize;
	bool empty;
	QString svgOutline = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), outlineLayerIDs, outlineLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgOutline.isEmpty()) {
        displayMessage(QObject::tr("outline is empty"), displayMessageBoxes);
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
		displayMessage(QObject::tr("outline file export failure (1)"), displayMessageBoxes);
        return;
    }

    // create copper0 gerber from svg
    SVG2gerber outlineGerber;
	int outlineInvalidCount = outlineGerber.convert(svgOutline, sketchWidget->boardLayers() == 2, "contour", SVG2gerber::ForOutline, svgSize * GraphicsUtils::StandardFritzingDPI);
	if (outlineInvalidCount > 0) {
		outlineInvalidCount = 0;
		svgOutline = clipToBoard(svgOutline, board, "board", SVG2gerber::ForOutline);
		outlineInvalidCount = outlineGerber.convert(svgOutline, sketchWidget->boardLayers() == 2, "contour", SVG2gerber::ForOutline, svgSize * GraphicsUtils::StandardFritzingDPI);
	}

    // contour / board outline
    QString contourFile = exportDir + "/" +
                          QFileInfo(filename).fileName().remove(FritzingSketchExtension)
                          + "_contour.gm1";
    QFile contourOut(contourFile);
	if (!contourOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(QObject::tr("outline file export failure (2)"), displayMessageBoxes);
		return;
	}

    QTextStream contourStream(&contourOut);
    contourStream << outlineGerber.getGerber();

	doDrill(board, sketchWidget, filename, exportDir, displayMessageBoxes);

	if (outlineInvalidCount > 0 || silkInvalidCount > 0 || copperInvalidCount > 0 || maskInvalidCount) {
		QString s;
		if (outlineInvalidCount > 0) s += QObject::tr("the board outline layer, ");
		if (silkInvalidCount > 0) s += QObject::tr("silkscreen layer(s), ");
		if (copperInvalidCount > 0) s += QObject::tr("copper layer(s), ");
		if (maskInvalidCount > 0) s += QObject::tr("mask layer(s), ");
		s.chop(2);
		displayMessage(QObject::tr("Unable to translate svg curves in ").arg(s), displayMessageBoxes);
	}

}

int GerberGenerator::doCopper(ItemBase * board, PCBSketchWidget * sketchWidget, LayerList & viewLayerIDs, const QString & copperName, const QString & copperSuffix, const QString & filename, const QString & exportDir, bool displayMessageBoxes) 
{
	QSizeF imageSize;
	bool empty;
	QString svg = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
	if (svg.isEmpty()) {
		displayMessage(QObject::tr("%1 file export failure (1)").arg(copperName), displayMessageBoxes);
		return 0;
	}

	QXmlStreamReader streamReader(svg);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svg = clipToBoard(svg, board, copperName, SVG2gerber::ForCopper);
	if (svg.isEmpty()) {
		displayMessage(QObject::tr("%1 file export failure (3)").arg(copperName), displayMessageBoxes);
		return 0;
	}

    // create copper gerber from svg
    SVG2gerber copperGerber;
	int copperInvalidCount = copperGerber.convert(svg, sketchWidget->boardLayers() == 2, copperName, SVG2gerber::ForCopper, svgSize * GraphicsUtils::StandardFritzingDPI);

    QString copperFile = exportDir + "/" +
                          QFileInfo(filename).fileName().remove(FritzingSketchExtension)
                          + copperSuffix;
    QFile copperOut(copperFile);
	if (!copperOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(QObject::tr("%1 file export failure (3)").arg(copperName), displayMessageBoxes);
		return 0;
	}

    QTextStream copperStream(&copperOut);
    copperStream << copperGerber.getGerber();
	copperStream.flush();
	copperOut.close();

	return copperInvalidCount;
}


int GerberGenerator::doSilk(LayerList silkLayerIDs, const QString & silkName, const QString & gerberSuffix, ItemBase * board, PCBSketchWidget * sketchWidget, const QString & filename, const QString & exportDir, bool displayMessageBoxes ) 
{
	QSizeF imageSize;
	bool empty;
	QString svgSilk = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, silkLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgSilk.isEmpty()) {
		displayMessage(QObject::tr("silk file export failure (1)"), displayMessageBoxes);
        return 0;
    }

	if (empty) {
		// don't bother with file
		return 0;
	}

	//QFile f(silkName + "original.svg");
	//f.open(QFile::WriteOnly);
	//QTextStream fs(&f);
	//fs << svgSilk;
	//f.close();

	QXmlStreamReader streamReader(svgSilk);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svgSilk = clipToBoard(svgSilk, board, silkName, SVG2gerber::ForSilk);
	if (svgSilk.isEmpty()) {
		displayMessage(QObject::tr("silk export failure"), displayMessageBoxes);
		return 0;
	}

	//QFile f2(silkName + "clipped.svg");
	//f2.open(QFile::WriteOnly);
	//QTextStream fs2(&f2);
	//fs2 << svgSilk;
	//f2.close();


    // create silk gerber from svg
    SVG2gerber silkGerber;
	int silkInvalidCount = silkGerber.convert(svgSilk, sketchWidget->boardLayers() == 2, silkName, SVG2gerber::ForSilk, svgSize * GraphicsUtils::StandardFritzingDPI);

    QString silkFile = exportDir + "/" +
                          QFileInfo(filename).fileName().remove(FritzingSketchExtension)
                          + gerberSuffix;
    QFile silkOut(silkFile);
	if (!silkOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(QObject::tr("silk file export failure (2)"), displayMessageBoxes);
		return 0;
	}

    QTextStream silkStream(&silkOut);
    silkStream << silkGerber.getGerber();
	silkStream.flush();
	silkOut.close();

	return silkInvalidCount;
}


int GerberGenerator::doDrill(ItemBase * board, PCBSketchWidget * sketchWidget, const QString & filename, const QString & exportDir, bool displayMessageBoxes) 
{
    LayerList drillLayerIDs;
    drillLayerIDs << ViewLayer::Copper0;

	QSizeF imageSize;
	bool empty;
	QString svgDrill = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), drillLayerIDs, drillLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgDrill.isEmpty()) {
		displayMessage(QObject::tr("drill file export failure (1)"), displayMessageBoxes);
        return 0;
    }

	if (empty) {
		// don't bother with file
		return 0;
	}

	QXmlStreamReader streamReader(svgDrill);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svgDrill = clipToBoard(svgDrill, board, "Copper0", SVG2gerber::ForDrill);
	if (svgDrill.isEmpty()) {
		displayMessage(QObject::tr("drill export failure"), displayMessageBoxes);
		return 0;
	}

    // create silk gerber from svg
    SVG2gerber drillGerber;
	int drillInvalidCount = drillGerber.convert(svgDrill, sketchWidget->boardLayers() == 2, "drill", SVG2gerber::ForDrill, svgSize * GraphicsUtils::StandardFritzingDPI);


		// drill file
	QString drillFile = exportDir + "/" +
							  QFileInfo(filename).fileName().remove(FritzingSketchExtension)
							  + "_drill.txt";
	QFile drillOut(drillFile);
	if (!drillOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(QObject::tr("drill file export failure (5)"), displayMessageBoxes);
		return 0;
	}

	QTextStream drillStream(&drillOut);
	drillStream << drillGerber.getGerber();
	drillStream.flush();
	drillOut.close();

	return drillInvalidCount;
}

int GerberGenerator::doMask(LayerList maskLayerIDs, const QString &maskName, const QString & gerberSuffix, ItemBase * board, PCBSketchWidget * sketchWidget, const QString & filename, const QString & exportDir, bool displayMessageBoxes ) 
{
	// don't want these in the mask laqyer
	QList<ItemBase *> copperLogoItems;
	foreach (QGraphicsItem * item, sketchWidget->items()) {
		CopperLogoItem * logoItem = dynamic_cast<CopperLogoItem *>(item);
		if (logoItem && logoItem->isVisible()) {
			copperLogoItems.append(logoItem);
			logoItem->setVisible(false);
		}
	}

	QSizeF imageSize;
	bool empty;
	QString svgMask = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), maskLayerIDs, maskLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgMask.isEmpty()) {
		displayMessage(QObject::tr("mask file export failure (1)"), displayMessageBoxes);
        return 0;
    }

	foreach (ItemBase * logoItem, copperLogoItems) {
		logoItem->setVisible(true);
	}

	if (empty) {
		// don't bother with file
		return 0;
	}

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument.setContent(svgMask, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		displayMessage(QObject::tr("%1 file export failure (2)").arg(maskName), displayMessageBoxes);
		return 0;
	}

	QXmlStreamReader streamReader(svgMask);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svgMask = clipToBoard(svgMask, board, maskName, SVG2gerber::ForMask);
	if (svgMask.isEmpty()) {
		displayMessage(QObject::tr("mask export failure"), displayMessageBoxes);
		return 0;
	}

    // create mask gerber from svg
    SVG2gerber maskGerber;
	int maskInvalidCount = maskGerber.convert(svgMask, sketchWidget->boardLayers() == 2, maskName, SVG2gerber::ForMask, svgSize * GraphicsUtils::StandardFritzingDPI);

    QString maskFile = exportDir + "/" +
                          QFileInfo(filename).fileName().remove(FritzingSketchExtension)
                          + gerberSuffix;
    QFile maskOut(maskFile);
	if (!maskOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(QObject::tr("mask file export failure (2)"), displayMessageBoxes);
		return 0;
	}

    QTextStream maskStream(&maskOut);
    maskStream << maskGerber.getGerber();
	maskStream.flush();
	maskOut.close();

	return maskInvalidCount;
}

void GerberGenerator::displayMessage(const QString & message, bool displayMessageBoxes) {
	// don't use QMessageBox if running conversion as a service
	if (displayMessageBoxes) {
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), message);
		return;
	}

	DebugDialog::debug(message);
}

QString GerberGenerator::clipToBoard(QString svgString, ItemBase * board, const QString & layerName, SVG2gerber::ForWhy forWhy) {
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

    if (TextUtils::squashElement(domDocument1, "rect", "rx", QRegExp())) {
		anyConverted = true;
    }

    if (TextUtils::squashElement(domDocument1, "rect", "ry", QRegExp())) {
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

	double res = GraphicsUtils::StandardFritzingDPI;
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
		if (forWhy == SVG2gerber::ForOutline) {
			int tinyWidth = source.width() / FSvgRenderer::printerScale();
			int tinyHeight = source.height() / FSvgRenderer::printerScale();
			QRectF tinyTarget(0, 0, tinyWidth, tinyHeight);
			QImage tinyImage(tinyWidth, tinyHeight, QImage::Format_RGB32);
			QPainter painter;
			painter.begin(&tinyImage);
			renderer.render(&painter, tinyTarget);
			painter.end();
			tinyImage.invertPixels();				// need white pixels on a black background for GroundPlaneGenerator
			gpg.scanOutline(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#ffffff", layerName, false, 1, false, QSizeF(0, 0), 0);
		}
		else {
			gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#ffffff", layerName, false, 1, false, QSizeF(0, 0), 0);
		}

		if (gpg.newSVGs().count() > 0) {
			QDomDocument doc;
			TextUtils::mergeSvg(doc, svgString, "");
			foreach (QString gsvg, gpg.newSVGs()) {
				TextUtils::mergeSvg(doc, gsvg, "");
			}
			svgString = TextUtils::mergeSvgFinish(doc);
		}
	}

	return svgString;
}
