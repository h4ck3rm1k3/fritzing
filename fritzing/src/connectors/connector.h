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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QHash>
#include <QString>
#include <QList>
#include <QXmlStreamWriter>
#include <QGraphicsScene>
#include <QSvgRenderer>
#include <QPointer>

#include "../viewidentifierclass.h"
#include "../viewlayer.h"

class Connector : public QObject
{
Q_OBJECT
public:
	enum ConnectorType {
		Male,
		Female,
		Wire,
		Unknown
	};

public:
	Connector(class ConnectorShared *, class ModelPart * modelPart);
	~Connector();

	Connector::ConnectorType connectorType();
	void addViewItem(class ConnectorItem *);
	void removeViewItem(class ConnectorItem *);
	class ConnectorShared * connectorShared();
	void connectTo(Connector *);
	void disconnectFrom(Connector *);
	void saveAsPart(QXmlStreamWriter & writer);
	const QList<Connector *> & toConnectors();
	ConnectorItem * connectorItem(QGraphicsScene *);
	bool connectionIsAllowed(Connector* that);
	const QString & connectorSharedID();
	const QString & connectorSharedName();
	class ErcData * connectorSharedErcData();
	const QString & busID();
	class Bus * bus();
	void setBus(class Bus *);
	bool setUpConnector(class FSvgRenderer * renderer, const QString & moduleID, ViewIdentifierClass::ViewIdentifier, ViewLayer::ViewLayerID, QRectF & connectorRect, QPointF & terminalPoint, qreal & radius, qreal & strokeWidth, bool ignoreTerminalPoint);
	long modelIndex();
	ModelPart * modelPart();
	int connectorItemCount();
	void unprocess(ViewIdentifierClass::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID);

public:
	static void initNames();
	static const QString & connectorNameFromType(ConnectorType);
	static ConnectorType connectorTypeFromName(const QString & name);

protected:
	void writeLayerAttr(QXmlStreamWriter &writer, ViewLayer::ViewLayerID);
	void writeSvgIdAttr(QXmlStreamWriter &writer, ViewIdentifierClass::ViewIdentifier view, QString connId);
	void writeTerminalIdAttr(QXmlStreamWriter &writer, ViewIdentifierClass::ViewIdentifier view, QString terminalId);
	QPointF calcTerminalPoint(const QString & terminalId, class FSvgRenderer * renderer, 
							  const QRectF & connectorRect, bool ignoreTerminalPoint, const QRectF & viewBox, QMatrix &);

protected:
	QPointer<class ConnectorShared> m_connectorShared;
	QList< QPointer<class ConnectorItem> > m_connectorItems;
	QList<Connector *> m_toConnectors;
	QPointer<class ModelPart> m_modelPart;
	class Bus * m_bus;

protected:
	static QHash<ConnectorType, QString> names;
};

#endif
