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
 *  bus.h
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/4/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#ifndef BUS_H
#define BUS_H

#include <QString>
#include <QDomElement>
#include <QHash>
#include <QList>
#include <QXmlStreamWriter>
#include <QGraphicsScene>

class Bus {
	
public:
	Bus(class BusStuff *, class ModelPart *);
	
	const QString & id();
	const QList<class Connector *> & connectors();
	void addViewItem(class ConnectorItem *);
	void removeViewItem(class ConnectorItem *);
	void addConnector(class Connector *);
	class Connector * busConnector();
	void merge(Bus * that);
	void unmerge(Bus * that);
	class ModelPart * modelPart();
	ConnectorItem * connectorItem(QGraphicsScene * scene);
	
public:
	static QHash<QString, class Bus *> ___emptyBusList___;

	
protected:

	QList<class ConnectorItem *> m_connectorItems;
	QList<class Connector *> m_connectors;
	BusStuff * m_busStuff;
	class Connector * m_busConnector;
	class ModelPart * m_modelPart;
	QList <Bus *> m_merged;

};


#endif
