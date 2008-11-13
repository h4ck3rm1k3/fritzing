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
