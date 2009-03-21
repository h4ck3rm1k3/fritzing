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

#include "tracewire.h"
#include "connectoritem.h"
#include "modelpart.h"

static double connectorRectClipInset = 0.5;

TraceWire::TraceWire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu  ) 
	: Wire(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu)
{
	m_clipEnds = true;
}

const QLineF & TraceWire::getPaintLine() {	
	if (!m_clipEnds) {
		return Wire::getPaintLine();
	}

	// it would be nice to cache all these calculations

	ConnectorItem* to0 = NULL;
	foreach (ConnectorItem * toConnectorItem, m_connector0->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
			to0 = toConnectorItem;
			break;
		}
	}

	ConnectorItem* to1 = NULL;
	foreach (ConnectorItem * toConnectorItem, m_connector1->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
			to1 = toConnectorItem;
			break;
		}
	}

	if (to0 == NULL && to1 == NULL) {
		// no need to clip; get out
		return Wire::getPaintLine();
	}


	QPointF p1 = line().p1();
	QPointF p2 = line().p2();
	// does connector0 always go with p1?
	if (to0) {
		p1 = findIntersection(to0, p1);
	}
	if (to1) {
		p2 = findIntersection(to1, p2);
	}

	m_cachedLine.setP1(p1);
	m_cachedLine.setP2(p2);
	return m_cachedLine;

}

void TraceWire::setClipEnds(bool clipEnds ) {
	m_clipEnds = clipEnds;
}

QPointF TraceWire::findIntersection(ConnectorItem * connectorItem, QPointF original)
{
	QRectF r = connectorItem->rect();
	r.adjust(connectorRectClipInset, connectorRectClipInset, -connectorRectClipInset, -connectorRectClipInset);	// inset it a little bit so the wire touches
	QPolygonF poly = this->mapFromScene(connectorItem->mapToScene(r));
	QLineF l1 = this->line();
	int count = poly.count();
	for (int i = 0; i < count; i++) {
		QLineF l2(poly[i], poly[(i + 1) % count]);
		QPointF intersectingPoint;
		if (l1.intersect(l2, &intersectingPoint) == QLineF::BoundedIntersection) {
			return intersectingPoint;
		}
	}

	return original;
}