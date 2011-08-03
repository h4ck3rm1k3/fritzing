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

#include "bezier.h"
#include "graphicsutils.h"
#include "../debugdialog.h"

/////////////////////////////////////////////

// utilities from http://www.flong.com/texts/code/shapers_bez/

double B0 (double t){
  return (1-t)*(1-t)*(1-t);
}
double B1 (double t){
  return  3*t* (1-t)*(1-t);
}
double B2 (double t){
  return 3*t*t* (1-t);
}
double B3 (double t){
  return t*t*t;
}

/////////////////////////////////////////////

Bezier::Bezier(QPointF cp0, QPointF cp1)
{
	m_length = 0;
	m_cp0 = cp0;
	m_cp1 = cp1;
	m_isEmpty = false;
}

Bezier::Bezier()
{
	m_length = 0;
	m_isEmpty = true;
}


bool Bezier::isEmpty() const
{
	return m_isEmpty;
}

void Bezier::clear()
{
	m_isEmpty = true;
}

QPointF Bezier::cp0() const
{
	return m_cp0;
}

QPointF Bezier::cp1() const
{
	return m_cp1;
}

void Bezier::set_cp0(QPointF cp0)
{
	m_cp0 = cp0;
	m_isEmpty = false;
}

void Bezier::set_cp1(QPointF cp1)
{
	m_cp1 = cp1;
	m_isEmpty = false;
}

Bezier Bezier::fromElement(QDomElement & element) 
{
	Bezier bezier;
	if (element.tagName().compare("bezier") != 0) return bezier;

	QDomElement point = element.firstChildElement("cp0");
	double x = point.attribute("x").toDouble();
	double y = point.attribute("y").toDouble();
	bezier.set_cp0(QPointF(x, y));
	point = element.firstChildElement("cp1");
	x = point.attribute("x").toDouble();
	y = point.attribute("y").toDouble();
	bezier.set_cp1(QPointF(x, y));

	return bezier;
}

void Bezier::write(QXmlStreamWriter & streamWriter)
{
	if (isEmpty()) return;

	streamWriter.writeStartElement("bezier");

	streamWriter.writeStartElement("cp0");
	streamWriter.writeAttribute("x", QString::number(m_cp0.x()));
	streamWriter.writeAttribute("y", QString::number(m_cp0.y()));
	streamWriter.writeEndElement();
	streamWriter.writeStartElement("cp1");
	streamWriter.writeAttribute("x", QString::number(m_cp1.x()));
	streamWriter.writeAttribute("y", QString::number(m_cp1.y()));
	streamWriter.writeEndElement();
		
	streamWriter.writeEndElement();
}

bool Bezier::operator==(const Bezier & other) const
{
	if (m_isEmpty != other.isEmpty()) return false;
	if (m_cp0 != other.cp0()) return false;
	if (m_cp1 != other.cp1()) return false;

	return true;
}

bool Bezier::operator!=(const Bezier & other) const
{
	if (m_isEmpty != other.isEmpty()) return true;
	if (m_cp0 != other.cp0()) return true;
	if (m_cp1 != other.cp1()) return true;

	return false;
}

void Bezier::recalc(int controlIndex, QPointF p)
{
	// from http://www.tinaja.com/text/bezmath.html, 
	// http://www.lemoda.net/maths/bezier-length/index.html,
	// and http://www.flong.com/texts/code/shapers_bez/

	// arbitrary but reasonable 
	// t-values for interior control points
	double t0 = 0.3;
	double t1 = 0.7;

	if (controlIndex == 0) {
		double x = (p.x() - m_endPoint0.x() * B0(t0) - m_cp1.x() * B2(t0) - m_endPoint1.x() * B3(t0)) / B1(t0);
		double y = (p.y() - m_endPoint0.y() * B0(t0) - m_cp1.y() * B2(t0) - m_endPoint1.y() * B3(t0)) / B1(t0);
		m_cp0 = QPointF(x, y);
	}
	else {
		double x = (p.x() - m_endPoint0.x() * B0(t1) - m_cp0.x() * B1(t1) - m_endPoint1.x() * B3(t1)) /  B2(t1);
		double y = (p.y() - m_endPoint0.y() * B0(t1) - m_cp0.y() * B1(t1) - m_endPoint1.y() * B3(t1)) /  B2(t1);
		m_cp1 = QPointF(x, y);
	}
	
	DebugDialog::debug(QString("ix:%1 p0x:%2,p0y:%3 p1x:%4,p1y:%5 px:%6,py:%7")
							.arg(controlIndex)
							.arg(m_endPoint0.x())
							.arg(m_endPoint0.y())
							.arg(m_endPoint1.x())
							.arg(m_endPoint1.y())
							.arg(p.x())
							.arg(p.y())
							);


	m_isEmpty = false;
}

void Bezier::initToEnds(QPointF cp0, QPointF cp1) 
{
	m_endPoint0 = m_cp0 = cp0;
	m_endPoint1 = m_cp1 = cp1;
	m_isEmpty = false;
}

double Bezier::xFromT(double t)
{
    // from http://www.lemoda.net/maths/bezier-length/index.html

    return m_endPoint0.x() * B0(t) + m_cp0.x() * B1(t) + m_cp1.x() * B2(t) + m_endPoint1.x() * B3(t);
}

double Bezier::yFromT(double t)
{
    // from http://www.lemoda.net/maths/bezier-length/index.html

    return m_endPoint0.y() * B0(t) + m_cp0.y() * B1(t) + m_cp1.y() * B2(t) + m_endPoint1.y() * B3(t);
}

void Bezier::split(double t, Bezier & left, Bezier & right)
{
	// from http://steve.hollasch.net/cgindex/curves/cbezarclen.html

	// t ranges from 0 to 1

    int   i, j;                             
    QPointF   vtemp[4][4];                      /* Triangle Matrix */
    
    /* Copy control points  */

	vtemp[0][0] = m_endPoint0;
	vtemp[0][1] = m_cp0;
	vtemp[0][2] = m_cp1;
	vtemp[0][3] = m_endPoint1;
       
    /* Triangle computation */
    for (i = 1; i <= 3; i++) {  
		for (j =0 ; j <= 3 - i; j++) {
			vtemp[i][j].setX(t * vtemp[i-1][j].x() + (1 - t) * vtemp[i-1][j+1].x());
			vtemp[i][j].setY(t * vtemp[i-1][j].y() + (1 - t) * vtemp[i-1][j+1].y());
		}                                      
    }                                       
    
    left.m_endPoint0 = vtemp[0][0];
    left.m_cp0 = vtemp[1][0];
    left.m_cp1 = vtemp[2][0];
    left.m_endPoint1 = vtemp[3][0];
      
    right.m_endPoint0 = vtemp[3][0];
    right.m_cp0 = vtemp[2][1];
    right.m_cp1 = vtemp[1][2];
    right.m_endPoint1 = vtemp[0][3];
} 

int Bezier::findControlIndex(QPointF p)
{
	double d0 = GraphicsUtils::distanceSqd(p, m_cp0);
	double d1 = GraphicsUtils::distanceSqd(p, m_cp1);
	return (d0 <= d1) ? 0 : 1;
}
