/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef GRAPHICSUTILS_H
#define GRAPHICSUTILS_H

#include <QPointF>
#include <QTransform>
#include <QXmlStreamWriter>
#include <QDomElement>
#include <QPixmap>

class GraphicsUtils
{

public:
	static void distanceFromLine(double cx, double cy, double ax, double ay, double bx, double by, 
								 double & dx, double & dy, double &distanceSegment, bool & atEndpoint);
	static QPointF calcConstraint(QPointF initial, QPointF current);

	static qreal pixels2mils(qreal p, qreal dpi);
	static qreal pixels2ins(qreal p, qreal dpi);
	static qreal distance2(QPointF p1, QPointF p2);
	static qreal getNearestOrdinate(qreal ordinate, qreal units);

	static qreal mm2mils(qreal mm);
	static qreal pixels2mm(qreal p, qreal dpi);
	static qreal mils2pixels(qreal m, qreal dpi);
	static void saveTransform(QXmlStreamWriter & streamWriter, const QTransform & transform);
	static bool loadTransform(const QDomElement & transformElement, QTransform & transform);
	static bool isRect(const QPolygonF & poly);
	static QRectF getRect(const QPolygonF & poly);
	static void shortenLine(QPointF & p1, QPointF & p2, qreal d1, qreal d2);
	static bool liangBarskyLineClip(double x1, double y1, double x2, double y2, 
									double wxmin, double wxmax, double wymin, double wymax, 
									double & x11, double & y11, double & x22, double & y22);
	static QString toHtmlImage(QPixmap *pixmap, const char* format = "PNG");

public:
	static const int IllustratorDPI = 72;
	static const int StandardFritzingDPI = 1000;
	static const qreal InchesPerMeter;


};

#endif
