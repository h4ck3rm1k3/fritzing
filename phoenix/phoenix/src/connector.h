#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QHash>
#include <QString>
#include <QList>
#include <QXmlStreamWriter>
#include <QGraphicsScene>
#include <QSvgRenderer>

#include "itembase.h"

class Connector
{
public:
	enum ConnectorType {
		Male,
		Female,
		Wire,
		BusType,
		Unknown
	};

public:
	Connector(class ConnectorStuff *, class ModelPart * modelPart);

	Connector::ConnectorType connectorType();
	void addViewItem(class ConnectorItem *);
	void removeViewItem(class ConnectorItem *);
	class ConnectorStuff * connectorStuff();
	void connectTo(Connector *);
	void disconnectFrom(Connector *);
	//void saveInstances(QXmlStreamWriter & writer);
	void saveAsPart(QXmlStreamWriter & writer);
	const QList<Connector *> & toConnectors();
	ConnectorItem * connectorItem(QGraphicsScene *);
	bool connectionIsAllowed(Connector* that);
	const QString & connectorStuffID();
	const QString & busID();
	class Bus * bus();
	void setBus(class Bus *);
	bool setUpConnector(QSvgRenderer * renderer, ItemBase::ViewIdentifier, ViewLayer::ViewLayerID, QRectF & connectorRect, QPointF & terminalPoint, bool ignoreTerminalPoint);
	long modelIndex();
	ModelPart * modelPart();

public:
	static void initNames();
	static const QString & connectorNameFromType(ConnectorType);
	static ConnectorType connectorTypeFromName(const QString & name);

protected:
	void writeLayerAttr(QXmlStreamWriter &writer, ViewLayer::ViewLayerID);
	void writeSvgIdAttr(QXmlStreamWriter &writer, ItemBase::ViewIdentifier view, QString connId);
	void writeTerminalIdAttr(QXmlStreamWriter &writer, ItemBase::ViewIdentifier view, QString terminalId);
	QPointF calcTerminalPoint(const QString & terminalId, QSvgRenderer * renderer, ItemBase::ViewIdentifier, ViewLayer::ViewLayerID, const QRectF & connectorRect, class ConnectorViewThing *, bool ignoreTerminalPoint);

protected:
	class ConnectorStuff * m_connectorStuff;
	QList<class ConnectorItem *> m_connectorItems;
	QList<Connector *> m_toConnectors;
	class ModelPart * m_modelPart;
	class Bus * m_bus;

	static QHash<ConnectorType, QString> names;
};

#endif
