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

static QString Copper0LayerTemplate = "";

/////////////////////////////////////////////////////////

JumperItem::JumperItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel) 
	: PaletteItem(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu, doLabel)
{
	m_dragItem = NULL;
	if (Copper0LayerTemplate.isEmpty()) {
		QFile file(":/resources/jumper_copper0LayerTemplate.txt");
		if (file.open(QFile::ReadOnly)) {
			Copper0LayerTemplate = file.readAll();
			file.close();
		}
	}
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

bool JumperItem::setUpImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors)
{
	bool result = PaletteItem::setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, doConnectors);

	if (doConnectors) {
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
	}

	return result;
}


void JumperItem::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	m_dragItem = connectorItem;
	m_dragStartPos = event->scenePos();
	m_originalRect = connectorItem->rect();

	foreach (QGraphicsItem * childItem, childItems()) {
		if (childItem == connectorItem) continue;

		m_otherItem = dynamic_cast<ConnectorItem *>(childItem);
		if (m_otherItem != NULL) break;
	}

}

void JumperItem::mouseReleaseConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	if (m_dragItem == NULL) return;

	// undo command
}

void JumperItem::mouseMoveConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	if (m_dragItem == NULL) return;

	QRectF copy = m_originalRect;
	QPointF d = event->scenePos() - m_dragStartPos;
	copy.translate(d);
	connectorItem->setRect(copy);
}

bool JumperItem::acceptsMouseMoveConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

bool JumperItem::acceptsMouseReleaseConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}
