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

/*
 *  bus.cpp
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/4/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include "bus.h"
#include "busstuff.h"
#include "connectorstuff.h"
#include "debugdialog.h"
#include "connectoritem.h"
#include "modelpart.h"


QHash<QString, class Bus *> Bus::___emptyBusList___;


Bus::Bus(BusStuff * busStuff, ModelPart * modelPart)
{
	m_busStuff = busStuff;
	m_modelPart = modelPart;
	m_busConnector = NULL;

}

const QString & Bus::id() {
	if (m_busStuff == NULL) return ___emptyString___;

	return m_busStuff->id();
}


const QList<Connector *> & Bus::connectors() {
	return m_connectors;
}

void Bus::addViewItem(ConnectorItem * item) {
	m_connectorItems.append(item);
}

void Bus::removeViewItem(ConnectorItem * item) {
	m_connectorItems.removeOne(item);
}

void Bus::addConnector(Connector * connector) {
	// the list of connectors which make up the bus
	m_connectors.append(connector);
}

Connector * Bus::busConnector() {
	if (m_busConnector == NULL) {
		m_busConnector = new Connector(NULL, m_modelPart);
		m_busConnector->setBus(this);
	}

	return m_busConnector;
}

void Bus::merge(Bus * that) {
	if (!m_merged.contains(that)) {
		m_merged.append(that);
	}
}

void Bus::unmerge(Bus * that) {
	m_merged.removeOne(that);
}

ModelPart * Bus::modelPart() {
	return m_modelPart;
}

ConnectorItem * Bus::connectorItem(QGraphicsScene * scene) {
	foreach (ConnectorItem * connectorItem, m_connectorItems) {
		if (connectorItem->scene() == scene) return connectorItem;
	}

	return NULL;
}
