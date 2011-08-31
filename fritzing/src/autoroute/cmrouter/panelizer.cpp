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

$Revision: 2979 $:
$Author$:
$Date$

********************************************************************/

#include "panelizer.h"
#include "../../debugdialog.h"
#include "../../sketch/pcbsketchwidget.h"
#include "../../utils/textutils.h"
#include "../../utils/folderutils.h"
#include "../../utils/folderutils.h"
#include "../../items/resizableboard.h"
#include "../../fsvgrenderer.h"
#include "../../fapplication.h"
#include "tileutils.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDir>

bool areaGreaterThan(PanelItem * p1, PanelItem * p2)
{
	return p1->boardSizeInches.width() * p1->boardSizeInches.height() > p2->boardSizeInches.width() * p2->boardSizeInches.height();
}

void checkRotate(BestPlace * bestPlace, bool normalFit, bool rotateFit, int w, int h) {
	if (!normalFit) {
		bestPlace->rotate90 = true;
		return;
	}
	
	if (!rotateFit) {
		bestPlace->rotate90 = false;
		return;
	}

	double a1 = (w - bestPlace->width) * (bestPlace->height);
	double a2 = (h - bestPlace->height) * w;
	double a = qMax(a1, a2);
	double b1 = (w - bestPlace->height) * (bestPlace->width);
	double b2 = (h - bestPlace->width) * w;
	double b = qMax(b1, b2);
	bestPlace->rotate90 = (a < b);
}

/////////////////////////////////////////////////////////////////////////////////

void Panelizer::panelize(FApplication * app, const QString & panelFilename) 
{
	QFile file(panelFilename);

	QString errorStr;
	int errorLine;
	int errorColumn;

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
	app->loadReferenceModel();
	if (!app->loadBin("")) {
		DebugDialog::debug(QString("load bin failed"));
		return;
	}

	QHash<QString, PanelItem *> refPanelItems;
	board = boards.firstChildElement("board");
	if (!openWindows(board, fzzFilePaths, app, panelParams.outputFolder, refPanelItems)) return;

	// use the total area to initially allocate a set of PlanePairs
	QList<PanelItem *> insertPanelItems;
	foreach (PanelItem * panelItem, refPanelItems.values()) {
		for (int i = 0; i < panelItem->required; i++) {
			PanelItem * copy = new PanelItem(panelItem);
			insertPanelItems.append(copy);
		}
	}

	PlanePair planePair;
	makePlanePair(panelParams, planePair);

	// this svg just for debugging
	QString svg = TextUtils::makeSVGHeader(1, 1000, panelParams.panelWidth, panelParams.panelHeight);

	qSort(insertPanelItems.begin(), insertPanelItems.end(), areaGreaterThan);

	bestFit(insertPanelItems, panelParams, planePair, svg);

	// deal with optional pool

	svg += "</svg>";

	QFile outfile("panelizer.svg");
	if (outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&outfile);
		out << svg;
		outfile.close();
	}


	QList<LayerList> layerList;
	layerList << ViewLayer::silkLayers(ViewLayer::Top)
		<< ViewLayer::silkLayers(ViewLayer::Bottom)
		<< ViewLayer::copperLayers(ViewLayer::Top)
		<< ViewLayer::copperLayers(ViewLayer::Bottom)
		<< ViewLayer::maskLayers(ViewLayer::Top)
		<< ViewLayer::maskLayers(ViewLayer::Bottom)
		<< ViewLayer::outlineLayers();

	QStringList svgs;
	for (int i = 0; i < layerList.count(); i++) {
		svgs << TextUtils::makeSVGHeader(1, 1000, panelParams.panelWidth, panelParams.panelHeight);
	}

	foreach (PanelItem * panelItem, insertPanelItems) {
		QRectF offsetRect;
		
		if (panelItem->rotate90) {
			panelItem->window->pcbView()->scene()->clearSelection();
			panelItem->board->setSelected(true);
			panelItem->window->pcbView()->rotateX(90, false);
		}

		offsetRect = panelItem->board->sceneBoundingRect();
		offsetRect.moveTo(offsetRect.left() - (panelItem->x * FSvgRenderer::printerScale()), offsetRect.top() - (panelItem->y * FSvgRenderer::printerScale()));

		QSizeF imageSize;
		bool empty;

		for (int i = 0; i < svgs.count(); i++) {
			QString one = panelItem->window->pcbView()->renderToSVG(FSvgRenderer::printerScale(), layerList.at(i), layerList.at(i), true, imageSize, offsetRect, 1000, false, false, false, empty);
			int left = one.indexOf("<svg");
			left = one.indexOf(">", left + 1);
			int right = one.lastIndexOf("<");
			svgs.replace(i, svgs.at(i) + one.mid(left + 1, right - left - 1));
		}

		if (panelItem->rotate90) {
			panelItem->window->pcbView()->rotateX(-90, false);
		}

	}

	for (int i = 0; i < svgs.count(); i++) {
		QFile outfile(QString("panelizer%1.svg").arg(i));
		if (outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&outfile);
			out << svgs.at(i);
			out << "</svg>";
			outfile.close();
		}
	}
}

void Panelizer::bestFit(QList<PanelItem *> insertPanelItems, PanelParams & panelParams, PlanePair & planePair, QString & svg)
{
	foreach (PanelItem * panelItem, insertPanelItems) {
		BestPlace bestPlace;
		bestPlace.bestTile = NULL;
		bestPlace.rotate90 = false;
		bestPlace.width = realToTile(panelItem->boardSizeInches.width() + (panelParams.panelSpacing / 2));
		bestPlace.height = realToTile(panelItem->boardSizeInches.height() + (panelParams.panelSpacing / 2));
		TiSrArea(NULL, planePair.thePlane, &planePair.tilePanelRect, placeBestFit, &bestPlace);
		if (bestPlace.bestTile == NULL) {
			TiSrArea(NULL, planePair.thePlane90, &planePair.tilePanelRect90, placeBestFit, &bestPlace);
			if (bestPlace.bestTile == NULL) {
				DebugDialog::debug(QString("ran out of room placing %1").arg(panelItem->boardName));
				svg += QString("<!-- unable to place %1 -->\n").arg(panelItem->boardName);

				// TODO: check next plane
				continue;
			}

			TileRect tileRect;
			tileUnrotate90(bestPlace.bestTileRect, tileRect);
			bestPlace.bestTileRect = tileRect;
			bestPlace.rotate90 = !bestPlace.rotate90;
		}

		panelItem->x = tileToReal(bestPlace.bestTileRect.xmini);
		panelItem->y = tileToReal(bestPlace.bestTileRect.ymini);
		panelItem->rotate90 = bestPlace.rotate90;

		TileRect tileRect;
		tileRect.xmini = bestPlace.bestTileRect.xmini;
		tileRect.ymini = bestPlace.bestTileRect.ymini;
		if (bestPlace.rotate90) {
			tileRect.xmaxi = tileRect.xmini + bestPlace.height;
			tileRect.ymaxi = tileRect.ymini + bestPlace.width;
		}
		else {
			tileRect.ymaxi = tileRect.ymini + bestPlace.height;
			tileRect.xmaxi = tileRect.xmini + bestPlace.width;
		}

		qreal w = panelItem->boardSizeInches.width();
		qreal h = panelItem->boardSizeInches.height();
		if (panelItem->rotate90) {
			w = h;
			h = panelItem->boardSizeInches.width();
		}

		svg += QString("<rect x='%1' y='%2' width='%3' height='%4' stroke='none' fill='red'/>\n")
			.arg(panelItem->x * 1000)
			.arg(panelItem->y * 1000)
			.arg(1000 * w)
			.arg(1000 * h);
		svg += QString("<text x='%1' y='%2' anchor='middle' font-family='DroidSans' stroke='none' fill='#000000' text-anchor='middle' font-size='85'>%3</text>\n")
			.arg(1000 * (panelItem->x + (w / 2)))
			.arg(1000 * (panelItem->y + (h  / 2)))
			.arg(QFileInfo(panelItem->path).completeBaseName());


		TiInsertTile(planePair.thePlane, &tileRect, NULL, Tile::OBSTACLE);
		TileRect tileRect90;
		tileRotate90(tileRect, tileRect90);
		TiInsertTile(planePair.thePlane90, &tileRect90, NULL, Tile::OBSTACLE);
	}
}


void Panelizer::makePlanePair(PanelParams & panelParams, PlanePair & planePair)
{
	Tile * bufferTile = TiAlloc();
	TiSetType(bufferTile, Tile::BUFFER);
	TiSetBody(bufferTile, NULL);

	QRectF panelRect(0, 0, panelParams.panelWidth + (panelParams.panelSpacing /2) - panelParams.panelBorder, 
							panelParams.panelHeight + (panelParams.panelSpacing / 2) - panelParams.panelBorder);

    SETLEFT(bufferTile, fasterRealToTile(panelRect.left() - 10));
    SETYMIN(bufferTile, fasterRealToTile(panelRect.top() - 10));		

	planePair.thePlane = TiNewPlane(bufferTile);

    SETRIGHT(bufferTile, fasterRealToTile(panelRect.right() + 10));
	SETYMAX(bufferTile, fasterRealToTile(panelRect.bottom() + 10));		

	qrectToTile(panelRect, planePair.tilePanelRect);
	TiInsertTile(planePair.thePlane, &planePair.tilePanelRect, NULL, Tile::SPACE); 

	QMatrix matrix90;
	matrix90.rotate(90);
	QRectF panelRect90 = matrix90.mapRect(panelRect);

	Tile * bufferTile90 = TiAlloc();
	TiSetType(bufferTile90, Tile::BUFFER);
	TiSetBody(bufferTile90, NULL);

    SETLEFT(bufferTile90, fasterRealToTile(panelRect90.left() - 10));
    SETYMIN(bufferTile90, fasterRealToTile(panelRect90.top() - 10));		

	planePair.thePlane90 = TiNewPlane(bufferTile90);

    SETRIGHT(bufferTile90, fasterRealToTile(panelRect.right() + 10));
	SETYMAX(bufferTile90, fasterRealToTile(panelRect.bottom() + 10));		

	qrectToTile(panelRect90, planePair.tilePanelRect90);
	TiInsertTile(planePair.thePlane90, &planePair.tilePanelRect90, NULL, Tile::SPACE); 
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
		FolderUtils::collectFiles(dir, QStringList("*" + FritzingBundleExtension), filepaths);
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
		bool ok;
		int test = board.attribute("requiredCount", "").toInt(&ok);
		if (!ok) {
			DebugDialog::debug(QString("requiredCount for board '%1' not an integer: '%2'").arg(boardname).arg(board.attribute("requiredCount")));
			return false;
		}

		test = board.attribute("maxOptionalCount", "").toInt(&ok);
		if (!ok) {
			DebugDialog::debug(QString("maxOptionalCount for board '%1' not an integer: '%2'").arg(boardname).arg(board.attribute("maxOptionalCount")));
			return false;
		}

		QString path = fzzFilePaths.value(boardname, "");
		if (path.isEmpty()) {
			DebugDialog::debug(QString("File for board '%1' not found in search paths").arg(boardname));
			return false;
		}

		board = board.nextSiblingElement("board");
	}

	return true;
}

bool Panelizer::openWindows(QDomElement & board, QHash<QString, QString> & fzzFilePaths, FApplication * app, const QString & outputFolder, QHash<QString, PanelItem *> & refPanelItems)
{
	while (!board.isNull()) {
		QString boardName = board.attribute("name");
		QString path = fzzFilePaths.value(boardName, "");
		int loaded = 0;
		MainWindow * mainWindow = app->loadWindows(loaded);

		FolderUtils::setOpenSaveFolderAux(outputFolder);
		if (!mainWindow->loadWhich(path, false, false, true)) {
			DebugDialog::debug(QString("failed to load '%1'").arg(path));
			return false;
		}

		mainWindow->showPCBView();
		
		ItemBase * boardItem = mainWindow->pcbView()->findBoard();
		if (boardItem == NULL) {
			DebugDialog::debug(QString("no board found in '%1'").arg(path));
			return false;
		}

		PanelItem * panelItem = new PanelItem;
		panelItem->boardName = boardName;
		panelItem->window = mainWindow;
		panelItem->path = path;
		panelItem->board = boardItem;
		panelItem->required = board.attribute("requiredCount", "").toInt();
		panelItem->maxOptional = board.attribute("maxOptionalCount", "").toInt();

		QSizeF boardSize = boardItem->size();
		ResizableBoard * resizableBoard = qobject_cast<ResizableBoard *>(boardItem->layerKinChief());
		if (resizableBoard != NULL) {
			panelItem->boardSizeInches = resizableBoard->getSizeMM() / 25.4;
		}
		else {
			panelItem->boardSizeInches = boardSize / FSvgRenderer::printerScale();
		}

		refPanelItems.insert(boardName, panelItem);
		mainWindow->setCloseSilently(true);

		board = board.nextSiblingElement("board");
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
	bool normalFit = false;
	bool rotateFit = false;
	if (w >= bestPlace->width && h >= bestPlace->height) {
		normalFit = true;
	}
	if (h >= bestPlace->width && w >= bestPlace->height) {
		rotateFit = true;
	}
	
	if (!(normalFit || rotateFit)) return 0;

	if (bestPlace->bestTile == NULL) {
		bestPlace->bestTile = tile;
		bestPlace->bestTileRect = tileRect;
		checkRotate(bestPlace, normalFit, rotateFit, w, h);
		return 0;
	}

	double bestArea = ((double) (bestPlace->bestTileRect.xmaxi - bestPlace->bestTileRect.xmini)) * (bestPlace->bestTileRect.ymaxi - bestPlace->bestTileRect.ymini);
	double area =  (double) w * h;
	if (area < bestArea) {
		bestPlace->bestTile = tile;
		bestPlace->bestTileRect = tileRect;
		checkRotate(bestPlace, normalFit, rotateFit, w, h);
	}

	return 0;
}

