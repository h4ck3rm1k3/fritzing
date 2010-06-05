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

	// TODO:  need copper1 (.gbl) mask (.gbs) 

	LayerList viewLayerIDs;
	viewLayerIDs << ViewLayer::GroundPlane0 << ViewLayer::Copper0 << ViewLayer::Copper0Trace;
	doCopper(board, viewLayerIDs, "copper0", "_copperTop.gtl", "_maskTop.gts", true, exportDir);

	if (m_pcbGraphicsView->boardLayers() == 2) {
		viewLayerIDs.clear();
		viewLayerIDs << ViewLayer::GroundPlane1 << ViewLayer::Copper1 << ViewLayer::Copper1Trace;
		doCopper(board, viewLayerIDs, "copper1", "_copperBottom.gbl", "_maskBottom.gbs", false, exportDir);
	}

    LayerList silkLayerIDs;
    silkLayerIDs << ViewLayer::Silkscreen1  << ViewLayer::Silkscreen1Label;
	doSilk(silkLayerIDs, "_silkBottom.gbo", board, exportDir);
    silkLayerIDs.clear();
    silkLayerIDs << ViewLayer::Silkscreen0  << ViewLayer::Silkscreen0Label;
	doSilk(silkLayerIDs, "_silkTop.gto", board, exportDir);

    // now do it for the outline/contour
    LayerList outlineLayerIDs;
    outlineLayerIDs << ViewLayer::Board;
	QSizeF imageSize;
	QString svgOutline = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), outlineLayerIDs, outlineLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false);
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
		QMessageBox::warning(this, tr("Fritzing"), tr("outline file export failure (1)"));
        return;
    }

    // create copper0 gerber from svg
    SVG2gerber outlineGerber;
	outlineGerber.convert(svgOutline, "board");

    // contour / board outline
    QString contourFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_contour.gm1";
    QFile contourOut(contourFile);
	if (!contourOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Fritzing"), tr("outline file export failure (2)"));
		return;
	}

    QTextStream contourStream(&contourOut);
    contourStream << outlineGerber.getContour();
}

void MainWindow::doCopper(ItemBase * board, LayerList & viewLayerIDs, const QString & copperName, const QString & copperSuffix, const QString & solderMaskSuffix, bool doDrill, const QString & exportDir) 
{
	QSizeF imageSize;
	QString svg = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false);
	if (svg.isEmpty()) {
		QMessageBox::warning(this, tr("Fritzing"), tr("%1 file export failure (1)").arg(copperName));
		return;
	}

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn);
	if (!result) {
		QMessageBox::warning(this, tr("Fritzing"), tr("%1 file export failure (2)").arg(copperName));
		return;
	}

    // create copper gerber from svg
    SVG2gerber copperGerber;
	copperGerber.convert(svg, copperName);

    QString copperFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + copperSuffix;
    QFile copperOut(copperFile);
	if (!copperOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Fritzing"), tr("%1 file export failure (3)").arg(copperName));
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
		QMessageBox::warning(this, tr("Fritzing"), tr("%1 mask file export failure (4)").arg(copperName));
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
			QMessageBox::warning(this, tr("Fritzing"), tr("%1 drill file export failure (5)").arg(copperName));
			return;
		}

		QTextStream drillStream(&drillOut);
		drillStream << copperGerber.getNCDrill();
		drillStream.flush();
		drillOut.close();
	}
}

void MainWindow::doSilk(LayerList silkLayerIDs, const QString & gerberSuffix, ItemBase * board, const QString & exportDir ) 
{
	QSizeF imageSize;
	QString svgSilk = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, silkLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false);
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
		gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#ffffff", "silkscreen", false);
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

    // create silk gerber from svg
    SVG2gerber silkGerber;
	silkGerber.convert(svgSilk, "silk");

    QString silkFile = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + gerberSuffix;
    QFile silkOut(silkFile);
	if (!silkOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Fritzing"), tr("silk file export failure"));
		return;
	}

    QTextStream silkStream(&silkOut);
    silkStream << silkGerber.getGerber();
	silkStream.flush();
	silkOut.close();
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
