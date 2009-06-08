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

$Revision: 2964 $:
$Author: cohen@irascible.com $:
$Date: 2009-05-19 07:03:39 +0200 (Tue, 19 May 2009) $

********************************************************************/

#include "clipablewire.h"
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

#define CONVEX

/* ======= Crossings algorithm ============================================ */

// from: http://tog.acm.org/GraphicsGems//gemsiv/ptpoly_haines/ptinpoly.c

#define XCOORD	0
#define YCOORD	1

/* Shoot a test ray along +X axis.  The strategy, from MacMartin, is to
 * compare vertex Y values to the testing point's Y and quickly discard
 * edges which are entirely to one side of the test ray.
 *
 * Input 2D polygon _pgon_ with _numverts_ number of vertices and test point
 * _point_, returns 1 if inside, 0 if outside.	WINDING and CONVEX can be
 * defined for this test.
 */
int CrossingsTest( double pgon[][2], int numverts, double point[2] )
{
#ifdef	WINDING
register int	crossings ;
#endif
register int	j, yflag0, yflag1, inside_flag, xflag0 ;
register double ty, tx, *vtx0, *vtx1 ;
#ifdef	CONVEX
register int	line_flag ;
#endif

    tx = point[XCOORD] ;
    ty = point[YCOORD] ;

    vtx0 = pgon[numverts-1] ;
    /* get test bit for above/below X axis */
    yflag0 = ( vtx0[YCOORD] >= ty ) ;
    vtx1 = pgon[0] ;

#ifdef	WINDING
    crossings = 0 ;
#else
    inside_flag = 0 ;
#endif
#ifdef	CONVEX
    line_flag = 0 ;
#endif
    for ( j = numverts+1 ; --j ; ) {

	yflag1 = ( vtx1[YCOORD] >= ty ) ;
	/* check if endpoints straddle (are on opposite sides) of X axis
	 * (i.e. the Y's differ); if so, +X ray could intersect this edge.
	 */
	if ( yflag0 != yflag1 ) {
	    xflag0 = ( vtx0[XCOORD] >= tx ) ;
	    /* check if endpoints are on same side of the Y axis (i.e. X's
	     * are the same); if so, it's easy to test if edge hits or misses.
	     */
	    if ( xflag0 == ( vtx1[XCOORD] >= tx ) ) {

		/* if edge's X values both right of the point, must hit */
#ifdef	WINDING
		if ( xflag0 ) crossings += ( yflag0 ? -1 : 1 ) ;
#else
		if ( xflag0 ) inside_flag = !inside_flag ;
#endif
	    } else {
		/* compute intersection of pgon segment with +X ray, note
		 * if >= point's X; if so, the ray hits it.
		 */
		if ( (vtx1[XCOORD] - (vtx1[YCOORD]-ty)*
		     ( vtx0[XCOORD]-vtx1[XCOORD])/(vtx0[YCOORD]-vtx1[YCOORD])) >= tx ) {
#ifdef	WINDING
		    crossings += ( yflag0 ? -1 : 1 ) ;
#else
		    inside_flag = !inside_flag ;
#endif
		}
	    }
#ifdef	CONVEX
	    /* if this is second edge hit, then done testing */
	    if ( line_flag ) goto Exit ;

	    /* note that one edge has been hit by the ray's line */
	    line_flag = TRUE ;
#endif
	}

	/* move to next pair of vertices, retaining info as possible */
	yflag0 = yflag1 ;
	vtx0 = vtx1 ;
	vtx1 += 2 ;
    }
#ifdef	CONVEX
    Exit: ;
#endif
#ifdef	WINDING
    /* test if crossings is not zero */
    inside_flag = (crossings != 0) ;
#endif

    return( inside_flag ) ;
}


/////////////////////////////////////////////////////////

ClipableWire::ClipableWire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu  ) 
	: Wire(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu)
{
	m_justFilteredEvent = NULL;
	m_cachedOriginalLine.setPoints(QPointF(-99999,-99999), QPointF(-99999,-99999));
}

const QLineF & ClipableWire::getPaintLine() {	
	if (!m_clipEnds) {
		return Wire::getPaintLine();
	}

	QLineF originalLine = this->line();
	if (m_cachedOriginalLine == originalLine) {
		return m_cachedLine;
	}

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

	QPointF p1 = originalLine.p1();
	QPointF p2 = originalLine.p2();
	// does connector0 always go with p1?
	if (to0) {
		p1 = findIntersection(to0, p1);
	}
	if (to1) {
		p2 = findIntersection(to1, p2);
	}

	m_cachedOriginalLine = originalLine;
	m_cachedLine.setPoints(p1, p2);
	return m_cachedLine;

}

void ClipableWire::setClipEnds(bool clipEnds ) {
	if (m_clipEnds != clipEnds) {
		prepareGeometryChange();
		m_clipEnds = clipEnds;
	}
}

QPointF ClipableWire::findIntersection(ConnectorItem * connectorItem, QPointF original)
{
	if (connectorItem->radius() > 0) {
		QRectF r = connectorItem->rect();
		qreal mu1, mu2;
		// penwidth / 2 deals with the extra length of the round line caps
		if (RaySphere(line().p1(), line().p2(), original, calcClipRadius(connectorItem), &mu1, &mu2)) {
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

qreal ClipableWire::calcClipRadius(ConnectorItem * connectorItem) {
	return connectorItem->radius() - (connectorItem->strokeWidth() / 2.0) + (m_pen.width() / 2.0);
}

bool ClipableWire::filterMousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	m_justFilteredEvent = NULL;

	if (!m_clipEnds) return false;

	ConnectorItem * to = NULL;
	foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
			to = toConnectorItem;
			break;
		}
	}
	if (to == NULL) return false;

	qreal rad = to->radius();
	if (rad <= 0) return false;

	rad -= ((to->strokeWidth() / 2) + 0.5);			// shrink it a little bit

	QRectF r = connectorItem->rect();
	QPointF c = r.center();
	QPointF p = event->pos();
	if ( (p.x() - c.x()) * (p.x() - c.x()) + (p.y() - c.y()) * (p.y() - c.y())  < rad * rad ) {
		// inside the inner circle, so ignore the event
		m_justFilteredEvent = event;
		return true;
	}

	QLineF normal = m_cachedOriginalLine.normalVector();
	normal.setLength(m_pen.width() / 2.0);
	double parallelogram[4][2];
	parallelogram[0][XCOORD] = normal.p2().x();
	parallelogram[0][YCOORD] = normal.p2().y();
	parallelogram[1][XCOORD] = normal.p1().x() - normal.dx();
	parallelogram[1][YCOORD] = normal.p1().y() - normal.dy();
	parallelogram[2][XCOORD] = parallelogram[1][XCOORD] + m_cachedOriginalLine.dx();
	parallelogram[2][YCOORD] = parallelogram[1][YCOORD] + m_cachedOriginalLine.dy();
	parallelogram[3][XCOORD] = parallelogram[0][XCOORD] + m_cachedOriginalLine.dx();
	parallelogram[3][YCOORD] = parallelogram[0][YCOORD] + m_cachedOriginalLine.dy();
	QPointF mp = mapFromScene(event->scenePos());
	double point[2];
	point[XCOORD] = mp.x();
	point[YCOORD] = mp.y();
	if (CrossingsTest(parallelogram, 4, point)) {
		return false;
	}

	event->ignore();
	return true;
}

void ClipableWire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if ((long) event == (long) m_justFilteredEvent) {
		event->ignore();
		return;
	}

	Wire::mousePressEvent(event);
}
