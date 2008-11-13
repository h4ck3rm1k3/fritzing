#ifndef CONNECTORSTUFF_H
#define CONNECTORSTUFF_H

#include <QString>
#include <QDomElement>
#include <QMultiHash>

#include "viewthing.h"
#include "itembase.h"
#include "connector.h"


struct SvgIdLayer {
	QString m_svgId;
	QString m_terminalId;
	ViewLayer::ViewLayerID m_viewLayerID;
};


class ConnectorStuff
{

public:
	ConnectorStuff();
	ConnectorStuff(const QDomElement & domElement);

	const QString & id();
	void setId(QString id);
	const QString & description();
	void setDescription(QString description);
	const QString & name();
	void setName(QString name);
	const QString & connectorTypeString();
	void setConnectorType(QString type);
	Connector::ConnectorType connectorType();

	const QMultiHash<ItemBase::ViewIdentifier,SvgIdLayer *> &pins();
	const QString pin(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID viewLayerID);
	const QString terminal(ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerID);
	const SvgIdLayer * fullPinInfo(ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerID);
	void addPin(ItemBase::ViewIdentifier layer, QString connectorId, ViewLayer::ViewLayerID, QString terminalId);
	void removePins(ItemBase::ViewIdentifier layer);

	ViewThing * viewThing();
	void setViewThing(ViewThing *);
	class BusStuff * bus();
	void setBus(class BusStuff *);
	const QString & busID();

protected:
	void loadPins(const QDomElement & domElement);
	void loadPin(QDomElement elem, ItemBase::ViewIdentifier viewId);

	QString m_description;
	QString m_id;
	QString m_name;
	QString m_typeString;
	Connector::ConnectorType m_type;

	/*
	 * The connectorId is used to generate the attribute svgId and terminalId in the FZ file,
	 * appending the words "pin" and "terminal" (respectively) to it
	 */
	QMultiHash<ItemBase::ViewIdentifier, SvgIdLayer*> m_pins;

	ViewThing * m_viewThing;
	class BusStuff * m_bus;
};

static QList<ConnectorStuff *> ___emptyConnectorStuffList___;

#endif
