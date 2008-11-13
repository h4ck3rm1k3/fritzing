/*
 *  busstuff.h
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/4/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#ifndef BUSSTUFF_H
#define BUSSTUFF_H

#include <QString>
#include <QDomElement>
#include <QHash>
#include <QList>
#include <QXmlStreamWriter>

class BusStuff {
	
public:
	BusStuff(const QDomElement & busElement, const QHash<QString, class ConnectorStuff *> & connectorHash);
	
	const QString & id();
	const QList<class ConnectorStuff *> & connectors();
	
protected:
	QString m_id;
	QList<class ConnectorStuff *> m_connectors;
};

#endif
