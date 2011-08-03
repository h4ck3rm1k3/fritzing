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

#ifndef BEZIER_H
#define BEZIER_H

#include <QPointF>
#include <QDomElement>
#include <QXmlStreamWriter>

// cubic bezier auxiliary implementation

class Bezier
{
public:
	Bezier(QPointF cp1, QPointF cp2);
	Bezier();

	QPointF cp0() const;
	QPointF cp1() const;
	void set_cp0(QPointF);
	void set_cp1(QPointF);
	bool isEmpty() const;
	void clear();
	void write(QXmlStreamWriter &);
	bool operator==(const Bezier &) const;
	bool operator!=(const Bezier &) const;
	void recalc(int controlIndex, QPointF p);
	void initToEnds(QPointF cp0, QPointF cp1); 
	double xFromT(double t);
	double yFromT(double t);
	void split(double t, Bezier & left, Bezier & right);
	int findControlIndex(QPointF fromPoint);

	static Bezier fromElement(QDomElement & element);

protected:
	QPointF m_endPoint0;
	QPointF m_endPoint1;
	QPointF m_cp0;
	QPointF m_cp1;
	double m_length;
	bool m_isEmpty;
};

#endif
