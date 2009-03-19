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

#ifndef PALETTEITEMBASE_H
#define PALETTEITEMBASE_H

#include <QGraphicsSvgItem>
#include <QGraphicsSceneMouseEvent>
#include <QSet>

#include "modelpart.h"
#include "itembase.h"
#include "viewgeometry.h"
#include "graphicssvglineitem.h"
#include "viewlayer.h"

class LayerKinPaletteItem;

class PaletteItemBase : public ItemBase
{
	Q_OBJECT

public:
	PaletteItemBase(ModelPart *, ItemBase::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu);

	void saveGeometry();
	bool itemMoved();
	virtual void saveInstanceLocation(QXmlStreamWriter &);
	void moveItem(ViewGeometry &);
	virtual void syncKinSelection(bool selected, PaletteItemBase *originator);
	virtual void syncKinMoved(QPointF offset, QPointF loc);
 	void blockItemSelectedChange(bool selected);
 	bool syncSelected();
 	QPointF syncMoved();
	void mousePressConnectorEvent(class ConnectorItem *, QGraphicsSceneMouseEvent *);
	bool acceptsMousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
 	virtual bool setUpImage(ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors);
	const QString & filename();
	void connectedMoved(ConnectorItem * from, ConnectorItem * to);
	void updateConnectionsAux();
	void collectFemaleConnectees(QSet<ItemBase *> & items);
	void collectWireConnectees(QSet<class Wire *> & wires);
	virtual void updateConnections() = 0;
	virtual void mousePressEvent(PaletteItemBase * originalItem, QGraphicsSceneMouseEvent *event);
	//bool isBuriedConnectorHit(QGraphicsSceneMouseEvent *event);
	virtual void figureHover() = 0;
	QRectF boundingRect() const;
	virtual bool isLowerConnectorLayerVisible(PaletteItemBase *);
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	virtual void saveStickyOffsets(QGraphicsSceneMouseEvent *event);


	/*
	// for debugging
	void setPos(const QPointF & pos);
	void setPos(qreal x, qreal y);
	 */

protected:
	QPainterPath shape() const;
 	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void setUpConnectors(FSvgRenderer *, bool ignoreTerminalPoints);
	void findConnectorsUnder();
	void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );

	virtual LayerKinPaletteItem * newLayerKinPaletteItem(
		PaletteItemBase * chief, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier,
		const ViewGeometry & viewGeometry, long id,ViewLayer::ViewLayerID viewLayerID, QMenu* itemMenu, const LayerHash & viewLayers
	);

protected:
 	bool m_blockItemSelectedChange;
 	bool m_blockItemSelectedValue;
 	QPointF m_offset;
 	bool m_syncSelected;
 	QPointF m_syncMoved;
 	bool m_svg;
	QString m_filename;
	QPointF m_stickyPos;

};


#endif
