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

#include "panelizer.h"
#include "../../debugdialog.h"
#include "../../sketch/pcbsketchwidget.h"
#include "../../utils/textutils.h"
#include "../../utils/graphicsutils.h"
#include "../../utils/folderutils.h"
#include "../../utils/folderutils.h"
#include "../../items/resizableboard.h"
#include "../../items/logoitem.h"
#include "../../items/groundplane.h"
#include "../../fsvgrenderer.h"
#include "../../fapplication.h"
#include "../../svg/gerbergenerator.h"
#include "../../autoroute/cmrouter/cmrouter.h"
#include "../../referencemodel/referencemodel.h"
#include "../../version/version.h"
#include "../../processeventblocker.h"

#include "tileutils.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <qmath.h>
#include <limits>
#include <QPrinter>

static int OutlineLayer = 0;
static int SilkTopLayer = 0;

bool areaGreaterThan(PanelItem * p1, PanelItem * p2)
{
	return p1->boardSizeInches.width() * p1->boardSizeInches.height() > p2->boardSizeInches.width() * p2->boardSizeInches.height();
}

int allSpaces(Tile * tile, UserData userData) {
	QList<Tile*> * tiles = (QList<Tile*> *) userData;
	if (TiGetType(tile) == Tile::SPACE) {
		tiles->append(tile);
		return 0;
	}

	tiles->clear();
	return 1;			// stop the search
}

int allObstacles(Tile * tile, UserData userData) {
	if (TiGetType(tile) == Tile::OBSTACLE) {
		QList<Tile*> * obstacles = (QList<Tile*> *) userData;
		obstacles->append(tile);

	}

	return 0;
}

static int PlanePairIndex = 0;

static double Worst = std::numeric_limits<double>::max() / 4;

int roomOn(Tile * tile, TileRect & tileRect, BestPlace * bestPlace)
{
	int w = tileRect.xmaxi - tileRect.xmini;
	int h = tileRect.ymaxi - tileRect.ymini;
	if (bestPlace->width <= w && bestPlace->height <= h) {
		bestPlace->bestTile = tile;
		return 1;
	}

	TileRect temp;
	temp.xmini = tileRect.xmini;
	temp.xmaxi = temp.xmini + bestPlace->width;
	temp.ymini = tileRect.ymini;
	temp.ymaxi = temp.ymini + bestPlace->height;
	QList<Tile*> spaces;
	TiSrArea(tile, bestPlace->plane, &temp, allSpaces, &spaces);
	if (spaces.count()) {
		bestPlace->bestTile = tile;
		return 1;
	}

	return 0;
}

int roomAnywhere(Tile * tile, UserData userData) 
{
	if (TiGetType(tile) != Tile::SPACE) return 0;

	BestPlace * bestPlace = (BestPlace *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);

	return roomOn(tile, tileRect, bestPlace);
}

int roomOnTop(Tile * tile, UserData userData) 
{
	if (TiGetType(tile) != Tile::SPACE) return 0;

	BestPlace * bestPlace = (BestPlace *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);

	if (tileRect.ymini != bestPlace->maxRect.ymini) return 0;

	return roomOn(tile, tileRect, bestPlace);
}

int roomOnBottom(Tile * tile, UserData userData) 
{
	if (TiGetType(tile) != Tile::SPACE) return 0;

	BestPlace * bestPlace = (BestPlace *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);

	if (tileRect.ymaxi != bestPlace->maxRect.ymaxi) return 0;

	return roomOn(tile, tileRect, bestPlace);
}

int roomOnLeft(Tile * tile, UserData userData) 
{
	if (TiGetType(tile) != Tile::SPACE) return 0;

	BestPlace * bestPlace = (BestPlace *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);

	if (tileRect.xmini != bestPlace->maxRect.xmini) return 0;

	return roomOn(tile, tileRect, bestPlace);
}

int roomOnRight(Tile * tile, UserData userData) 
{
	if (TiGetType(tile) != Tile::SPACE) return 0;

	BestPlace * bestPlace = (BestPlace *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);

	if (tileRect.xmaxi != bestPlace->maxRect.xmaxi) return 0;

	return roomOn(tile, tileRect, bestPlace);
}


BestPlace::BestPlace() {
    bestTile = NULL;
    bestArea = Worst;
}

PanelItem::PanelItem(PanelItem * from) {
	this->boardName = from->boardName;
	this->path = from->path;
	this->required = from->required;
	this->maxOptional = from->maxOptional;
	this->boardSizeInches = from->boardSizeInches;
	this->boardID = from->boardID;
}

/////////////////////////////////////////////////////////////////////////////////

void Panelizer::panelize(FApplication * app, const QString & panelFilename) 
{
	QFile file(panelFilename);

	QString errorStr;
	int errorLine;
	int errorColumn;

	DebugDialog::setEnabled(true);

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("Unable to parse '%1': '%2' line:%3 column:%4").arg(panelFilename).arg(errorStr).arg(errorLine).arg(errorColumn));
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull() || root.tagName() != "panelizer") {
		DebugDialog::debug(QString("root element is not 'panelizer'"));
		return;
	}

	PanelParams panelParams;
	if (!initPanelParams(root, panelParams)) return;

	QDir outputDir(panelParams.outputFolder);
	outputDir.mkdir("svg");
	outputDir.mkdir("gerber");
	outputDir.mkdir("fz");

	QDir svgDir(outputDir);
	svgDir.cd("svg");
	if (!svgDir.exists()) {
		DebugDialog::debug(QString("unable to create svg folder in '%1'").arg(panelParams.outputFolder));
		return;
	}

	DebugDialog::debug(QString("svg folder '%1'\n").arg(svgDir.absolutePath()));

	QDir gerberDir(outputDir);
	gerberDir.cd("gerber");
	if (!gerberDir.exists()) {
		DebugDialog::debug(QString("unable to create gerber folder in '%1'").arg(panelParams.outputFolder));
		return;
	}

	DebugDialog::debug(QString("gerber folder '%1'\n").arg(gerberDir.absolutePath()));

	QDir fzDir(outputDir);
	fzDir.cd("fz");
	if (!fzDir.exists()) {
		DebugDialog::debug(QString("unable to create fz folder in '%1'").arg(panelParams.outputFolder));
		return;
	}

	DebugDialog::debug(QString("fz folder '%1'\n").arg(fzDir.absolutePath()));


	QDomElement boards = root.firstChildElement("boards");
	QDomElement board = boards.firstChildElement("board");
	if (board.isNull()) {
		DebugDialog::debug(QString("no <board> elements found"));
		return;
	}

	QHash<QString, QString> fzzFilePaths;
	QDomElement paths = root.firstChildElement("paths");
	QDomElement path = paths.firstChildElement("path");
	if (path.isNull()) {
		DebugDialog::debug(QString("no <path> elements found"));
		return;
	}

	collectFiles(path, fzzFilePaths);
	if (fzzFilePaths.count() == 0) {
		DebugDialog::debug(QString("no fzz files found in paths"));
		return;
	}
	
	board = boards.firstChildElement("board");
	if (!checkBoards(board, fzzFilePaths)) return;

	app->createUserDataStoreFolderStructure();
	app->registerFonts();
	app->loadReferenceModel("", false);

    QList<LayerThing> layerThingList;
	layerThingList.append(LayerThing("outline", ViewLayer::outlineLayers(), SVG2gerber::ForOutline, GerberGenerator::OutlineSuffix));  
	layerThingList.append(LayerThing("copper_top", ViewLayer::copperLayers(ViewLayer::Top), SVG2gerber::ForCopper, GerberGenerator::CopperTopSuffix));
	layerThingList.append(LayerThing("copper_bottom", ViewLayer::copperLayers(ViewLayer::Bottom), SVG2gerber::ForCopper, GerberGenerator::CopperBottomSuffix));
	layerThingList.append(LayerThing("mask_top", ViewLayer::maskLayers(ViewLayer::Top), SVG2gerber::ForMask, GerberGenerator:: MaskTopSuffix));
	layerThingList.append(LayerThing("mask_bottom", ViewLayer::maskLayers(ViewLayer::Bottom), SVG2gerber::ForMask, GerberGenerator::MaskBottomSuffix));
	layerThingList.append(LayerThing("paste_mask_top", ViewLayer::maskLayers(ViewLayer::Top), SVG2gerber::ForPasteMask, GerberGenerator:: PasteMaskTopSuffix));
	layerThingList.append(LayerThing("paste_mask_bottom", ViewLayer::maskLayers(ViewLayer::Bottom), SVG2gerber::ForPasteMask, GerberGenerator::PasteMaskBottomSuffix));
	layerThingList.append(LayerThing("silk_top", ViewLayer::silkLayers(ViewLayer::Top), SVG2gerber::ForSilk, GerberGenerator::SilkTopSuffix));
	layerThingList.append(LayerThing("silk_bottom", ViewLayer::silkLayers(ViewLayer::Bottom), SVG2gerber::ForSilk, GerberGenerator::SilkBottomSuffix));
	layerThingList.append(LayerThing("drill", ViewLayer::drillLayers(), SVG2gerber::ForDrill, GerberGenerator::DrillSuffix));

    for (int i = 0; i < layerThingList.count(); i++) {
        LayerThing layerThing = layerThingList.at(i);
        if (layerThing.name.compare("outline") == 0) OutlineLayer = i;
        else if (layerThing.name.compare("silk_top") == 0) SilkTopLayer = i;
    }

	QList<PanelItem *> refPanelItems;
	board = boards.firstChildElement("board");
	if (!openWindows(board, fzzFilePaths, app, panelParams, fzDir, svgDir, refPanelItems, layerThingList)) return;

	QList<PanelItem *> insertPanelItems;
	int optionalCount = 0;
	foreach (PanelItem * panelItem, refPanelItems) {
		for (int i = 0; i < panelItem->required; i++) {
			PanelItem * copy = new PanelItem(panelItem);
			insertPanelItems.append(copy);
		}
		optionalCount += panelItem->maxOptional;
	}

	QList<PlanePair *> planePairs;
	planePairs << makePlanePair(panelParams, true);  

	qSort(insertPanelItems.begin(), insertPanelItems.end(), areaGreaterThan);
	bestFit(insertPanelItems, panelParams, planePairs);

    shrinkLastPanel(planePairs, insertPanelItems, panelParams);

	addOptional(optionalCount, refPanelItems, insertPanelItems, panelParams, planePairs);

	foreach (PlanePair * planePair, planePairs) {
		planePair->layoutSVG += "</svg>";
		QString fname = svgDir.absoluteFilePath(QString("%1.panel_%2.layout.svg").arg(panelParams.prefix).arg(planePair->index));
        TextUtils::writeUtf8(fname, planePair->layoutSVG);
	}

	foreach (PlanePair * planePair, planePairs) {
		for (int i = 0; i < layerThingList.count(); i++) {
			planePair->svgs << TextUtils::makeSVGHeader(1, GraphicsUtils::StandardFritzingDPI, planePair->panelWidth, planePair->panelHeight);
		}

        QList<PanelItem *> needToRotate;
		foreach (PanelItem * panelItem, insertPanelItems) {
			if (panelItem->planePair != planePair) continue;

			DebugDialog::debug(QString("placing %1 on panel %2").arg(panelItem->boardName).arg(planePair->index));

            doOnePanelItem(planePair, layerThingList, panelItem, svgDir);
		}


		DebugDialog::debug("after placement");
		QString prefix = QString("%1.panel_%2").arg(panelParams.prefix).arg(planePair->index);
		for (int i = 0; i < planePair->svgs.count(); i++) {
			if (planePair->svgs.at(i).isEmpty()) continue;

			planePair->svgs.replace(i, planePair->svgs.at(i) + "</svg>");

			QString fname = svgDir.absoluteFilePath(QString("%1.%2.svg").arg(prefix).arg(layerThingList.at(i).name));
			TextUtils::writeUtf8(fname, planePair->svgs.at(i));

			QString suffix = layerThingList.at(i).suffix;
			DebugDialog::debug("converting " + prefix + " " + suffix);
			QSizeF svgSize(planePair->panelWidth, planePair->panelHeight);
			SVG2gerber::ForWhy forWhy = layerThingList.at(i).forWhy;
			if (forWhy == SVG2gerber::ForMask || forWhy == SVG2gerber::ForPasteMask) forWhy = SVG2gerber::ForCopper;
			GerberGenerator::doEnd(planePair->svgs.at(i), 2, layerThingList.at(i).name, forWhy, svgSize, gerberDir.absolutePath(), prefix, suffix, false);
			DebugDialog::debug("after converting " + prefix + " " + suffix);
		}

		QDomDocument doc;
		QString merger = planePair->svgs.at(OutlineLayer);  // outline layer
		merger.replace("black", "#90f0a0");
		merger.replace("#000000", "#90f0a0");
		merger.replace("fill-opacity=\"0.5\"", "fill-opacity=\"1\"");
		TextUtils::mergeSvg(doc, merger, "");
		merger = planePair->svgs.at(SilkTopLayer);			// silktop layer
		merger.replace("black", "#909090");
		merger.replace("#000000", "#909090");
		TextUtils::mergeSvg(doc, merger, "");		
		merger = planePair->layoutSVG;				// layout
		merger.replace("'red'", "'none'");		// hide background rect
		TextUtils::mergeSvg(doc, merger, "");
		merger = TextUtils::mergeSvgFinish(doc);
		QString fname = svgDir.absoluteFilePath(QString("%1.%2.svg").arg(prefix).arg("identification"));
		TextUtils::writeUtf8(fname, merger);

        // save to pdf
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
        QString pdfname = fname;
        pdfname.replace(".svg", ".pdf");
		printer.setOutputFileName(pdfname);
		int res = printer.resolution();
			
		// now convert to pdf
		QSvgRenderer svgRenderer;
		svgRenderer.load(merger.toLatin1());
		double trueWidth = planePair->panelWidth;
		double trueHeight = planePair->panelHeight;
		QRectF target(0, 0, trueWidth * res, trueHeight * res);

		QSizeF psize((target.width() + printer.paperRect().width() - printer.width()) / res, 
						(target.height() + printer.paperRect().height() - printer.height()) / res);
		printer.setPaperSize(psize, QPrinter::Inch);

		QPainter painter;
		if (painter.begin(&printer))
		{
			svgRenderer.render(&painter, target);
		}

		painter.end();
	}
}


void Panelizer::bestFit(QList<PanelItem *> & insertPanelItems, PanelParams & panelParams, QList<PlanePair *> & planePairs)
{
	foreach (PanelItem * panelItem, insertPanelItems) {
		bestFitOne(panelItem, panelParams, planePairs, true);
	}
}

bool Panelizer::bestFitOne(PanelItem * panelItem, PanelParams & panelParams, QList<PlanePair *> & planePairs, bool createNew)
{
	//DebugDialog::debug(QString("panel %1").arg(panelItem->boardName));
	BestPlace bestPlace1, bestPlace2;
	bestPlace1.rotate90 = bestPlace2.rotate90 = false;
	bestPlace1.width = bestPlace2.width = realToTile(panelItem->boardSizeInches.width() + panelParams.panelSpacing);
	bestPlace1.height = bestPlace2.height = realToTile(panelItem->boardSizeInches.height() + panelParams.panelSpacing);
	int ppix = 0;
	while (ppix < planePairs.count()) {
		PlanePair *  planePair = planePairs.at(ppix);
		bestPlace1.plane = planePair->thePlane;
		bestPlace2.plane = planePair->thePlane90;
		bestPlace1.maxRect = planePair->tilePanelRect;
		TiSrArea(NULL, planePair->thePlane, &planePair->tilePanelRect, placeBestFit, &bestPlace1);
		if (bestPlace1.bestTile == NULL) {
			bestPlace2.maxRect = planePair->tilePanelRect90;
			TiSrArea(NULL, planePair->thePlane90, &planePair->tilePanelRect90, placeBestFit, &bestPlace2);
		}
		if (bestPlace1.bestTile == NULL && bestPlace2.bestTile == NULL ) {
			if (++ppix < planePairs.count()) {
				// try next panel
				continue;
			}

			if (!createNew) {
				return false;
			}

			// create next panel
			planePair = makePlanePair(panelParams, true);
			planePairs << planePair;
			DebugDialog::debug(QString("ran out of room placing %1").arg(panelItem->boardName));
			continue;
		}

		bool use2 = false;
		if (bestPlace1.bestTile == NULL) {
			use2 = true;
		}
		else if (bestPlace2.bestTile == NULL) {
		}
		else {
			// never actually get here
			use2 = bestPlace2.bestArea < bestPlace1.bestArea;
		}

		if (use2) {
			tileUnrotate90(bestPlace2.bestTileRect, bestPlace1.bestTileRect);
			bestPlace1.rotate90 = !bestPlace2.rotate90;
		}

		panelItem->x = tileToReal(bestPlace1.bestTileRect.xmini) ;
		panelItem->y = tileToReal(bestPlace1.bestTileRect.ymini);
		panelItem->rotate90 = bestPlace1.rotate90;

		DebugDialog::debug(QString("setting rotate90:%1 %2").arg(panelItem->rotate90).arg(panelItem->path));
		panelItem->planePair = planePair;

		TileRect tileRect;
		tileRect.xmini = bestPlace1.bestTileRect.xmini;
		tileRect.ymini = bestPlace1.bestTileRect.ymini;
		if (bestPlace1.rotate90) {
			tileRect.xmaxi = tileRect.xmini + bestPlace1.height;
			tileRect.ymaxi = tileRect.ymini + bestPlace1.width;
		}
		else {
			tileRect.ymaxi = tileRect.ymini + bestPlace1.height;
			tileRect.xmaxi = tileRect.xmini + bestPlace1.width;
		}

		double w = panelItem->boardSizeInches.width();
		double h = panelItem->boardSizeInches.height();
		if (panelItem->rotate90) {
			w = h;
			h = panelItem->boardSizeInches.width();
		}

		planePair->layoutSVG += QString("<rect x='%1' y='%2' width='%3' height='%4' stroke='none' fill='red'/>\n")
			.arg(panelItem->x * GraphicsUtils::StandardFritzingDPI)
			.arg(panelItem->y * GraphicsUtils::StandardFritzingDPI)
			.arg(GraphicsUtils::StandardFritzingDPI * w)
			.arg(GraphicsUtils::StandardFritzingDPI * h);

		QStringList strings = QFileInfo(panelItem->path).completeBaseName().split("_");
		double cx = GraphicsUtils::StandardFritzingDPI * (panelItem->x + (w / 2));
		int fontSize1 = 250;
		int fontSize2 = 150;
		int fontSize = fontSize1;
		double cy = GraphicsUtils::StandardFritzingDPI * (panelItem->y + (h  / 2));
		cy -= ((strings.count() - 1) * fontSize2 / 2);
		foreach (QString string, strings) {
			planePair->layoutSVG += QString("<text x='%1' y='%2' anchor='middle' font-family='OCRA' stroke='none' fill='#000000' text-anchor='middle' font-size='%3'>%4</text>\n")
				.arg(cx)
				.arg(cy)
				.arg(fontSize)
				.arg(string);
			cy += fontSize;
			if (fontSize == fontSize1) fontSize = fontSize2;
		}

		TiInsertTile(planePair->thePlane, &tileRect, NULL, Tile::OBSTACLE);
		TileRect tileRect90;
		tileRotate90(tileRect, tileRect90);
		TiInsertTile(planePair->thePlane90, &tileRect90, NULL, Tile::OBSTACLE);

		return true;
	}

	DebugDialog::debug("bestFitOne should never reach here");
	return false;
}

PlanePair * Panelizer::makePlanePair(PanelParams & panelParams, bool big)
{
	PlanePair * planePair = new PlanePair;

    if (big) {
        planePair->panelWidth = panelParams.panelWidth;
        planePair->panelHeight = panelParams.panelHeight;
    }
    else {
        planePair->panelWidth = panelParams.panelSmallWidth;
        planePair->panelHeight = panelParams.panelSmallHeight;
    }

	// for debugging
	planePair->layoutSVG = TextUtils::makeSVGHeader(1, GraphicsUtils::StandardFritzingDPI, planePair->panelWidth, planePair->panelHeight);
	planePair->index = PlanePairIndex++;

	Tile * bufferTile = TiAlloc();
	TiSetType(bufferTile, Tile::BUFFER);
	TiSetBody(bufferTile, NULL);

	QRectF panelRect(0, 0, planePair->panelWidth + panelParams.panelSpacing - panelParams.panelBorder, 
							planePair->panelHeight + panelParams.panelSpacing - panelParams.panelBorder);

    int l = fasterRealToTile(panelRect.left() - 10);
    int t = fasterRealToTile(panelRect.top() - 10);
    int r = fasterRealToTile(panelRect.right() + 10);
    int b = fasterRealToTile(panelRect.bottom() + 10);
    SETLEFT(bufferTile, l);
    SETYMIN(bufferTile, t);		

	planePair->thePlane = TiNewPlane(bufferTile, l, t, r, b);

    SETRIGHT(bufferTile, r);
	SETYMAX(bufferTile, b);		

	qrectToTile(panelRect, planePair->tilePanelRect);
	TiInsertTile(planePair->thePlane, &planePair->tilePanelRect, NULL, Tile::SPACE); 

	QMatrix matrix90;
	matrix90.rotate(90);
	QRectF panelRect90 = matrix90.mapRect(panelRect);

	Tile * bufferTile90 = TiAlloc();
	TiSetType(bufferTile90, Tile::BUFFER);
	TiSetBody(bufferTile90, NULL);

    l = fasterRealToTile(panelRect90.left() - 10);
    t = fasterRealToTile(panelRect90.top() - 10);
    r = fasterRealToTile(panelRect90.right() + 10);
    b = fasterRealToTile(panelRect90.bottom() + 10);
    SETLEFT(bufferTile90, l);
    SETYMIN(bufferTile90, t);		

	planePair->thePlane90 = TiNewPlane(bufferTile90, l, t, r, b);

    SETRIGHT(bufferTile90, r);
	SETYMAX(bufferTile90, b);		

	qrectToTile(panelRect90, planePair->tilePanelRect90);
	TiInsertTile(planePair->thePlane90, &planePair->tilePanelRect90, NULL, Tile::SPACE); 

	return planePair;
}

void Panelizer::collectFiles(QDomElement & path, QHash<QString, QString> & fzzFilePaths)
{
	while (!path.isNull()) {
		QDomNode node = path.firstChild();
		if (!node.isText()) {
			DebugDialog::debug(QString("missing text in <path> element"));
			return;
		}

		QString p = node.nodeValue();
		QDir dir(p);
		if (!dir.exists()) {
			DebugDialog::debug(QString("Directory '%1' doesn't exist").arg(p));
			return;
		}

		QStringList filepaths;
        QStringList filters("*" + FritzingBundleExtension);
        FolderUtils::collectFiles(dir, filters, filepaths);
		foreach (QString filepath, filepaths) {
			QFileInfo fileInfo(filepath);

			fzzFilePaths.insert(fileInfo.fileName(), filepath);
		}

		path = path.nextSiblingElement("path");
	}
}

bool Panelizer::checkBoards(QDomElement & board, QHash<QString, QString> & fzzFilePaths)
{
	while (!board.isNull()) {
		QString boardname = board.attribute("name");
		//DebugDialog::debug(QString("board %1").arg(boardname));
		bool ok;
		int optional = board.attribute("maxOptionalCount", "").toInt(&ok);
		if (!ok) {
			DebugDialog::debug(QString("maxOptionalCount for board '%1' not an integer: '%2'").arg(boardname).arg(board.attribute("maxOptionalCount")));
			return false;
		}

		int required = board.attribute("requiredCount", "").toInt(&ok);
		if (!ok) {
			DebugDialog::debug(QString("required for board '%1' not an integer: '%2'").arg(boardname).arg(board.attribute("maxOptionalCount")));
			return false;
		}

        if (optional > 0 || required> 0) {
		    QString path = fzzFilePaths.value(boardname, "");
		    if (path.isEmpty()) {
			    DebugDialog::debug(QString("File for board '%1' not found in search paths").arg(boardname));
			    return false;
		    }
        }
        else {
            DebugDialog::debug(QString("skipping board '%1'").arg(boardname));
        }

		board = board.nextSiblingElement("board");
	}

	return true;
}

bool Panelizer::openWindows(QDomElement & boardElement, QHash<QString, QString> & fzzFilePaths, FApplication * app, PanelParams & panelParams, QDir & fzDir, QDir & svgDir, QList<PanelItem *> & refPanelItems, QList<LayerThing> & layerThingList)
{
    QDir rotateDir(svgDir);
    QDir norotateDir(svgDir);
    rotateDir.mkdir("rotate");
    rotateDir.cd("rotate");
    norotateDir.mkdir("norotate");
    norotateDir.cd("norotate");

	while (!boardElement.isNull()) {
		int required = boardElement.attribute("requiredCount", "").toInt();
		int optional = boardElement.attribute("maxOptionalCount", "").toInt();
		if (required == 0 && optional == 0) {
			boardElement = boardElement.nextSiblingElement("board"); 
			continue;
		}

		QString boardName = boardElement.attribute("name");
		QString path = fzzFilePaths.value(boardName, "");
		int loaded = 0;
		MainWindow * mainWindow = app->loadWindows(loaded, false);
		mainWindow->noBackup();

		FolderUtils::setOpenSaveFolderAux(fzDir.absolutePath());

		if (!mainWindow->loadWhich(path, false, false, "")) {
			DebugDialog::debug(QString("failed to load '%1'").arg(path));
			return false;
		}

		mainWindow->showPCBView();
		foreach (QGraphicsItem * item, mainWindow->pcbView()->scene()->items()) {
			ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
			if (itemBase == NULL) continue;

			itemBase->setMoveLock(false);
		}
		
		QList<ItemBase *> boards = mainWindow->pcbView()->findBoard();
        foreach (ItemBase * boardItem, boards) {
		    PanelItem * panelItem = new PanelItem;
		    panelItem->boardName = boardName;
		    panelItem->path = path;
		    panelItem->required = required;
		    panelItem->maxOptional = optional;
            panelItem->boardID = boardItem->id();

		    QRectF sbr = boardItem->layerKinChief()->sceneBoundingRect();
		    panelItem->boardSizeInches = sbr.size() / GraphicsUtils::SVGDPI;
		    DebugDialog::debug(QString("board size inches c %1, %2, %3")
				    .arg(panelItem->boardSizeInches.width())
				    .arg(panelItem->boardSizeInches.height())
				    .arg(path));

		    /*
		    QSizeF boardSize = boardItem->size();
		    ResizableBoard * resizableBoard = qobject_cast<ResizableBoard *>(boardItem->layerKinChief());
		    if (resizableBoard != NULL) {
			    panelItem->boardSizeInches = resizableBoard->getSizeMM() / 25.4;
			    DebugDialog::debug(QString("board size inches a %1, %2, %3")
				    .arg(panelItem->boardSizeInches.width())
				    .arg(panelItem->boardSizeInches.height())
				    .arg(path), boardItem->sceneBoundingRect());
		    }
		    else {
			    panelItem->boardSizeInches = boardSize / GraphicsUtils::SVGDPI;
			    DebugDialog::debug(QString("board size inches b %1, %2, %3")
				    .arg(panelItem->boardSizeInches.width())
				    .arg(panelItem->boardSizeInches.height())
				    .arg(path), boardItem->sceneBoundingRect());
		    }
		    */

		    bool tooBig = false;
		    if (panelItem->boardSizeInches.width() >= panelParams.panelWidth) {
			    tooBig = panelItem->boardSizeInches.width() >= panelParams.panelHeight;
			    if (!tooBig) {
				    tooBig = panelItem->boardSizeInches.height() >= panelParams.panelWidth;
			    }
		    }

		    if (!tooBig) {
			    if (panelItem->boardSizeInches.height() >= panelParams.panelHeight) {
				    tooBig = panelItem->boardSizeInches.height() >= panelParams.panelWidth;
				    if (!tooBig) {
					    tooBig = panelItem->boardSizeInches.width() >= panelParams.panelHeight;
				    }
			    }
		    }

		    if (tooBig) {
			    DebugDialog::debug(QString("board is too big for panel '%1'").arg(path));
			    return false;
		    }


            makeSVGs(mainWindow, boardItem, boardName, layerThingList, norotateDir);

		    refPanelItems << panelItem;
        }

        // now save the rotated version
		mainWindow->pcbView()->selectAllItems(true, false);
	    QMatrix matrix;
		mainWindow->pcbView()->rotateX(90, false);

        foreach (ItemBase * boardItem, boards) {
            makeSVGs(mainWindow, boardItem, boardName, layerThingList, rotateDir);
        }

		mainWindow->setCloseSilently(true);
        mainWindow->close();
        delete mainWindow,

		boardElement = boardElement.nextSiblingElement("board");
	}

	return true;
}

bool Panelizer::initPanelParams(QDomElement & root, PanelParams & panelParams)
{
	panelParams.outputFolder = root.attribute("outputFolder");
	QDir outputDir(panelParams.outputFolder);
	if (!outputDir.exists()) {
		DebugDialog::debug(QString("Output folder '%1' doesn't exist").arg(panelParams.outputFolder));
		return false;
	}

	panelParams.prefix = root.attribute("prefix");
	if (panelParams.prefix.isEmpty()) {
		DebugDialog::debug(QString("Output file prefix not specified"));
		return false;
	}

	bool ok;
	panelParams.panelWidth = TextUtils::convertToInches(root.attribute("width"), &ok, false);
	if (!ok) {
		DebugDialog::debug(QString("Can't parse panel width '%1'").arg(root.attribute("width")));
		return false;
	}

	panelParams.panelHeight = TextUtils::convertToInches(root.attribute("height"), &ok, false);
	if (!ok) {
		DebugDialog::debug(QString("Can't parse panel height '%1'").arg(root.attribute("height")));
		return false;
	}

	panelParams.panelSmallWidth = TextUtils::convertToInches(root.attribute("small-width"), &ok, false);
	if (!ok) {
		DebugDialog::debug(QString("Can't parse panel small-width '%1'").arg(root.attribute("small-width")));
		return false;
	}

	panelParams.panelSmallHeight = TextUtils::convertToInches(root.attribute("small-height"), &ok, false);
	if (!ok) {
		DebugDialog::debug(QString("Can't parse panel small-height '%1'").arg(root.attribute("small-height")));
		return false;
	}

	panelParams.panelSpacing = TextUtils::convertToInches(root.attribute("spacing"), &ok, false);
	if (!ok) {
		DebugDialog::debug(QString("Can't parse panel spacing '%1'").arg(root.attribute("spacing")));
		return false;
	}

	panelParams.panelBorder = TextUtils::convertToInches(root.attribute("border"), &ok, false);
	if (!ok) {
		DebugDialog::debug(QString("Can't parse panel border '%1'").arg(root.attribute("border")));
		return false;
	}

	return true;

}

int Panelizer::placeBestFit(Tile * tile, UserData userData) {
	if (TiGetType(tile) != Tile::SPACE) return 0;

	BestPlace * bestPlace = (BestPlace *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);
	int w = tileRect.xmaxi - tileRect.xmini;
	int h = tileRect.ymaxi - tileRect.ymini;
	if (bestPlace->width > w && bestPlace->height > w) {
		return 0;
	}

	int fitCount = 0;
	bool fit[4];
	double area[4];
	for (int i = 0; i < 4; i++) {
		fit[i] = false;
		area[i] = Worst;
	}

	if (w >= bestPlace->width && h >= bestPlace->height) {
		fit[0] = true;
		area[0] = (w * h) - (bestPlace->width * bestPlace->height);
		fitCount++;
	}
	if (h >= bestPlace->width && w >= bestPlace->height) {
		fit[1] = true;
		area[1] = (w * h) - (bestPlace->width * bestPlace->height);
		fitCount++;
	}

	if (!fit[0] && w >= bestPlace->width) {
		// see if adjacent tiles below are open
		TileRect temp;
		temp.xmini = tileRect.xmini;
		temp.xmaxi = temp.xmini + bestPlace->width;
		temp.ymini = tileRect.ymini;
		temp.ymaxi = temp.ymini + bestPlace->height;
		if (temp.ymaxi < bestPlace->maxRect.ymaxi) {
			QList<Tile*> spaces;
			TiSrArea(tile, bestPlace->plane, &temp, allSpaces, &spaces);
			if (spaces.count()) {
				int y = temp.ymaxi;
				foreach (Tile * t, spaces) {
					if (YMAX(t) > y) y = YMAX(t);			// find the bottom of the lowest open tile
				}
				fit[2] = true;
				fitCount++;
				area[2] = (w * (y - temp.ymini)) - (bestPlace->width * bestPlace->height);
			}
		}
	}

	if (!fit[1] && w >= bestPlace->height) {
		// see if adjacent tiles below are open
		TileRect temp;
		temp.xmini = tileRect.xmini;
		temp.xmaxi = temp.xmini + bestPlace->height;
		temp.ymini = tileRect.ymini;
		temp.ymaxi = temp.ymini + bestPlace->width;
		if (temp.ymaxi < bestPlace->maxRect.ymaxi) {
			QList<Tile*> spaces;
			TiSrArea(tile, bestPlace->plane, &temp, allSpaces, &spaces);
			if (spaces.count()) {
				int y = temp.ymaxi;
				foreach (Tile * t, spaces) {
					if (YMAX(t) > y) y = YMAX(t);			// find the bottom of the lowest open tile
				}
				fit[3] = true;
				fitCount++;
				area[3] = (w * (y - temp.ymini)) - (bestPlace->width * bestPlace->height);
			}
		}
	}

	if (fitCount == 0) return 0;

	// area is white space remaining after board has been inserteds
	
	int result = -1;
	for (int i = 0; i < 4; i++) {
		if (area[i] < bestPlace->bestArea) {
			result = i;
			break;
		}
	}
	if (result < 0) return 0;			// current bestArea is better

	bestPlace->bestTile = tile;
	bestPlace->bestTileRect = tileRect;
	if (fitCount == 1 || (bestPlace->width == bestPlace->height)) {
		if (fit[0] || fit[2]) {
			bestPlace->rotate90 = false;
		}
		else {
			bestPlace->rotate90 = true;
		}
		bestPlace->bestArea = area[result];
		return 0;
	}

	if (TiGetType(BL(tile)) == Tile::BUFFER) {
		// this is a leftmost tile
		// select for creating the biggest area after putting in the tile;
		double a1 = (w - bestPlace->width) * (bestPlace->height);
		double a2 = (h - bestPlace->height) * w;
		double a = qMax(a1, a2);
		double b1 = (w - bestPlace->height) * (bestPlace->width);
		double b2 = (h - bestPlace->width) * w;
		double b = qMax(b1, b2);
		bestPlace->rotate90 = (a < b);
		if (bestPlace->rotate90) {
			bestPlace->bestArea = fit[1] ? area[1] : area[3];
		}
		else {
			bestPlace->bestArea = fit[0] ? area[0] : area[2];
		}

		return 0;
	}

	TileRect temp;
	temp.xmini = bestPlace->maxRect.xmini;
	temp.xmaxi = tileRect.xmini - 1;
	temp.ymini = tileRect.ymini;
	temp.ymaxi = tileRect.ymaxi;
	QList<Tile*> obstacles;
	TiSrArea(tile, bestPlace->plane, &temp, allObstacles, &obstacles);
	int maxBottom = 0;
	foreach (Tile * obstacle, obstacles) {
		if (YMAX(obstacle) > maxBottom) maxBottom = YMAX(obstacle);
	}

	if (tileRect.ymini + bestPlace->width <= maxBottom && tileRect.ymini + bestPlace->height <= maxBottom) {
		// use the max length 
		if (bestPlace->width >= bestPlace->height) {
			bestPlace->rotate90 = true;
			bestPlace->bestArea = fit[1] ? area[1] : area[3];
		}
		else {
			bestPlace->rotate90 = false;
			bestPlace->bestArea = fit[0] ? area[0] : area[2];
		}

		return 0;
	}

	if (tileRect.ymini + bestPlace->width > maxBottom && tileRect.ymini + bestPlace->height > maxBottom) {
		// use the min length
		if (bestPlace->width <= bestPlace->height) {
			bestPlace->rotate90 = true;
			bestPlace->bestArea = fit[1] ? area[1] : area[3];
		}
		else {
			bestPlace->rotate90 = false;
			bestPlace->bestArea = fit[0] ? area[0] : area[2];
		}

		return 0;
	}

	if (tileRect.ymini + bestPlace->width <= maxBottom) {
		bestPlace->rotate90 = true;
		bestPlace->bestArea = fit[1] ? area[1] : area[3];
		return 0;
	}

	bestPlace->rotate90 = false;
	bestPlace->bestArea = fit[0] ? area[0] : area[2];
	return 0;
}

void Panelizer::addOptional(int optionalCount, QList<PanelItem *> & refPanelItems, QList<PanelItem *> & insertPanelItems, PanelParams & panelParams, QList<PlanePair *> & planePairs)
{
	while (optionalCount > 0) {
		int ix = qFloor(qrand() * optionalCount / (double) RAND_MAX);
		int soFar = 0;
		foreach (PanelItem * panelItem, refPanelItems) {
			if (panelItem->maxOptional == 0) continue;

			if (ix >= soFar && ix < soFar + panelItem->maxOptional) {
				PanelItem * copy = new PanelItem(panelItem);
				if (bestFitOne(copy, panelParams, planePairs, false)) {
					// got one
					panelItem->maxOptional--;
					optionalCount--;
					insertPanelItems.append(copy);
				}
				else {
					// don't bother trying this one again
					optionalCount -= panelItem->maxOptional;
					panelItem->maxOptional = 0;
				}
				break;
			}

			soFar += panelItem->maxOptional;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////

void Panelizer::inscribe(FApplication * app, const QString & panelFilename) 
{
	QFile file(panelFilename);

	QString errorStr;
	int errorLine;
	int errorColumn;

	DebugDialog::setEnabled(true);

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("Unable to parse '%1': '%2' line:%3 column:%4").arg(panelFilename).arg(errorStr).arg(errorLine).arg(errorColumn));
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull() || root.tagName() != "panelizer") {
		DebugDialog::debug(QString("root element is not 'panelizer'"));
		return;
	}

	PanelParams panelParams;
	if (!initPanelParams(root, panelParams)) return;

	QDir outputDir = QDir::temp();
	QDir fzDir(outputDir);
	fzDir.cd("fz");
	if (!fzDir.exists()) {
		DebugDialog::debug(QString("unable to create fz folder in '%1'").arg(panelParams.outputFolder));
		return;
	}

	DebugDialog::debug(QString("fz folder '%1'\n").arg(fzDir.absolutePath()));


	QDomElement boards = root.firstChildElement("boards");
	QDomElement board = boards.firstChildElement("board");
	if (board.isNull()) {
		DebugDialog::debug(QString("no <board> elements found"));
		return;
	}

	QHash<QString, QString> fzzFilePaths;
	QDomElement paths = root.firstChildElement("paths");
	QDomElement path = paths.firstChildElement("path");
	if (path.isNull()) {
		DebugDialog::debug(QString("no <path> elements found"));
		return;
	}

	collectFiles(path, fzzFilePaths);
	if (fzzFilePaths.count() == 0) {
		DebugDialog::debug(QString("no fzz files found in paths"));
		return;
	}
	
	board = boards.firstChildElement("board");
	if (!checkBoards(board, fzzFilePaths)) return;

	app->createUserDataStoreFolderStructure();
	app->registerFonts();
	app->loadReferenceModel("", false);

	board = boards.firstChildElement("board");
	while (!board.isNull()) {
		MainWindow * mainWindow = inscribeBoard(board, fzzFilePaths, app, fzDir);
		if (mainWindow) {
			mainWindow->setCloseSilently(true);
			mainWindow->close();
            delete mainWindow;
		}
		board = board.nextSiblingElement("board");
	}

	// TODO: delete temp fz folder

}

MainWindow * Panelizer::inscribeBoard(QDomElement & board, QHash<QString, QString> & fzzFilePaths, FApplication * app, QDir & fzDir)
{
	QString boardName = board.attribute("name");
	int optional = board.attribute("maxOptionalCount", "").toInt();
	int required = board.attribute("requiredCount", "").toInt();
    if (optional <= 0 && required <= 0) return NULL;

	QString path = fzzFilePaths.value(boardName, "");

	int loaded = 0;
	MainWindow * mainWindow = app->loadWindows(loaded, false);
	mainWindow->noBackup();

	FolderUtils::setOpenSaveFolderAux(fzDir.absolutePath());

	if (!mainWindow->loadWhich(path, false, false, "")) {
		DebugDialog::debug(QString("failed to load '%1'").arg(path));
		return mainWindow;
	}

	mainWindow->showPCBView();

    int moved = mainWindow->pcbView()->checkLoadedTraces();
    if (moved > 0) {
        QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("%1 wires moved from their saved position in %2.").arg(moved).arg(path));
    }


	foreach (QGraphicsItem * item, mainWindow->pcbView()->scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		itemBase->setMoveLock(false);
	}

	QString fritzingVersion = mainWindow->fritzingVersion();
	VersionThing versionThing;
	versionThing.majorVersion = 0;
	versionThing.minorVersion = 7;
	versionThing.minorSubVersion = 0;
	versionThing.releaseModifier = "";

	VersionThing versionThingFz;
	Version::toVersionThing(fritzingVersion, versionThingFz);
	bool oldGround = !Version::greaterThan(versionThing, versionThingFz);
		
    bool filled = false;
    QList<ItemBase *> boards = mainWindow->pcbView()->findBoard();	
    bool wasOne = false;
	foreach (ItemBase * boardItem, boards) {
        mainWindow->pcbView()->selectAllItems(false, false);
        boardItem->setSelected(true);
	    if (boardItem->prop("layers").compare("1") == 0) {
            mainWindow->swapLayers(boardItem, 2, "", false, 0);
            ProcessEventBlocker::processEvents();
            wasOne = true;
        }

        LayerList groundLayers;
        groundLayers << ViewLayer::GroundPlane0 << ViewLayer::GroundPlane1;
        foreach (ViewLayer::ViewLayerID viewLayerID, groundLayers) {
            QString fillType = mainWindow->pcbView()->characterizeGroundFill(viewLayerID);
	        if (fillType == GroundPlane::fillTypeNone) {
		        mainWindow->copperFill(viewLayerID);
                ProcessEventBlocker::processEvents();
		        filled = true;
	        }
	        else if ((fillType == GroundPlane::fillTypeGround) && oldGround) {
		        mainWindow->groundFill(viewLayerID);
                ProcessEventBlocker::processEvents();
		        filled = true;
	        }
        }
    }

	if (filled) { 
		mainWindow->saveAsShareable(path, true);
		DebugDialog::debug(QString("%1 filled:%2").arg(path).arg(filled));
	}

	return mainWindow;
}

void Panelizer::makeSVGs(MainWindow * mainWindow, ItemBase * board, const QString & boardName, QList<LayerThing> & layerThingList, QDir & saveDir) {
	try {
		
		QSizeF imageSize;
		bool empty;

		QString maskTop;
		QString maskBottom;

		foreach (LayerThing layerThing, layerThingList) {					
			SVG2gerber::ForWhy forWhy = layerThing.forWhy;
			QString name = layerThing.name;
			QList<ItemBase *> copperLogoItems, holes;
            switch (forWhy) {
                case SVG2gerber::ForPasteMask:
                    mainWindow->pcbView()->hideHoles(holes);
                case SVG2gerber::ForMask:
				    mainWindow->pcbView()->hideCopperLogoItems(copperLogoItems);
                default:
                    break;
			}
			QString one = mainWindow->pcbView()->renderToSVG(GraphicsUtils::SVGDPI, layerThing.layerList, true, imageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, empty);
					
			QString clipString;
					
			switch (forWhy) {
				case SVG2gerber::ForOutline:
					one = GerberGenerator::cleanOutline(one);
					break;
				case SVG2gerber::ForPasteMask:
					mainWindow->pcbView()->restoreCopperLogoItems(copperLogoItems);
					mainWindow->pcbView()->restoreCopperLogoItems(holes);
                    one = mainWindow->pcbView()->makePasteMask(one, board, GraphicsUtils::StandardFritzingDPI, layerThing.layerList);
                    if (one.isEmpty()) continue;

					forWhy = SVG2gerber::ForCopper;
                    break;
				case SVG2gerber::ForMask:
					mainWindow->pcbView()->restoreCopperLogoItems(copperLogoItems);
					one = TextUtils::expandAndFill(one, "black", GerberGenerator::MaskClearanceMils * 2);
					forWhy = SVG2gerber::ForCopper;
					if (name.contains("bottom")) {
						maskBottom = one;
					}
					else {
						maskTop = one;
					}
					break;
				case SVG2gerber::ForSilk:
					if (name.contains("bottom")) {
						clipString = maskBottom;
					}
					else {
						clipString = maskTop;
					}
					break;
				default:
					break;
			}
					
			one = GerberGenerator::clipToBoard(one, board, name, forWhy, clipString);
			if (one.isEmpty()) continue;

            QString filename = saveDir.absoluteFilePath(QString("%1_%2_%3.svg").arg(boardName).arg(board->id()).arg(name));
            TextUtils::writeUtf8(filename, one);
		}
	}
	catch (const char * msg) {
		DebugDialog::debug(QString("panelizer error 1 %1 %2").arg(boardName).arg(msg));
	}
	catch (const QString & msg) {
		DebugDialog::debug(QString("panelizer error 2 %1 %2").arg(boardName).arg(msg));
	}
	catch (...) {
		DebugDialog::debug(QString("panelizer error 3 %1").arg(boardName));
	}
}

void Panelizer::doOnePanelItem(PlanePair * planePair, QList<LayerThing> & layerThingList, PanelItem * panelItem, QDir & svgDir) {
	try {
		
		for (int i = 0; i < planePair->svgs.count(); i++) {					
			QString name = layerThingList.at(i).name;

            QString rot = panelItem->rotate90 ? "rotate" : "norotate";
            QString filename = svgDir.absoluteFilePath(QString("%1/%2_%3_%4.svg").arg(rot).arg(panelItem->boardName).arg(panelItem->boardID).arg(name));
            QFile file(filename);
            if (file.open(QFile::ReadOnly)) {		
			    QString one = file.readAll();
			    if (one.isEmpty()) continue;

			    int left = one.indexOf("<svg");
			    left = one.indexOf(">", left + 1);
			    int right = one.lastIndexOf("<");
			    one = QString("<g transform='translate(%1,%2)'>\n").arg(panelItem->x * GraphicsUtils::StandardFritzingDPI).arg(panelItem->y * GraphicsUtils::StandardFritzingDPI) + 
							    one.mid(left + 1, right - left - 1) + 
							    "</g>\n";

			    planePair->svgs.replace(i, planePair->svgs.at(i) + one);
            }
            else {
                DebugDialog::debug(QString("panelizer error? 1 %1 %2").arg(filename).arg("one text not found"));
            }
		}
	}
	catch (const char * msg) {
		DebugDialog::debug(QString("panelizer error 1 %1 %2").arg(panelItem->boardName).arg(msg));
	}
	catch (const QString & msg) {
		DebugDialog::debug(QString("panelizer error 2 %1 %2").arg(panelItem->boardName).arg(msg));
	}
	catch (...) {
		DebugDialog::debug(QString("panelizer error 3 %1").arg(panelItem->boardName));
	}
}

void Panelizer::shrinkLastPanel( QList<PlanePair *> & planePairs, QList<PanelItem *> & insertPanelItems, PanelParams & panelParams) 
{
    PlanePair * lastPlanePair = planePairs.last();
    PlanePair * smallPlanePair = makePlanePair(panelParams, false);
    smallPlanePair->index = lastPlanePair->index;
    QList<PlanePair *> smallPlanePairs;
    smallPlanePairs << smallPlanePair;
    bool canFitSmaller = true;
    QList<PanelItem *> smallPanelItems;
    foreach (PanelItem * panelItem, insertPanelItems) {
        if (panelItem->planePair != lastPlanePair) continue;

        PanelItem * copy = new PanelItem(panelItem);
        smallPanelItems << copy;

		if (!bestFitOne(copy, panelParams, smallPlanePairs, false)) {
            canFitSmaller = false;
        }
	}

    if (canFitSmaller) {
        foreach (PanelItem * panelItem, insertPanelItems) {
            if (panelItem->planePair != lastPlanePair) continue;

            insertPanelItems.removeOne(panelItem);
            delete panelItem;
        }
        planePairs.removeOne(lastPlanePair);
        delete lastPlanePair;
        insertPanelItems.append(smallPanelItems);
        planePairs.append(smallPlanePair);      
    }
    else {
        foreach (PanelItem * copy, smallPanelItems) {
            delete copy;
        }
        delete smallPlanePair;
    }

}