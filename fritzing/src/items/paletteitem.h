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

$Revision$:
$Author$:
$Date$

********************************************************************/


#ifndef PALETTEITEM_H
#define PALETTEITEM_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>

#include "paletteitembase.h"
#include "../viewlayer.h"

class PaletteItem : public PaletteItemBase 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	PaletteItem(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel = true);
	~PaletteItem();

	void removeLayerKin();
	void addLayerKin(class LayerKinPaletteItem * lkpi);
	const QList<class ItemBase *> & layerKin();
 	virtual void loadLayerKin(const LayerHash & viewLayers);
	void rotateItem(qreal degrees);
	void flipItem(Qt::Orientations orientation);
	void moveItem(ViewGeometry & viewGeometry);
	void setItemPos(QPointF & pos);

	bool renderImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors);

	void setTransforms();
	void syncKinMoved(QPointF offset, QPointF loc);

	void setInstanceTitle(const QString&);
	void updateTooltip();

	bool swap(ModelPart* newModelPart, const LayerHash &layerHash, bool reinit, class SwapCommand *);
	QString family();
	void setHidden(bool hidden);
	void collectFemaleConnectees(QSet<ItemBase *> & items);
	void collectWireConnectees(QSet<class Wire *> & wires);
	void clearModelPart();
	void mousePressEvent(PaletteItemBase * originalItem, QGraphicsSceneMouseEvent *);
	ItemBase * lowerConnectorLayerVisible(ItemBase *);
	void resetID();
	void blockSyncKinMoved(bool block);

protected:
	void syncKinSelection(bool selected, PaletteItemBase * originator);
 	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateConnections();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void figureHover();

protected:
 	QList<class ItemBase *> m_layerKin;
	bool m_blockSyncKinMoved;

};

#endif
