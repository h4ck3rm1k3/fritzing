/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "modelpart.h"
#include "itembase.h"
#include "viewgeometry.h"
#include "graphicssvglineitem.h"
#include "viewlayer.h"

class PaletteItemBase : public ItemBase
{
	Q_OBJECT

public:
	PaletteItemBase(ModelPart *, ItemBase::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool topLevel);

	void saveGeometry();
	bool itemMoved();
	virtual void saveInstanceLocation(QXmlStreamWriter &);
	void moveItem(ViewGeometry &);
	void rotateItem(qreal degrees);
	void flipItem(Qt::Orientations orientation);
	virtual void syncKinSelection(bool selected, PaletteItemBase *originator);
	virtual void syncKinMoved(QPointF offset, QPointF loc);
 	void blockItemSelectedChange(bool selected);
 	bool syncSelected();
 	QPointF syncMoved();
	void mousePressConnectorEvent(class ConnectorItem *, QGraphicsSceneMouseEvent *);
 	virtual bool setUpImage(ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors);
	const QString & filename();
	void connectedMoved(ConnectorItem * from, ConnectorItem * to);
	void updateConnections(ConnectorItem *);
	void updateConnectionsAux();
	virtual void updateConnections() = 0;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	bool isBuriedConnectorHit(QGraphicsSceneMouseEvent *event);
	
	/*
	// for debugging
	void setPos(const QPointF & pos);
	void setPos(qreal x, qreal y);
	 */

public:
	static QSvgRenderer * setUpImage(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID, class LayerAttributes &);

signals:
	void connectionChangedSignal(ConnectorItem * from, ConnectorItem * to, bool connect);

protected:
	QRectF boundingRect() const;
	QPainterPath shape() const;
 	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void setUpConnectors(QSvgRenderer *, bool ignoreTerminalPoints);
	void transformItem(QTransform currTransf);
	void findConnectorsUnder();

protected:
 	bool m_blockItemSelectedChange;
 	bool m_blockItemSelectedValue;
 	QPointF m_offset;
 	bool m_syncSelected;
 	QPointF m_syncMoved;
 	bool m_svg;
	QString m_filename;
	QPointF m_stickyPos;

protected:
	static QString SvgFilesDir;
};


#endif
