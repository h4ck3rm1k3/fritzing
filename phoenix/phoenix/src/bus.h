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
