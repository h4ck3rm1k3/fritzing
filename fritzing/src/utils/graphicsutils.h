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

$Revision: 3418 $:
$Author: merunga $:
$Date: 2009-09-07 10:55:58 +0200 (Mon, 07 Sep 2009) $

********************************************************************/

#ifndef GRAPHICSUTILS_H
#define GRAPHICSUTILS_H

#include <QPointF>

class GraphicsUtils
{

public:
	static void distanceFromLine(double cx, double cy, double ax, double ay, double bx, double by, 
								 double & dx, double & dy, double &distanceSegment, bool & atEndpoint);
	static QPointF calcConstraint(QPointF initial, QPointF current);

};

#endif
