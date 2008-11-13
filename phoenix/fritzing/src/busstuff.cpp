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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

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



	
