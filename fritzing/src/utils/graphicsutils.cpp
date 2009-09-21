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

$Revision: 3406 $:
$Author: cohen@irascible.com $:
$Date: 2009-09-04 12:26:26 +0200 (Fri, 04 Sep 2009) $

********************************************************************/

#include "graphicsutils.h"
#include "../fsvgrenderer.h"

#include <QList>
#include <QLineF>

void GraphicsUtils::distanceFromLine(double cx, double cy, double ax, double ay, double bx, double by, 
									 double & dx, double & dy, double &distanceSegment, bool & atEndpoint)
{

	// http://www.codeguru.com/forum/showthread.php?t=194400

	//
	// find the distance from the point (cx,cy) to the line
	// determined by the points (ax,ay) and (bx,by)
	//

	double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
	double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
	double r = r_numerator / r_denomenator;
     
	if ( (r >= 0) && (r <= 1) )
	{
		dx = ax + r*(bx-ax);
		dy = ay + r*(by-ay);
		distanceSegment = (cx-dx)*(cx-dx) + (cy-dy)*(cy-dy);
		atEndpoint = false;
	}
	else
	{
		atEndpoint = true;
		double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
		double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
		if (dist1 < dist2)
		{
			dx = ax;
			dy = ay;
			distanceSegment = dist1;
		}
		else
		{
			dx = bx;
			dy = by;
			distanceSegment = dist2;
		}
	}

	return;
}

struct PD {
	QPointF p;
	qreal d;
};

bool pdLessThan(PD* pd1, PD* pd2) {
	return pd1->d < pd2->d;
}

QPointF GraphicsUtils::calcConstraint(QPointF initial, QPointF current) {
	QList<PD *> pds;

	PD * pd = new PD;
	pd->p.setX(current.x());
	pd->p.setY(initial.y());
	pd->d = (current.y() - initial.y()) * (current.y() - initial.y());
	pds.append(pd);

	pd = new PD;
	pd->p.setX(initial.x());
	pd->p.setY(current.y());
	pd->d = (current.x() - initial.x()) * (current.x() - initial.x());
	pds.append(pd);

	qreal dx, dy, d;
	bool atEndpoint;

	QLineF plus45(initial.x() - 10000, initial.y() - 10000, initial.x() + 10000, initial.y() + 10000);
	distanceFromLine(current.x(), current.y(), plus45.p1().x(), plus45.p1().y(), plus45.p2().x(), plus45.p2().y(), dx, dy, d, atEndpoint);
	pd = new PD;
	pd->p.setX(dx);
	pd->p.setY(dy);
	pd->d = d;
	pds.append(pd);
		
	QLineF minus45(initial.x() + 10000, initial.y() - 10000, initial.x() - 10000, initial.y() + 10000);
	distanceFromLine(current.x(), current.y(), minus45.p1().x(), minus45.p1().y(), minus45.p2().x(), minus45.p2().y(), dx, dy, d, atEndpoint);
	pd = new PD;
	pd->p.setX(dx);
	pd->p.setY(dy);
	pd->d = d;
	pds.append(pd);

	qSort(pds.begin(), pds.end(), pdLessThan);
	QPointF result = pds[0]->p;
	foreach (PD* pd, pds) {
		delete pd;
	}
	return result;
}

qreal GraphicsUtils::pixels2mils(qreal p) {
	return p * 1000.0 / FSvgRenderer::printerScale();
}

qreal GraphicsUtils::pixels2ins(qreal p) {
	return p / FSvgRenderer::printerScale();
}

qreal GraphicsUtils::distance2(QPointF p1, QPointF p2) {
	return ((p1.x() - p2.x()) * (p1.x() - p2.x())) + ((p1.y() - p2.y()) * (p1.y() - p2.y()));
}


qreal GraphicsUtils::mm2mils(qreal mm) {
	return (mm / 25.4 * 1000);
}

qreal GraphicsUtils::pixels2mm(qreal p) {
	return (p / FSvgRenderer::printerScale() * 25.4);
}

qreal GraphicsUtils::mils2pixels(qreal m) {
	return (FSvgRenderer::printerScale() * m / 1000);
}

