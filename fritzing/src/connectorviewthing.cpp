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

#include "connectorviewthing.h"
#include "debugdialog.h"

int ConnectorViewThing::instanceCount = 0;

ConnectorViewThing::ConnectorViewThing(  ) 
{
	instanceCount++;
	DebugDialog::debug(QString("instanceCount %1").arg(instanceCount));
	m_visible = m_processed = false;
}

void ConnectorViewThing::setVisibleInView(bool visible) {
	m_visible = visible;	
}

bool ConnectorViewThing::visibleInView() {
	return m_visible;
}

QRectF ConnectorViewThing::rectInView() {
	return m_rect;
}

void ConnectorViewThing::setRectInView(QRectF rect) {
	m_rect = rect;
	m_visible = true;	
}

void ConnectorViewThing::setProcessed(bool processed) {
	m_processed = processed;
	m_visible = false;	
}

bool ConnectorViewThing::processed() {
	return m_processed;
}

QPointF ConnectorViewThing::terminalPointInView() {
	return m_point;
}

void ConnectorViewThing::setTerminalPointInView(QPointF point) {
	m_point = point;
}


