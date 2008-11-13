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
