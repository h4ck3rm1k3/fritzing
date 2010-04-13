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

$Revision: 4073 $:
$Author: cohen@irascible.com $:
$Date: 2010-03-31 18:11:02 +0200 (Wed, 31 Mar 2010) $

********************************************************************/

#include <QMessageBox>
#include <QFileDialog>
#include <QWebPage>
#include <QWebFrame>

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

//TODO: this whole thing should probably be cleaned up and moved to another file
void MainWindow::exportToGerber() {

    //NOTE: this assumes just one board per sketch
    //TODO: should deal with crazy multi-board setups someday...

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

	exportToGerber(exportDir, board);
}

void MainWindow::exportToGerber(const QString & exportDir, ItemBase * board) 
{
	if (board == NULL) {
		board = m_pcbGraphicsView->findBoard();
	}
	if (board == NULL) {
		DebugDialog::debug("board not found");
		return;
	}

	LayerList viewLayerIDs;
	viewLayerIDs << ViewLayer::GroundPlane << ViewLayer::Copper0 << ViewLayer::Copper0Trace;
	QSizeF imageSize;
	QString svg = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false);
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
	if (!copper0Out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber export: cannot open output file");
		return;
	}

    QTextStream copperStream(&copper0Out);
    copperStream << copper0Gerber.getGerber();

    // soldermask
    QString soldermaskFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_maskTop.gts";
    QFile maskOut(soldermaskFile);
	if (!maskOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber export: cannot open output file");
		return;
	}

    QTextStream maskStream(&maskOut);
    maskStream << copper0Gerber.getSolderMask();

    // drill file
    QString drillFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_drill.txt";
    QFile drillOut(drillFile);
	if (!drillOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber export: cannot open output file");
		return;
	}

    QTextStream drillStream(&drillOut);
    drillStream << copper0Gerber.getNCDrill();

    LayerList silkLayerIDs;
    silkLayerIDs << ViewLayer::Silkscreen  << ViewLayer::SilkscreenLabel;
	doSilk(silkLayerIDs, "_silkBottom.gbo", imageSize, board, exportDir);
    silkLayerIDs.clear();
    silkLayerIDs << ViewLayer::Silkscreen0  << ViewLayer::Silkscreen0Label;
	doSilk(silkLayerIDs, "_silkTop.gto", imageSize, board, exportDir);

    // now do it for the outline/contour
    LayerList outlineLayerIDs;
    outlineLayerIDs << ViewLayer::Board;
	QString svgOutline = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), outlineLayerIDs, outlineLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false);
    if (svgOutline.isEmpty()) {
        // tell the user something reasonable
        return;
    }

    result = domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn);
    if (!result) {
            // tell the user something reasonable
            return;
    }

    // create copper0 gerber from svg
    SVG2gerber outlineGerber(svgOutline, "board");

    // contour / board outline
    QString contourFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_contour.gm1";
    QFile contourOut(contourFile);
	if (!contourOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber export: cannot open output file");
		return;
	}

    QTextStream contourStream(&contourOut);
    contourStream << outlineGerber.getContour();
}

void MainWindow::doSilk(LayerList silkLayerIDs, const QString & gerberSuffix, QSizeF imageSize, ItemBase * board, const QString & exportDir ) 
{
	QString svgSilk = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, silkLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false);
    if (svgSilk.isEmpty()) {
        // tell the user something reasonable
        return;
    }

	bool anyConverted = false;
	bool converted[3];
	converted[0] = converted[1] = converted[2] = false;
	QString text[3];
	text[0] = text[1] = text[2] = svgSilk;
    if (TextUtils::squashElement(svgSilk, "text", "", QRegExp())) {
        TextUtils::squashNotElement(text[0], "text", "", QRegExp());
        anyConverted = converted[0] = true; 
	}

	// gerber can't handle ellipses that are rotated, so cull them all
    if (TextUtils::squashElement(svgSilk, "ellipse", "", QRegExp())) {
        TextUtils::squashNotElement(text[1], "ellipse", "", QRegExp());
		anyConverted = converted[1] = true;
    }

	// gerber can't handle paths with curves
    if (TextUtils::squashElement(svgSilk, "path", "d", AaCc)) {
        TextUtils::squashNotElement(text[2], "path", "d", AaCc);
		anyConverted = converted[2] = true;
    }

    if (anyConverted) {

		QString svg;
		bool firstTime = true;
		for (int i = 0; i < 3; i++) {
			if (converted[i]) {
				if (firstTime) {
					firstTime = false;
					svg = text[i];
				}
				else {
					svg = TextUtils::mergeSvg(svg, text[i]);
				}
			}
		}

		qreal res = GraphicsUtils::StandardFritzingDPI;
		QRectF source = board->boundingRect();
		int swidth = source.width();
		int sheight = source.height();
		int twidth = res * swidth / FSvgRenderer::printerScale();
		int theight = res * sheight / FSvgRenderer::printerScale();

		QSize imgSize(twidth, theight);

		// expand the svg to fill the space of the image
		QRegExp widthFinder("width=([^i]+)in");
		svg.replace(widthFinder, QString("width=\"%1px").arg(twidth));
		QRegExp heightFinder("height=([^i]+)in");
		svg.replace(heightFinder, QString("height=\"%1px").arg(theight));

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
		gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#ffffff", "silkscreen");
		foreach (QString gsvg, gpg.newSVGs()) {
			svgSilk = TextUtils::mergeSvg(svgSilk, gsvg);
		}
	}

#ifndef QT_NO_DEBUG
	// for debugging silkscreen svg
    QFile silkout(QDir::temp().absoluteFilePath(gerberSuffix + ".svg"));
	if (silkout.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream silkStream(&silkout);
		silkStream << svgSilk;
		silkout.close();
	}
#endif

    // create copper0 gerber from svg
    SVG2gerber silk0Gerber(svgSilk, "silk");

    QString silk0File = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + gerberSuffix;
    QFile silk0Out(silk0File);
	if (!silk0Out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber export: cannot open output file");
		return;
	}

    QTextStream silkStream(&silk0Out);
    silkStream << silk0Gerber.getGerber();
}


void MainWindow::addSvgItem(const QString & svg, QPointF p, QList<QGraphicsSvgItem *> & items, QList<QSvgRenderer *> & renderers) {
	QStringList exceptions;
	exceptions << "none" << "";
	QString toColor("#FFFF00");
	QByteArray byteArray;
	SvgFileSplitter::changeColors(svg, toColor, exceptions, byteArray);
	QGraphicsSvgItem * item = new QGraphicsSvgItem();
	QSvgRenderer * renderer = new QSvgRenderer(byteArray);
	item->setSharedRenderer(renderer);
	item->setPos(p);								// the rendering is offset from the board, so move the textItem to the board location
	item->setVisible(true);
	item->setZValue(-999999);				// underneath
	m_pcbGraphicsView->scene()->addItem(item);
	items.append(item);
	renderers.append(renderer);
}
