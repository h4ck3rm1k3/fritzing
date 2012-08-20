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

#ifndef PANELIZER_H
#define PANELIZER_H

#include <QString>
#include <QSizeF>

#include "../../mainwindow.h"
#include "../../items/itembase.h"
#include "tile.h"


struct PlanePair
{
	Plane * thePlane;
	Plane * thePlane90;
	TileRect tilePanelRect;
	TileRect tilePanelRect90;
    double panelWidth;
    double panelHeight;
	QStringList svgs;
	QString layoutSVG;
	int index;
};

struct PanelItem {
	// per window
	QString boardName;
	QString path;
	int required;
	int maxOptional;
	QSizeF boardSizeInches;
    long boardID;

	// per instance
	double x, y;
	bool rotate90;
	PlanePair * planePair;

	PanelItem() {}

	PanelItem(PanelItem * from); 
};

struct BestPlace
{
	Tile * bestTile;
	TileRect bestTileRect;
	TileRect maxRect;
	int width;
	int height;
	double bestArea;
	bool rotate90;
	Plane* plane;

    BestPlace();
};

struct PanelParams
{
	double panelWidth;
	double panelHeight;
	double panelSmallWidth;
	double panelSmallHeight;
	double panelSpacing;
	double panelBorder;
	QString prefix;
	QString outputFolder;
};

struct LayerThing {
    LayerList layerList;
    QString name;
    SVG2gerber::ForWhy forWhy;
    QString suffix;

    LayerThing(const QString & n, LayerList ll, SVG2gerber::ForWhy fw, const QString & s) {
            layerList = ll;
            name = n;
            forWhy = fw;
            suffix = s;
    }
};

class Panelizer
{
public:
	static void panelize(class FApplication *, const QString & panelFilename, bool customPartsOnly);
	static void inscribe(class FApplication *, const QString & panelFilename);
	static int placeBestFit(Tile * tile, UserData userData);

protected:
	static bool initPanelParams(QDomElement & root, PanelParams &);
	static PlanePair * makePlanePair(PanelParams &, bool big);
	static void collectFiles(QDomElement & path, QHash<QString, QString> & fzzFilePaths);
	static bool checkBoards(QDomElement & board, QHash<QString, QString> & fzzFilePaths);
	static bool openWindows(QDomElement & board, QHash<QString, QString> & fzzFilePaths, class FApplication *, PanelParams &, QDir & fzDir, QDir & svgDir, QList<PanelItem *> & refPanelItems, QList<LayerThing> & layerThingList, bool customPartsOnly);
	static void bestFit(QList<PanelItem *> & insertPanelItems, PanelParams &, QList<PlanePair *> &, bool customPartsOnly);
	static bool bestFitOne(PanelItem * panelItem, PanelParams & panelParams, QList<PlanePair *> & planePairs, bool createNew, bool customPartsOnly);
	static void addOptional(int optionalCount, QList<PanelItem *> & refPanelItems, QList<PanelItem *> & insertPanelItems, PanelParams &, QList<PlanePair *> &);
	static class MainWindow * inscribeBoard(QDomElement & board, QHash<QString, QString> & fzzFilePaths, FApplication * app, QDir & fzDir);
    static void doOnePanelItem(PlanePair * planePair, QList<LayerThing> & layerThingList, PanelItem * panelItem, QDir & svgDir);
    static void makeSVGs(MainWindow *, ItemBase *, const QString & boardName, QList<LayerThing> & layerThingList, QDir & saveDir);
    static void shrinkLastPanel( QList<PlanePair *> & planePairs, QList<PanelItem *> & insertPanelItems, PanelParams &, bool customPartsOnly);
};

#endif
