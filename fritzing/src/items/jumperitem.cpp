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

$Revision: 3176 $:
$Author: cohen@irascible.com $:
$Date: 2009-06-18 20:07:42 +0200 (Thu, 18 Jun 2009) $

********************************************************************/

#include "jumperitem.h"
#include "../connectoritem.h"
#include "../fsvgrenderer.h"
#include "../layerattributes.h"
#include "../modelpart.h"

/////////////////////////////////////////////////////////

JumperItem::JumperItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu  ) 
	: Wire(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu)
{
}

QPainterPath JumperItem::hoverShape() const
{
    QPainterPath path;
	QRectF rect = m_connector0->rect();
	path.addEllipse(rect);
	rect = m_connector1->rect();
	path.addEllipse(rect);
    return qt_graphicsItem_shapeFromPath(path, m_pen, 4);
}

QPainterPath JumperItem::shape() const
{
    QPainterPath path;
	QRectF rect = m_connector0->rect();
	path.addEllipse(rect);
	rect = m_connector1->rect();
	path.addEllipse(rect);
	return qt_graphicsItem_shapeFromPath(path, m_pen, 1);
}

void JumperItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsSvgItem::paint(painter, option, widget);
}

void JumperItem::initEnds(const ViewGeometry & vg, QRectF defaultRect, InfoGraphicsView * infoGraphicsView) {
	Q_UNUSED(defaultRect);
	Q_UNUSED(infoGraphicsView);

	bool gotOne = false;
	bool gotTwo = false;
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
		if (item == NULL) continue;

		// check the name or is order good enough?

		if (gotOne) {
			gotTwo = true;
			m_connector1 = item;
			break;
		}
		else {
			m_connector0 = item;
			gotOne = true;
		}
	}

	if (!gotTwo) {
		return;
	}

	m_hasLine = false;
	
	prepareGeometryChange();
}

FSvgRenderer * JumperItem::setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash &  viewLayers, InfoGraphicsView * infoGraphicsView ) {
	
	FSvgRenderer * renderer = Wire::setUp(viewLayerID, viewLayers, infoGraphicsView);
	if (renderer != NULL) {
		this->setSharedRenderer(renderer);
	}

	m_hasLine = false;

	return renderer;
}


void JumperItem::setLine(const QLineF &line)
{
	Wire::setLine(line);
	m_hasLine = false;
}
