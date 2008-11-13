/*
 *  bus.cpp
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/4/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include "busstuff.h"
#include "connectorstuff.h"
#include "debugdialog.h"
#include "connectoritem.h"

BusStuff::BusStuff(const QDomElement & busElement, const QHash<QString, ConnectorStuff *> & connectorHash)
{
	m_id = busElement.attribute("id");
	
	QDomElement connector = busElement.firstChildElement("nodeMember");
	while (!connector.isNull()) {
		QString id = connector.attribute("connectorId");
		if (id.isNull()) continue;
		if (id.isEmpty()) continue;
				
		ConnectorStuff * stuff = connectorHash.value(id);
		if (stuff == NULL) continue;
		
		m_connectors.append(stuff);
		stuff->setBus(this);
		
		connector = connector.nextSiblingElement("nodeMember");
	}
	
}

const QString & BusStuff::id() {
	return m_id;
}


const QList<ConnectorStuff *> & BusStuff::connectors() {
	return m_connectors;
}



	
