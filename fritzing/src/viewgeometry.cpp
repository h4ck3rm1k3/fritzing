/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "viewgeometry.h"

// get a compiler errow when using WireFlags instead of QFlags<>
QFlags<ViewGeometry::WireFlag> ViewGeometry::TraceJumperRatsnestFlags = ViewGeometry::TraceFlag | ViewGeometry::JumperFlag | ViewGeometry::RatsnestFlag;

ViewGeometry::ViewGeometry(  )
{
	m_z = -1;
	m_loc.setX(-1);
	m_loc.setY(-1);
	m_line.setLine(-1,-1,-1,-1);
	m_wireFlags = NoFlag;
}

ViewGeometry::ViewGeometry(const ViewGeometry & that  )
{
	set(that);
}

ViewGeometry::ViewGeometry(QDomElement & geometry) {
	m_wireFlags = (WireFlags) geometry.attribute("wireFlags").toInt();
	m_z = geometry.attribute("z").toDouble();
	m_loc.setX(geometry.attribute("x").toDouble());
	m_loc.setY(geometry.attribute("y").toDouble());
	QString x1 = geometry.attribute("x1");
	if (!x1.isEmpty()) {
		m_line.setLine( geometry.attribute("x1").toDouble(),
						geometry.attribute("y1").toDouble(),
						geometry.attribute("x2").toDouble(),
						geometry.attribute("y2").toDouble() );
	}
	QString w = geometry.attribute("width");
	if (!w.isEmpty()) {
		m_rect.setX(geometry.attribute("x").toDouble());
		m_rect.setY(geometry.attribute("y").toDouble());
		m_rect.setWidth(geometry.attribute("width").toDouble());
		m_rect.setHeight(geometry.attribute("height").toDouble());
	}
	QDomElement transformElement = geometry.firstChildElement("transform");
	if (!transformElement.isNull()) {
		qreal m11 = m_transform.m11();
		qreal m12 = m_transform.m12();
		qreal m13 = m_transform.m13();
		qreal m21 = m_transform.m21();
		qreal m22 = m_transform.m22();
		qreal m23 = m_transform.m23();
		qreal m31 = m_transform.m31();
		qreal m32 = m_transform.m32();
		qreal m33 = m_transform.m33();
		bool ok;
		qreal temp;

		temp = transformElement.attribute("m11").toDouble(&ok);
		if (ok) m11 = temp;
		temp = transformElement.attribute("m12").toDouble(&ok);
		if (ok) m12 = temp;
		temp = transformElement.attribute("m13").toDouble(&ok);
		if (ok) m13 = temp;
		temp = transformElement.attribute("m21").toDouble(&ok);
		if (ok) m21 = temp;
		temp = transformElement.attribute("m22").toDouble(&ok);
		if (ok) m22 = temp;
		temp = transformElement.attribute("m23").toDouble(&ok);
		if (ok) m23 = temp;
		temp = transformElement.attribute("m31").toDouble(&ok);
		if (ok) m31 = temp;
		temp = transformElement.attribute("m32").toDouble(&ok);
		if (ok) m32 = temp;
		temp = transformElement.attribute("m33").toDouble(&ok);
		if (ok) m33 = temp;

		m_transform.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);
	}
}

void ViewGeometry::setZ(qreal z) {
	m_z = z;
}

qreal ViewGeometry::z() const {
	return m_z ;
}

void ViewGeometry::setLoc(QPointF loc) {
	m_loc = loc;
}

QPointF ViewGeometry::loc() const {
	return m_loc ;
}

void ViewGeometry::setLine(QLineF loc) {
	m_line = loc;
}

QLineF ViewGeometry::line() const {
	return m_line;
}

void ViewGeometry::offset(qreal x, qreal y) {
	m_loc.setX(x + m_loc.x());
	m_loc.setY(y + m_loc.y());
}

bool ViewGeometry::selected() {
	return m_selected;
}

void ViewGeometry::setSelected(bool selected) {
	m_selected = selected;
}

QRectF ViewGeometry::rect() const {
	return m_rect;
}

void ViewGeometry::setTransform(QTransform transform) {
	m_transform = transform;
}

QTransform ViewGeometry::transform() const {
	return m_transform;
}

void ViewGeometry::set(const ViewGeometry & that) {
	m_z = that.m_z;
	m_line = that.m_line;
	m_loc = that.m_loc;
	m_transform = that.m_transform;
	m_wireFlags = that.m_wireFlags;
}

void ViewGeometry::setVirtual(bool virt) {
	setWireFlag(virt, VirtualFlag);
}

void ViewGeometry::setRouted(bool routed) {
	setWireFlag(routed, RoutedFlag);
}

void ViewGeometry::setTrace(bool trace) {
	setWireFlag(trace, TraceFlag);
}

void ViewGeometry::setJumper(bool jumper) {
	setWireFlag(jumper, JumperFlag);
}

void ViewGeometry::setRatsnest(bool ratsnest) {
	setWireFlag(ratsnest, RatsnestFlag);
}

void ViewGeometry::setAutoroutable(bool autoroutable) {
	setWireFlag(autoroutable, AutoroutableFlag);
}

bool ViewGeometry::getVirtual() const {
	return m_wireFlags.testFlag(VirtualFlag);
}

bool ViewGeometry::getRouted() const {
	
	return m_wireFlags.testFlag(RoutedFlag);
}

bool ViewGeometry::getTrace() const {
	return m_wireFlags.testFlag(TraceFlag);
}

bool ViewGeometry::getJumper() const {
	return m_wireFlags.testFlag(JumperFlag);
}

bool ViewGeometry::getRatsnest() const {
	return m_wireFlags.testFlag(RatsnestFlag);
}

bool ViewGeometry::getAutoroutable() const {
	return m_wireFlags.testFlag(AutoroutableFlag);
}

void ViewGeometry::setWireFlag(bool setting, WireFlag flag) {
	if (setting) {
		m_wireFlags |= flag;
	}
	else {
		m_wireFlags &= ~flag;
	}

}

int ViewGeometry::flagsAsInt() const {
	return (int) m_wireFlags;
}

void ViewGeometry::setWireFlags(WireFlags wireFlags) {
	m_wireFlags = wireFlags;
}

bool ViewGeometry::hasFlag(ViewGeometry::WireFlag flag) {
	return (m_wireFlags & flag) ? true : false;
}

bool ViewGeometry::hasAnyFlag(ViewGeometry::WireFlags flags) {
	return (m_wireFlags & flags) ? true : false;
}


ViewGeometry::WireFlags ViewGeometry::wireFlags() {
	return m_wireFlags;
}
