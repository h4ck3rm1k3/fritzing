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

#define pixels2mils(p) ((p) * 1000.0 / FSvgRenderer::printerScale())
#define pixels2ins(p) ((p) / FSvgRenderer::printerScale())

// TODO: 
//	save and load
//	undo
//	ignore during autoroute
//	ignore during other connections



inline qreal distance2(QPointF p1, QPointF p2) {
	return ((p1.x() - p2.x()) * (p1.x() - p2.x())) + ((p1.y() - p2.y()) * (p1.y() - p2.y()));
}

/////////////////////////////////////////////////////////

JumperItem::JumperItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel) 
	: PaletteItem(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu, doLabel)
{
	m_renderer = NULL;
	m_connector0 = m_connector1 = m_dragItem = NULL;
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
    QPainterPath path = makePath();
    return qt_graphicsItem_shapeFromPath(path, m_pen, 1);
}

QPainterPath JumperItem::shape() const
{
	QPainterPath path = makePath();
	return qt_graphicsItem_shapeFromPath(path, m_pen, 4);
}

QPainterPath JumperItem::makePath() const {
	QPainterPath path;
	QRectF rect = m_connector0->rect();
	qreal dx = m_connectorTL.x();
	qreal dy = m_connectorTL.y();
	rect.adjust(-dx, -dy, dx, dy);
	path.addEllipse(rect);
	rect = m_connector1->rect();
	dx = m_connectorBR.x();
	dy = m_connectorBR.y();
	rect.adjust(-dx, -dy, dx, dy);
	path.addEllipse(rect);
	return path;
}

bool JumperItem::setUpImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors)
{
	bool result = PaletteItem::setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, doConnectors);

	if (doConnectors) {
		foreach (QGraphicsItem * childItem, childItems()) {
			ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
			if (item == NULL) continue;

			item->setCircular(true);
			if (item->connectorSharedName().contains('0')) {
				m_connector0 = item;
				m_connectorTL = m_connector0->rect().topLeft();			
			}
			else if (item->connectorSharedName().contains('1')) {
				m_connector1 = item;
				m_connectorBR = boundingRect().bottomRight() - m_connector1->rect().bottomRight();
			}
		}
	}

	return result;
}

void JumperItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QPointF c0 = m_connector0->rect().center();
	QPointF c1 = m_connector1->rect().center();
	if (distance2(c0, event->pos()) <= distance2(c1, event->pos())) {
		m_dragItem = m_connector0;
		m_otherItem = m_connector1;
	}
	else {
		m_dragItem = m_connector1;
		m_otherItem = m_connector0;
	}

	m_dragStartScenePos = event->scenePos();
	m_dragStartThisPos = this->pos();
	m_dragStartConnectorPos = this->mapToScene(m_dragItem->rect().topLeft());
	m_otherPos = this->mapToScene(m_otherItem->rect().topLeft());
}

void JumperItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_dragItem == NULL) return;

	// TODO: make sure the two connectors don't overlap

	QPointF d = event->scenePos() - m_dragStartScenePos;
	QPointF p = m_dragStartConnectorPos + d;
	QPointF myPos(qMin(p.x(), m_otherPos.x()) - m_connectorTL.x(), 
				  qMin(p.y(), m_otherPos.y()) - m_connectorTL.y());
	this->setPos(myPos);
	QRectF r = m_otherItem->rect();
	r.moveTo(mapFromScene(m_otherPos));
	m_otherItem->setRect(r);
	r = m_dragItem->rect();
	r.moveTo(mapFromScene(p));
	m_dragItem->setRect(r);
	resize();
}

void JumperItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
}

QString JumperItem::makeSvg() 
{
	QRectF r0 = m_connector0->rect();
	QRectF r1 = m_connector1->rect();
	QRectF r = r0.united(r1);
	qreal w = pixels2ins(r.width() + m_connectorTL.x() + m_connectorBR.x());
	qreal h = pixels2ins(r.height() + m_connectorTL.y() + m_connectorBR.y());
	return Copper0LayerTemplate
		.arg(w).arg(h)
		.arg(w * 1000).arg(h * 1000)			
		.arg(pixels2mils(r0.center().x())).arg(pixels2mils(r0.center().y()))
		.arg(pixels2mils(r1.center().x())).arg(pixels2mils(r1.center().y()));
}


void JumperItem::resize() {
	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}
	QString s = makeSvg();
	DebugDialog::debug(s);

	bool result = m_renderer->fastLoad(s.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
	}
	//	DebugDialog::debug(QString("fast load result %1 %2").arg(result).arg(s));
}

