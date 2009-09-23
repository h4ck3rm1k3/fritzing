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

#ifndef JUMPERITEM_H
#define JUMPERITEM_H

#include "paletteitem.h"

class JumperItem : public PaletteItem
{

public:
	JumperItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier,  const ViewGeometry & , long id, QMenu* itemMenu, bool doLabel = true); 

    QPainterPath shape() const;
    QPainterPath hoverShape() const;
 	bool setUpImage(ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors);
	void saveParams();
	void getParams(QPointF & pos, QPointF & c0, QPointF & c1);
	void resize(QPointF pos, QPointF c0, QPointF c1);
	void resize(QPointF p0, QPointF p1);
	QSizeF footprintSize();
	QString retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	bool autoroutable();
	void setAutoroutable(bool);
	class ConnectorItem * connector0();
	class ConnectorItem * connector1();

protected:
	void resize();
	QString makeSvg(ViewLayer::ViewLayerID);
	QPainterPath makePath() const;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void resizeAux(qreal r0x, qreal r0y, qreal r1x, qreal r1y);

protected:
	ConnectorItem * m_dragItem;
	ConnectorItem * m_connector0;
	ConnectorItem * m_connector1;
	ConnectorItem * m_otherItem;
	QPointF m_dragStartScenePos;
	QPointF m_dragStartThisPos;
	QPointF m_dragStartConnectorPos;
	QPointF m_otherPos;
	QPointF m_connectorTL;
	QPointF m_connectorBR;
	QPointF m_itemPos;
	QPointF m_itemC0;
	QPointF m_itemC1;
	class FSvgRenderer * m_renderer;
	class FSvgRenderer * m_jumperwiresRenderer;
	bool m_autoroutable;

};

#endif
