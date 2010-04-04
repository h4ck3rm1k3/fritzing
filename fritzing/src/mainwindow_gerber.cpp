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

void MainWindow::exportToGerber(QString exportDir, ItemBase * board) 
{
	if (board == NULL) {
		board = m_pcbGraphicsView->findBoard();
	}
	if (board == NULL) {
		DebugDialog::debug("board not found");
		return;
	}

	QList<ViewLayer::ViewLayerID> viewLayerIDs;
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

    // now do it for silk
    QList<ViewLayer::ViewLayerID> silkLayerIDs;
    silkLayerIDs << ViewLayer::Silkscreen /* TODO:  << ViewLayer::SilkscreenLabel  */;
	QString svgSilk = m_pcbGraphicsView->renderToSVG(FSvgRenderer::printerScale(), silkLayerIDs, silkLayerIDs, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false);
    if (svgSilk.isEmpty()) {
        // tell the user something reasonable
        return;
    }

    bool svgToConvert = false;
	QString textOnly = svgSilk;
    if (TextUtils::squashElement(svgSilk, "text", "", QRegExp())) {
        TextUtils::squashNotElement(textOnly, "text", "", QRegExp());
        svgToConvert = true;
	}

	// gerber can't handle ellipses that are rotated, so cull them all
	QString ellipseOnly = svgSilk;
    if (TextUtils::squashElement(svgSilk, "ellipse", "", QRegExp())) {
        TextUtils::squashNotElement(ellipseOnly, "ellipse", "", QRegExp());
        if (!svgToConvert) {
            textOnly = ellipseOnly;
        }
        else {
            textOnly = TextUtils::mergeSvg(textOnly, ellipseOnly);
        }
        svgToConvert = true;
    }

	// gerber can't handle paths with curves
	QString curvesOnly = svgSilk;
    if (TextUtils::squashElement(svgSilk, "path", "d", AaCc)) {
        TextUtils::squashNotElement(curvesOnly, "path", "d", AaCc);
        if (!svgToConvert) {
            textOnly = curvesOnly;
        }
        else {
            textOnly = TextUtils::mergeSvg(textOnly, curvesOnly);
        }
        svgToConvert = true;
    }

	bool partLabelsVisible = m_pcbGraphicsView->partLabelsVisible();

    if (partLabelsVisible || svgToConvert) {
		// add labels to silkscreen layer
		m_pcbGraphicsView->saveLayerVisibility();
		m_pcbGraphicsView->setAllLayersVisible(false);
		m_pcbGraphicsView->setLayerVisible(ViewLayer::SilkscreenLabel, true);

		QList<QGraphicsItem*> selItems = m_pcbGraphicsView->scene()->selectedItems();
		foreach(QGraphicsItem *item, selItems) {
			item->setSelected(false);
		}

		QSizeF boardImageSize = board->size();
		qreal res = GraphicsUtils::StandardFritzingDPI;

		QRectF source = board->boundingRect();
		QPointF p = board->mapToScene(QPointF(0,0));
		source.moveTo(p.x(), p.y());
		int swidth = source.width();
		int sheight = source.height();
		int twidth = res * swidth / FSvgRenderer::printerScale();
		int theight = res * sheight / FSvgRenderer::printerScale();

		QSize imgSize(twidth, theight);
		QImage image(imgSize, QImage::Format_RGB32);
		image.fill(0);
		image.setDotsPerMeterX(res * GraphicsUtils::InchesPerMeter);
		image.setDotsPerMeterY(res * GraphicsUtils::InchesPerMeter);
		QRectF target(0, 0, twidth, theight);

		QBrush brush = m_pcbGraphicsView->scene()->backgroundBrush();
		m_pcbGraphicsView->scene()->setBackgroundBrush(Qt::black);

		QPainter painter;
		painter.begin(&image);
		QGraphicsSvgItem * textItem = NULL;
		QSvgRenderer * textRenderer = NULL;
        if (svgToConvert) {
			QStringList exceptions;
			exceptions << "none" << "";
			QString toColor("#FFFFFF");
			QByteArray textByteArray;
			SvgFileSplitter::changeColors(textOnly, toColor, exceptions, textByteArray);
			textItem = new QGraphicsSvgItem();
			textRenderer = new QSvgRenderer(textByteArray);
			textItem->setSharedRenderer(textRenderer);
			textItem->setPos(p);								// the rendering is offset from the board, so move the textItem to the board location
			textItem->setVisible(true);
			textItem->setZValue(-999999);				// underneath
			m_pcbGraphicsView->scene()->addItem(textItem);
		}

		m_pcbGraphicsView->scene()->render(&painter, target, source, Qt::KeepAspectRatio);
		painter.end();

		if (textItem) {
			delete textItem;
			delete textRenderer;
		}

		foreach(QGraphicsItem *item, selItems) {
			item->setSelected(true);
		}

		m_pcbGraphicsView->scene()->setBackgroundBrush(brush);
		m_pcbGraphicsView->restoreLayerVisibility();

		GroundPlaneGenerator gpg;
		gpg.scanImage(image, image.width(), image.height(), GraphicsUtils::StandardFritzingDPI / res, GraphicsUtils::StandardFritzingDPI, "#ffffff", "silkscreen");
		foreach (QString gsvg, gpg.newSVGs()) {
			svgSilk = TextUtils::mergeSvg(svgSilk, gsvg);
		}
	}

#ifndef QT_NO_DEBUG
	// for debugging silkscreen svg
    QFile silkout(QDir::temp().absoluteFilePath("silk.svg"));
	if (silkout.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream silkStream(&silkout);
		silkStream << svgSilk;
		silkout.close();
	}
#endif

    result = domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn);
    if (!result) {
            // tell the user something reasonable
            return;
    }

    // create copper0 gerber from svg
    SVG2gerber silk0Gerber(svgSilk, "silk");

    QString silk0File = exportDir + "/" +
                          QFileInfo(m_fileName).fileName().remove(FritzingSketchExtension)
                          + "_silkBottom.gbo";
    QFile silk0Out(silk0File);
	if (!silk0Out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DebugDialog::debug("gerber export: cannot open output file");
		return;
	}

    QTextStream silkStream(&silk0Out);
    silkStream << silk0Gerber.getGerber();

    // now do it for the outline/contour
    QList<ViewLayer::ViewLayerID> outlineLayerIDs;
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

