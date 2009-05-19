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
#include "../connectoritem.h"
#include "../modelpart.h"
#include <math.h>

static double connectorRectClipInset = 0.5;
/*
	http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/raysphere.c
   Calculate the intersection of a ray and a sphere
   The line segment is defined from p1 to p2
   The sphere is of radius r and centered at sc
   There are potentially two points of intersection given by
   p = p1 + mu1 (p2 - p1)
   p = p1 + mu2 (p2 - p1)
   Return FALSE if the ray doesn't intersect the sphere.
*/
bool RaySphere(QPointF p1,QPointF p2,QPointF sc,double r,double *mu1,double *mu2)
{
   double a,b,c;
   double bb4ac;
   QPointF dp;

   dp.setX(p2.x() - p1.x());
   dp.setY(p2.y() - p1.y());
   a = dp.x() * dp.x() + dp.y() * dp.y();
   b = 2 * (dp.x() * (p1.x() - sc.x()) + dp.y() * (p1.y() - sc.y()));
   c = sc.x() * sc.x() + sc.y() * sc.y();
   c += p1.x() * p1.x() + p1.y() * p1.y();
   c -= 2 * (sc.x() * p1.x() + sc.y() * p1.y() );
   c -= r * r;
   bb4ac = b * b - 4 * a * c;
   if (qAbs(a) < .001 || bb4ac < 0) {
      *mu1 = 0;
      *mu2 = 0;
      return false;
   }

   *mu1 = (-b + sqrt(bb4ac)) / (2 * a);
   *mu2 = (-b - sqrt(bb4ac)) / (2 * a);

   return true;
}

float Magnitude( QPointF *Point1, QPointF *Point2 )
{
    QPointF Vector;

    Vector.setX(Point2->x() - Point1->x());
    Vector.setY(Point2->y() - Point1->y());

    return (float)sqrt( Vector.x() * Vector.x() + Vector.y() * Vector.y());
}

bool DistancePointLine( QPointF *Point, QPointF *LineStart, QPointF *LineEnd, float *Distance )
{
	Q_UNUSED(Distance);

    float LineMag;
    float U;
    QPointF Intersection;
 
    LineMag = Magnitude( LineEnd, LineStart );
 
    U = ( ( ( Point->x() - LineStart->x() ) * ( LineEnd->x() - LineStart->x() ) ) +
        ( ( Point->y() - LineStart->y() ) * ( LineEnd->y() - LineStart->y() ) )  ) /
        ( LineMag * LineMag );
 
    if( U < 0.0f || U > 1.0f )
        return false;   // closest point does not fall within the line segment
 

	// we don't actually care about distance, just whether we're on the line segment
	// so comment out the rest

	/* 
    Intersection.setX(LineStart->x() + U * ( LineEnd->x() - LineStart->x() ));
    Intersection.setY(LineStart->y() + U * ( LineEnd->y() - LineStart->y() ));
 
    *Distance = Magnitude( Point, &Intersection );
	*/
 
    return true;
}



/////////////////////////////////////////////////////////

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
	if (m_clipEnds != clipEnds) {
		prepareGeometryChange();
		m_clipEnds = clipEnds;
	}
}

QPointF TraceWire::findIntersection(ConnectorItem * connectorItem, QPointF original)
{
	if (connectorItem->radius() > 0) {
		QRectF r = connectorItem->rect();
		qreal mu1, mu2;
		// penwidth / 2 deals with the extra length of the round line caps
		if (RaySphere(line().p1(), line().p2(), original, connectorItem->radius() - (connectorItem->strokeWidth() / 2) + (m_pen.width() * 0.5), &mu1, &mu2)) {
			QPointF inter1 = line().p2() * mu1;
			QPointF p1 = line().p1();
			QPointF p2 = line().p2();
			float distance;
			if (DistancePointLine(&inter1, &p1, &p2, &distance)) {
				return inter1;
			}
			return line().p2() * mu2;
		}
	}

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




