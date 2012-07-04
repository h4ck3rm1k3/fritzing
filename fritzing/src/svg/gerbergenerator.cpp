/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include <qmath.h>

#include "gerbergenerator.h"
#include "../debugdialog.h"
#include "../fsvgrenderer.h"
#include "../sketch/pcbsketchwidget.h"
#include "svgfilesplitter.h"
#include "groundplanegenerator.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../utils/folderutils.h"

static QRegExp AaCc("[aAcCqQtTsS]");
static QRegExp MultipleZs("z\\s*[^\\s]");

const QString GerberGenerator::SilkTopSuffix = "_silkTop.gto";
const QString GerberGenerator::SilkBottomSuffix = "_silkBottom.gbo";
const QString GerberGenerator::CopperTopSuffix = "_copperTop.gtl";
const QString GerberGenerator::CopperBottomSuffix = "_copperBottom.gbl";
const QString GerberGenerator::MaskTopSuffix = "_maskTop.gts";
const QString GerberGenerator::MaskBottomSuffix = "_maskBottom.gbs";
const QString GerberGenerator::DrillSuffix = "_drill.txt";
const QString GerberGenerator::OutlineSuffix = "_contour.gm1";
const QString GerberGenerator::MagicBoardOutlineID = "boardoutline";

const double GerberGenerator::MaskClearanceMils = 3;	

////////////////////////////////////////////

bool pixelsCollide(QImage * image1, QImage * image2, int x1, int y1, int x2, int y2) {
	for (int y = y1; y < y2; y++) {
		for (int x = x1; x < x2; x++) {
			QRgb p1 = image1->pixel(x, y);
			if (p1 == 0xffffffff) continue;

			QRgb p2 = image2->pixel(x, y);
			if (p2 == 0xffffffff) continue;

			//DebugDialog::debug(QString("p1:%1 p2:%2").arg(p1, 0, 16).arg(p2, 0, 16));

			return true;
		}
	}

	return false;
}

////////////////////////////////////////////

void GerberGenerator::exportToGerber(const QString & prefix, const QString & exportDir, ItemBase * board, PCBSketchWidget * sketchWidget, bool displayMessageBoxes) 
{
	if (board == NULL) {
        int boardCount;
		board = sketchWidget->findSelectedBoard(boardCount);
		if (boardCount == 0) {
			DebugDialog::debug("board not found");
			return;
		}
		if (board == NULL) {
			DebugDialog::debug("multiple boards found");
			return;
		}
	}

	LayerList viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Bottom);
	int copperInvalidCount = doCopper(board, sketchWidget, viewLayerIDs, "Copper0", CopperBottomSuffix, prefix, exportDir, displayMessageBoxes);

    
    if (sketchWidget->boardLayers() == 2) {
		viewLayerIDs = ViewLayer::copperLayers(ViewLayer::Top);
		copperInvalidCount += doCopper(board, sketchWidget, viewLayerIDs, "Copper1", CopperTopSuffix, prefix, exportDir, displayMessageBoxes);
	}




	LayerList maskLayerIDs = ViewLayer::maskLayers(ViewLayer::Bottom);
	QString maskBottom, maskTop;
	int maskInvalidCount = doMask(maskLayerIDs, "Mask0", MaskBottomSuffix, board, sketchWidget, prefix, exportDir, displayMessageBoxes, maskBottom);

	if (sketchWidget->boardLayers() == 2) {
		maskLayerIDs = ViewLayer::maskLayers(ViewLayer::Top);
		maskInvalidCount += doMask(maskLayerIDs, "Mask1", MaskTopSuffix, board, sketchWidget, prefix, exportDir, displayMessageBoxes, maskTop);
	}

    LayerList silkLayerIDs = ViewLayer::silkLayers(ViewLayer::Top);
	int silkInvalidCount = doSilk(silkLayerIDs, "Silk1", SilkTopSuffix, board, sketchWidget, prefix, exportDir, displayMessageBoxes, maskTop);
    silkLayerIDs = ViewLayer::silkLayers(ViewLayer::Bottom);
	silkInvalidCount += doSilk(silkLayerIDs, "Silk0", SilkBottomSuffix, board, sketchWidget, prefix, exportDir, displayMessageBoxes, maskBottom);

    // now do it for the outline/contour
    LayerList outlineLayerIDs = ViewLayer::outlineLayers();
	QSizeF imageSize;
	bool empty;
	QString svgOutline = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), outlineLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, empty);
    if (svgOutline.isEmpty()) {
        displayMessage(QObject::tr("outline is empty"), displayMessageBoxes);
        return;
    }

	svgOutline = cleanOutline(svgOutline);
	svgOutline = clipToBoard(svgOutline, board, "board", SVG2gerber::ForOutline, "");

	QXmlStreamReader streamReader(svgOutline);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

    // create outline gerber from svg
    SVG2gerber outlineGerber;
	int outlineInvalidCount = outlineGerber.convert(svgOutline, sketchWidget->boardLayers() == 2, "contour", SVG2gerber::ForOutline, svgSize * GraphicsUtils::StandardFritzingDPI);
	
	//DebugDialog::debug(QString("outline output: %1").arg(outlineGerber.getGerber()));
	saveEnd("contour", exportDir, prefix, OutlineSuffix, displayMessageBoxes, outlineGerber);

	doDrill(board, sketchWidget, prefix, exportDir, displayMessageBoxes);

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
	QString svg = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, empty);
	if (svg.isEmpty()) {
		displayMessage(QObject::tr("%1 file export failure (1)").arg(copperName), displayMessageBoxes);
		return 0;
	}

	QXmlStreamReader streamReader(svg);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svg = clipToBoard(svg, board, copperName, SVG2gerber::ForCopper, "");
	if (svg.isEmpty()) {
		displayMessage(QObject::tr("%1 file export failure (3)").arg(copperName), displayMessageBoxes);
		return 0;
	}

	return doEnd(svg, sketchWidget->boardLayers(), copperName, SVG2gerber::ForCopper, svgSize * GraphicsUtils::StandardFritzingDPI, exportDir, filename, copperSuffix, displayMessageBoxes);
}


int GerberGenerator::doSilk(LayerList silkLayerIDs, const QString & silkName, const QString & gerberSuffix, ItemBase * board, PCBSketchWidget * sketchWidget, const QString & filename, const QString & exportDir, bool displayMessageBoxes, const QString & clipString) 
{
	QSizeF imageSize;
	bool empty;
	QString svgSilk = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, empty);
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

	svgSilk = clipToBoard(svgSilk, board, silkName, SVG2gerber::ForSilk, clipString);
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
	QString svgDrill = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), drillLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, empty);
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

	svgDrill = clipToBoard(svgDrill, board, "Copper0", SVG2gerber::ForDrill, "");
	if (svgDrill.isEmpty()) {
		displayMessage(QObject::tr("drill export failure"), displayMessageBoxes);
		return 0;
	}

	return doEnd(svgDrill, sketchWidget->boardLayers(), "drill", SVG2gerber::ForDrill, svgSize * GraphicsUtils::StandardFritzingDPI, exportDir, filename, DrillSuffix, displayMessageBoxes);
}

int GerberGenerator::doMask(LayerList maskLayerIDs, const QString &maskName, const QString & gerberSuffix, ItemBase * board, PCBSketchWidget * sketchWidget, const QString & filename, const QString & exportDir, bool displayMessageBoxes, QString & clipString) 
{
	// don't want these in the mask laqyer
	QList<ItemBase *> copperLogoItems;
	sketchWidget->hideCopperLogoItems(copperLogoItems);

	QSizeF imageSize;
	bool empty;
	QString svgMask = sketchWidget->renderToSVG(FSvgRenderer::printerScale(), maskLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, empty);
    if (svgMask.isEmpty()) {
		displayMessage(QObject::tr("mask file export failure (1)"), displayMessageBoxes);
        return 0;
    }

	sketchWidget->restoreCopperLogoItems(copperLogoItems);

	if (empty) {
		// don't bother with file
		return 0;
	}

	svgMask = TextUtils::expandAndFill(svgMask, "black", MaskClearanceMils * 2);
	if (svgMask.isEmpty()) {
		displayMessage(QObject::tr("%1 mask export failure (2)").arg(maskName), displayMessageBoxes);
		return 0;
	}

	QXmlStreamReader streamReader(svgMask);
	QSizeF svgSize = FSvgRenderer::parseForWidthAndHeight(streamReader);

	svgMask = clipToBoard(svgMask, board, maskName, SVG2gerber::ForCopper, "");
	if (svgMask.isEmpty()) {
		displayMessage(QObject::tr("mask export failure"), displayMessageBoxes);
		return 0;
	}

	clipString = svgMask;

	return doEnd(svgMask, sketchWidget->boardLayers(), maskName, SVG2gerber::ForCopper, svgSize * GraphicsUtils::StandardFritzingDPI, exportDir, filename, gerberSuffix, displayMessageBoxes);
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

    QString outname = exportDir + "/" +  prefix + suffix;
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

QString GerberGenerator::clipToBoard(QString svgString, ItemBase * board, const QString & layerName, SVG2gerber::ForWhy forWhy, const QString & clipString) {
	QRectF source = board->sceneBoundingRect();
	source.moveTo(0, 0);
	return clipToBoard(svgString, source, layerName, forWhy, clipString);
}

QString GerberGenerator::clipToBoard(QString svgString, QRectF & boardRect, const QString & layerName, SVG2gerber::ForWhy forWhy, const QString & clipString) {
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

    bool multipleContours = false;
    if (forWhy == SVG2gerber::ForOutline) { 
        // split path into multiple contours
        QDomNodeList paths = root.elementsByTagName("path");
        // should only be one
        for (int p = 0; p < paths.count(); p++) {
            QDomElement path = paths.at(p).toElement();
            QString originalPath = path.attribute("d", "").trimmed();
            if (MultipleZs.indexIn(originalPath) >= 0) {
                multipleContours = true;
                QStringList ds = path.attribute("d").split("z", QString::SkipEmptyParts);
                for (int i = 1; i < ds.count(); i++) {
                    QDomElement newPath = path.cloneNode(true).toElement();
                    QString z = ((i < ds.count() - 1) || originalPath.endsWith("z", Qt::CaseInsensitive)) ? "z" : "";
                    QString d = ds.at(i).trimmed() + z;
                    if (!d.startsWith("M")) {
                        DebugDialog::debug("subpath doesn't start with M: " + originalPath);
                    }
                    newPath.setAttribute("d",  d);
                    path.parentNode().appendChild(newPath);
                }
                path.setAttribute("d", ds.at(0) + "z");
            }
        }
    }

	// document 2 will contain svg that must be rasterized for gerber conversion
	QDomDocument domDocument2 = domDocument1.cloneNode(true).toDocument();

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

	// gerber can't handle multiple subpaths if there are intersections
    if (TextUtils::squashElement(domDocument1, "path", "d", MultipleZs)) {
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
	// convert from pixel dpi to StandardFritzingDPI
	QRectF sourceRes(boardRect.left() * res / FSvgRenderer::printerScale(), boardRect.top() * res / FSvgRenderer::printerScale(), 
					 boardRect.width() * res / FSvgRenderer::printerScale(), boardRect.height() * res / FSvgRenderer::printerScale());
	int twidth = sourceRes.width();
	int theight = sourceRes.height();
	QSize imgSize(twidth + 2, theight + 2);
	QRectF target(0, 0, twidth, theight);

	QImage * clipImage = NULL;
	if (!clipString.isEmpty()) {
		clipImage = new QImage(imgSize, QImage::Format_Mono);
		clipImage->fill(0xffffffff);
		clipImage->setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
		clipImage->setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);

		QXmlStreamReader reader(clipString);
		QSvgRenderer renderer(&reader);		
		QPainter painter;
		painter.begin(clipImage);
		renderer.render(&painter, target);
		painter.end();

#ifndef QT_NO_DEBUG
        clipImage->save(FolderUtils::getUserDataStorePath("") + "/clip.png");
#endif

	}

	svgString = TextUtils::removeXMLEntities(domDocument1.toString());

	QXmlStreamReader reader(svgString);
	QSvgRenderer renderer(&reader);
	bool anyClipped = false;
    if (forWhy !=  SVG2gerber::ForOutline) { 
	    for (int i = 0; i < transformCount1; i++) {
		    QString n = QString::number(i);
		    QRectF bounds = renderer.boundsOnElement(n);
		    QMatrix m = renderer.matrixForElement(n);
		    QDomElement element = leaves1.at(i);
		    QRectF mBounds = m.mapRect(bounds);
		    if (mBounds.left() < sourceRes.left() - 0.1|| mBounds.top() < sourceRes.top() - 0.1 || mBounds.right() > sourceRes.right() + 0.1 || mBounds.bottom() > sourceRes.bottom() + 0.1) {
			    // element is outside of bounds--squash it so it will be clipped
			    // we don't care if the board shape is irregular
			    // since anything printed between the shape and the bounding rectangle 
			    // will be physically clipped when the board is cut out
			    element.setTagName("g");
			    anyClipped = anyConverted = true;
		    }	
	    }
    }

	if (clipImage) {
		QImage another(imgSize, QImage::Format_Mono);
		another.fill(0xffffffff);
		another.setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
		another.setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);

		svgString = TextUtils::removeXMLEntities(domDocument1.toString());
		QXmlStreamReader reader(svgString);
		QSvgRenderer renderer(&reader);
		QPainter painter;
		painter.begin(&another);
		renderer.render(&painter, target);
		painter.end();

		for (int i = 0; i < transformCount1; i++) {
			QDomElement element = leaves1.at(i);
			if (element.tagName().compare("g") == 0) {
				// element is already converted to raster space, we'll clip it later
				continue;
			}

			QString n = QString::number(i);
			QRectF bounds = renderer.boundsOnElement(n);
			QMatrix m = renderer.matrixForElement(n);
			QRectF mBounds = m.mapRect(bounds);

			int x1 = qFloor(qMax((qreal) 0.0, mBounds.left() - sourceRes.left()));          // atmel compiler fails without cast
			int x2 = qCeil(qMin(sourceRes.width(), mBounds.right() - sourceRes.left()));
			int y1 = qFloor(qMax((qreal) 0.0, mBounds.top() - sourceRes.top()));            // atmel compiler fails without cast
			int y2 = qCeil(qMin(sourceRes.height(), mBounds.bottom() - sourceRes.top()));
			
			if (pixelsCollide(&another, clipImage, x1, y1, x2, y2)) {
				element.setTagName("g");
				anyClipped = anyConverted = true;
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
			if (element1.tagName().compare("g") != 0) {
				// document 1 element svg can be directly converted to gerber
				// so remove it from document 2
				QDomElement element2 = leaves2.at(i);
				element2.setTagName("g");
			}
		}
		

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

		QStringList exceptions;
		exceptions << "none" << "";
		QString toColor("#000000");
        QDomElement root2 = domDocument2.documentElement();
		SvgFileSplitter::changeColors(root2, toColor, exceptions);

        QImage image(imgSize, QImage::Format_Mono);
		image.setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
		image.setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);

        if (forWhy == SVG2gerber::ForOutline) {		
            QDomNodeList paths = root.elementsByTagName("path");
            for (int p = 0; p < paths.count(); p++) {
                QDomElement path = paths.at(p).toElement();
                path.setTagName("g");
            }
            for (int p = 0; p < paths.count(); p++) {
                QDomElement path = paths.at(p).toElement();
                path.setTagName("path");
                if (p > 0) {
                    paths.at(p - 1).toElement().setTagName("g");
                }
                image.fill(0xffffffff);
		        QByteArray svg = TextUtils::removeXMLEntities(domDocument2.toString()).toUtf8();

		        QSvgRenderer renderer(svg);
		        QPainter painter;
		        painter.begin(&image);
		        renderer.render(&painter, target);
		        painter.end();
		        image.invertPixels();				// need white pixels on a black background for GroundPlaneGenerator

                #ifndef QT_NO_DEBUG
		            image.save(QString("%2/output%1.png").arg(p).arg(FolderUtils::getUserDataStorePath("")));
                #endif

		        GroundPlaneGenerator gpg;
		        gpg.setLayerName(layerName);
		        gpg.setMinRunSize(1, 1);
                gpg.scanOutline(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#000000", false, false, QSizeF(0, 0), 0);
		        if (gpg.newSVGs().count() > 0) {
                    svgString = gpg.mergeSVGs(svgString, "");
		        }
            }
		}
        else {
		    image.fill(0xffffffff);
		    QByteArray svg = TextUtils::removeXMLEntities(domDocument2.toString()).toUtf8();
		    QSvgRenderer renderer(svg);
		    QPainter painter;
		    painter.begin(&image);
		    renderer.render(&painter, target);
		    painter.end();
		    image.invertPixels();				// need white pixels on a black background for GroundPlaneGenerator

    #ifndef QT_NO_DEBUG
		    image.save(FolderUtils::getUserDataStorePath("") + "/preclip_output.png");
    #endif

		    if (clipImage != NULL) {
			    // can this be done with a single blt using composition mode
			    // if not, grab a scanline instead of testing every pixel
			    for (int y = 0; y < theight; y++) {
				    for (int x = 0; x < twidth; x++) {
					    if (clipImage->pixel(x, y) != 0xffffffff) {
						    image.setPixel(x, y, 0);
					    }
				    }
			    }
		    }

    #ifndef QT_NO_DEBUG
		    image.save(FolderUtils::getUserDataStorePath("") + "/output.png");
    #endif

		    GroundPlaneGenerator gpg;
		    gpg.setLayerName(layerName);
		    gpg.setMinRunSize(1, 1);
			gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#000000", false, false, QSizeF(0, 0), 0, sourceRes.topLeft());
		    if (gpg.newSVGs().count() > 0) {
                svgString = gpg.mergeSVGs(svgString, "");
		    }
		}
	}

	if (clipImage) delete clipImage;

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
