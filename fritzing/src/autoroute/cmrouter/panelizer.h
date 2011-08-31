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

#ifndef PANELIZER_H
#define PANELIZER_H

#include <QString>
#include <QSizeF>

#include "../../mainwindow.h"
#include "../../items/itembase.h"
#include "tile.h"

struct PanelItem {
	QString boardName;
	QString path;
	MainWindow * window;
	int required;
	int maxOptional;
	QSizeF boardSizeInches;
	ItemBase * board;
	double x, y;
	bool rotate90;

	PanelItem() {
	}

	PanelItem(PanelItem * from) {
		this->boardName = from->boardName ;
		this->path = from->path;
		this->window = from->window;
		this->required = from->required;
		this->maxOptional = from->maxOptional;
		this->boardSizeInches = from->boardSizeInches;
		this->board = from->board;
	}
};

struct BestPlace
{
	Tile * bestTile;
	TileRect bestTileRect;
	int width;
	int height;
	bool rotate90;
};

struct PlanePair
{
	Plane * thePlane;
	Plane * thePlane90;
	TileRect tilePanelRect;
	TileRect tilePanelRect90;
};

struct PanelParams
{
	double panelWidth;
	double panelHeight;
	double panelSpacing;
	double panelBorder;
	QString prefix;
	QString outputFolder;
};

class Panelizer
{
public:
	static void panelize(class FApplication *, const QString & panelFilename);
	static int placeBestFit(Tile * tile, UserData userData);

protected:
	static bool initPanelParams(QDomElement & root, PanelParams & panelParams);
	static void makePlanePair(PanelParams &, PlanePair &);
	static void collectFiles(QDomElement & path, QHash<QString, QString> & fzzFilePaths);
	static bool checkBoards(QDomElement & board, QHash<QString, QString> & fzzFilePaths);
	static bool openWindows(QDomElement & board, QHash<QString, QString> & fzzFilePaths, class FApplication *, const QString & outputFolder, QHash<QString, PanelItem *> & refPanelItems);
	static void bestFit(QList<PanelItem *> insertPanelItems, PanelParams & panelParams, PlanePair & planePair, QString & svg);
};

#endif
