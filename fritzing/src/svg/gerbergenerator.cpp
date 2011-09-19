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

const QString GerberGenerator::SilkTopSuffix = "_silkTop.gto";
const QString GerberGenerator::SilkBottomSuffix = "_silkBottom.gbo";
const QString GerberGenerator::CopperTopSuffix = "_copperTop.gtl";
const QString GerberGenerator::CopperBottomSuffix = "_copperBottom.gbl";
const QString GerberGenerator::MaskTopSuffix = "_maskTop.gts";
const QString GerberGenerator::MaskBottomSuffix = "_maskBottom.gbs";
const QString GerberGenerator::DrillSuffix = "_drill.txt";
const QString GerberGenerator::OutlineSuffix = "_contour.gm1";
const QString GerberGenerator::MagicBoardOutlineID = "boardoutline";


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
	int copperInvalidCount = doCopper(board, sketchWidget, viewLayerIDs, "Copper0", CopperBottomSuffix, filename, exportDir, displayMessageBoxes);

	if (sketchWidget->boardLayers() == 2) {
		viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Top);
		copperInvalidCount += doCopper(board, sketchWidget, viewLayerIDs, "Copper1", CopperTopSuffix, filename, exportDir, displayMessageBoxes);
	}

	LayerList maskLayerIDs = ViewLayer::maskLayers(ViewLayer::Bottom);
	int maskInvalidCount = doMask(maskLayerIDs, "Mask0", MaskBottomSuffix, board, sketchWidget, filename, exportDir, displayMessageBoxes);

	if (sketchWidget->boardLayers() == 2) {
		maskLayerIDs = ViewLayer::maskLayers(ViewLayer::Top);
		maskInvalidCount += doMask(maskLayerIDs, "Mask1", MaskTopSuffix, board, sketchWidget, filename, exportDir, displayMessageBoxes);
	}

    LayerList silkLayerIDs = ViewLayer::silkLayers(ViewLayer::Top);
	int silkInvalidCount = doSilk(silkLayerIDs, "Silk1", SilkTopSuffix, board, sketchWidget, filename, exportDir, displayMessageBoxes);
    silkLayerIDs = ViewLayer::silkLayers(ViewLayer::Bottom);
	silkInvalidCount += doSilk(silkLayerIDs, "Silk0", SilkBottomSuffix, board, sketchWidget, filename, exportDir, displayMessageBoxes);

    // now do it for the outline/contour
    LayerList outlineLayerIDs = ViewLayer::outlineLayers();
	QSizeF imageSize;
	bool empty;
	QString svgOutline = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), outlineLayerIDs, outlineLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
    if (svgOutline.isEmpty()) {
        displayMessage(QObject::tr("outline is empty"), displayMessageBoxes);
        return;
    }

	svgOutline = cleanOutline(svgOutline);
	svgOutline = clipToBoard(svgOutline, board, "board", SVG2gerber::ForOutline);

	QXmlStreamReader streamReader(svgOutline);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

    // create outline gerber from svg
    SVG2gerber outlineGerber;
	int outlineInvalidCount = outlineGerber.convert(svgOutline, sketchWidget->boardLayers() == 2, "contour", SVG2gerber::ForOutline, svgSize * GraphicsUtils::StandardFritzingDPI);
	
	DebugDialog::debug(QString("outline output: %1").arg(outlineGerber.getGerber()));
	saveEnd("contour", exportDir, filename, OutlineSuffix, displayMessageBoxes, outlineGerber);

	doDrill(board, sketchWidget, filename, exportDir, displayMessageBoxes);

	if (outlineInvalidCount > 0 || silkInvalidCount > 0 || copperInvalidCount > 0 || maskInvalidCount) {
		QString s;
		if (outlineInvalidCount > 0) s += QObject::tr("the board outline layer, ");
		if (silkInvalidCount > 0) s += QObject::tr("silkscreen layer(s), ");
		if (copperInvalidCount > 0) s += QObject::tr("copper layer(s), ");
		if (maskInvalidCount > 0) s += QObject::tr("mask layer(s), ");
		s.chop(2);
		displayMessage(QObject::tr("Unable to translate svg curves in %1").arg(s), displayMessageBoxes);
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

	return doEnd(svg, sketchWidget->boardLayers(), copperName, SVG2gerber::ForCopper, svgSize * GraphicsUtils::StandardFritzingDPI, exportDir, filename, copperSuffix, displayMessageBoxes);
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

	return doEnd(svgSilk, sketchWidget->boardLayers(), silkName, SVG2gerber::ForSilk, svgSize * GraphicsUtils::StandardFritzingDPI, exportDir, filename, gerberSuffix, displayMessageBoxes);
}


int GerberGenerator::doDrill(ItemBase * board, PCBSketchWidget * sketchWidget, const QString & filename, const QString & exportDir, bool displayMessageBoxes) 
{
    LayerList drillLayerIDs;
    drillLayerIDs << ViewLayer::drillLayers();

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

	return doEnd(svgDrill, sketchWidget->boardLayers(), "drill", SVG2gerber::ForDrill, svgSize * GraphicsUtils::StandardFritzingDPI, exportDir, filename, DrillSuffix, displayMessageBoxes);
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

	return doEnd(svgMask, sketchWidget->boardLayers(), maskName, SVG2gerber::ForMask, svgSize * GraphicsUtils::StandardFritzingDPI, exportDir, filename, gerberSuffix, displayMessageBoxes);
}

int GerberGenerator::doEnd(const QString & svg, int boardLayers, const QString & layerName, SVG2gerber::ForWhy forWhy, QSizeF svgSize, 
							const QString & exportDir, const QString & prefix, const QString & suffix, bool displayMessageBoxes)
{
    // create mask gerber from svg
    SVG2gerber gerber;
	int invalidCount = gerber.convert(svg, boardLayers == 2, layerName, forWhy, svgSize);

	saveEnd(layerName, exportDir, prefix, suffix, displayMessageBoxes, gerber);

	return invalidCount;
}

bool GerberGenerator::saveEnd(const QString & layerName, const QString & exportDir, const QString & prefix, const QString & suffix, bool displayMessageBoxes, SVG2gerber & gerber)
{
    QString outname = exportDir + "/" +  QFileInfo(prefix).fileName().remove(FritzingSketchExtension) + suffix;
    QFile out(outname);
	if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
		displayMessage(QObject::tr("%1 file export failure (2)").arg(layerName), displayMessageBoxes);
		return false;
	}

    QTextStream stream(&out);
    stream << gerber.getGerber();
	stream.flush();
	out.close();
	return true;

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
	QRectF source = board->sceneBoundingRect();
	source.moveTo(0, 0);
	return clipToBoard(svgString, source, layerName, forWhy);
}

QString GerberGenerator::clipToBoard(QString svgString, QRectF & boardRect, const QString & layerName, SVG2gerber::ForWhy forWhy) {
	// document 1 will contain svg that is easy to convert to gerber
	QDomDocument domDocument1;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument1.setContent(svgString, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		return "";
	}

	QDomElement root = domDocument1.documentElement();
	if (root.firstChildElement().isNull()) {
		return "";
	}

	// document 2 will contain svg that must be rasterized for gerber conversion
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
	QRectF sourceRes(boardRect.left() * res / FSvgRenderer::printerScale(), boardRect.top() * res / FSvgRenderer::printerScale(), 
					 boardRect.width() * res / FSvgRenderer::printerScale(), boardRect.height() * res / FSvgRenderer::printerScale());
	int twidth = sourceRes.width();
	int theight = sourceRes.height();

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
		if (mBounds.left() < sourceRes.left() - 0.1|| mBounds.top() < sourceRes.top() - 0.1 || mBounds.right() > sourceRes.right() + 0.1 || mBounds.bottom() > sourceRes.bottom() + 0.1) {
			// element is outside of bounds, squash it so it will be clipped
			element.setTagName("g");
			anyClipped = anyConverted = true;
		}	
	}

	if (forWhy == SVG2gerber::ForOutline) {
		// make one big polygon.  Assume there are no holes in the board

		if (anyConverted || leaves1.count() > 1) {
			anyClipped = true;
			foreach (QDomElement l, leaves1) {
				l.setTagName("g");
			}
		}
	}

	if (anyClipped) {
		// svg has been changed by clipping process so get the string again
		svgString = TextUtils::removeXMLEntities(domDocument1.toString());
	}

    if (anyConverted) {
		for (int i = 0; i < transformCount1; i++) {
			QDomElement element1 = leaves1.at(i);
			if (element1.tagName() != "g") {
				// document 1 element svg can be directly converted to gerber
				// so remove it from document 2
				QDomElement element2 = leaves2.at(i);
				element2.setTagName("g");
			}
		}
		
		QSize imgSize(twidth, theight);

		// expand the svg to fill the space of the image
		QDomElement root = domDocument2.documentElement();
		root.setAttribute("width", QString("%1px").arg(twidth));
		root.setAttribute("height", QString("%1px").arg(theight));
		if (boardRect.x() != 0 || boardRect.y() != 0) {
			QString viewBox = root.attribute("viewBox");
			QStringList coords = viewBox.split(" ", QString::SkipEmptyParts);
			coords[0] = QString::number(sourceRes.left());
			coords[1] = QString::number(sourceRes.top());
			root.setAttribute("viewBox", coords.join(" "));
		}

		QString svg = TextUtils::removeXMLEntities(domDocument2.toString());

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

#ifndef QT_NO_DEBUG
		image.save("output.png");
#endif

		GroundPlaneGenerator gpg;
		if (forWhy == SVG2gerber::ForOutline) {
		//	int tinyWidth = boardRect.width();
		//	int tinyHeight = boardRect.height();
		//	QRectF tinyTarget(0, 0, tinyWidth, tinyHeight);
		//	QImage tinyImage(tinyWidth, tinyHeight, QImage::Format_RGB32);
		//	QPainter painter;
		//	painter.begin(&tinyImage);
		//	renderer.render(&painter, tinyTarget);
		//	painter.end();
		//	tinyImage.invertPixels();				// need white pixels on a black background for GroundPlaneGenerator
			gpg.scanOutline(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#000000", layerName, false, 1, false, QSizeF(0, 0), 0);
		}
		else {
			gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#000000", layerName, false, 1, false, QSizeF(0, 0), 0, sourceRes.topLeft());
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

QString GerberGenerator::cleanOutline(const QString & outlineSvg)
{
	QDomDocument doc;
	doc.setContent(outlineSvg);
	QList<QDomElement> leaves;
    QDomElement root = doc.documentElement();
    TextUtils::collectLeaves(root, leaves);

	if (leaves.count() == 0) return "";
	if (leaves.count() == 1) return outlineSvg;

	if (leaves.count() > 1) {
		for (int i = 0; i < leaves.count(); i++) {
			QDomElement leaf = leaves.at(i);
			if (leaf.attribute("id", "").compare(MagicBoardOutlineID) == 0) {
				for (int j = 0; j < leaves.count(); j++) {
					if (i != j) {
						leaves.at(j).parentNode().removeChild(leaves.at(j));
					}
				}

				return doc.toString();
			}
		}
	}

	if (leaves.count() == 0) return "";

	return outlineSvg;
}
