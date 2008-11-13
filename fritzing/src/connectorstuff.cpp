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

#include "connectorstuff.h"
#include "debugdialog.h"
#include "connector.h"
#include "busstuff.h"

ConnectorStuff::ConnectorStuff()
{
	m_id = "";
	m_name = "";
	m_typeString = "";
	m_type = Connector::Unknown;
	m_description = "";
	m_viewThing = NULL;
	m_bus = NULL;
}

ConnectorStuff::ConnectorStuff( const QDomElement & domElement )
{
	m_id = domElement.attribute("id", "");
	m_name = domElement.attribute("name", "");
	m_typeString = domElement.attribute("type", "");
	m_type = Connector::connectorTypeFromName(m_typeString);
	m_description = domElement.firstChildElement("description").text();
	loadPins(domElement);
	m_viewThing = NULL;
	m_bus = NULL;
}

const QString & ConnectorStuff::id() {
	return m_id;
}
void ConnectorStuff::setId(QString id) {
	m_id = id;
}

const QString & ConnectorStuff::description() {
	return m_description;
}
void ConnectorStuff::setDescription(QString description) {
	m_description = description;
}

const QString & ConnectorStuff::name() {
	return m_name;
}
void ConnectorStuff::setName(QString name) {
	m_name = name;
}

Connector::ConnectorType ConnectorStuff::connectorType() {
	return m_type;
}

const QString & ConnectorStuff::connectorTypeString() {
	return m_typeString;
}

void ConnectorStuff::setConnectorType(QString type) {
	m_typeString = type;
	m_type = Connector::connectorTypeFromName(type);
}

const QMultiHash<ItemBase::ViewIdentifier,SvgIdLayer*> & ConnectorStuff::pins() {
	return m_pins;
}

void ConnectorStuff::addPin(ItemBase::ViewIdentifier layer, QString connectorId, ViewLayer::ViewLayerID viewLayerID, QString terminalId) {
	SvgIdLayer * svgIdLayer = new SvgIdLayer;
	svgIdLayer->m_viewLayerID = viewLayerID;
	svgIdLayer->m_svgId = connectorId;
	svgIdLayer->m_terminalId = terminalId;
	m_pins.insert(layer, svgIdLayer);
}

void ConnectorStuff::removePins(ItemBase::ViewIdentifier layer) {
	m_pins.remove(layer);
	Q_ASSERT(m_pins.values(layer).size() == 0);
}

const QString ConnectorStuff::pin(ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerID) {
	QList<SvgIdLayer *> svgLayers = m_pins.values(viewId);
	foreach ( SvgIdLayer * svgIdLayer, svgLayers) {
		if (svgIdLayer->m_viewLayerID == viewLayerID) {
			return svgIdLayer->m_svgId;
		}
	}

	return ___emptyString___;
}

const QString ConnectorStuff::terminal(ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerID) {
	QList<SvgIdLayer *> svgLayers = m_pins.values(viewId);
	foreach ( SvgIdLayer * svgIdLayer, svgLayers) {
		if (svgIdLayer->m_viewLayerID == viewLayerID) {
			return svgIdLayer->m_terminalId;
		}
	}

	return ___emptyString___;

}

const SvgIdLayer * ConnectorStuff::fullPinInfo(ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerID) {
	QList<SvgIdLayer *> svgLayers = m_pins.values(viewId);
	foreach ( SvgIdLayer * svgIdLayer, svgLayers) {
		if (svgIdLayer->m_viewLayerID == viewLayerID) {
			return svgIdLayer;
		}
	}

	return NULL;
}

void ConnectorStuff::loadPins(const QDomElement & domElement) {
	//if(m_domElement == NULL) return;

	// TODO: this is view-related stuff and it would be nice if the model didn't know about it
	QDomElement viewsTag = domElement.firstChildElement("views");
	loadPin(viewsTag.firstChildElement("breadboardView"),ItemBase::BreadboardView);
	loadPin(viewsTag.firstChildElement("schematicView"),ItemBase::SchematicView);
	loadPin(viewsTag.firstChildElement("pcbView"),ItemBase::PCBView);
}

void ConnectorStuff::loadPin(QDomElement elem, ItemBase::ViewIdentifier viewId) {
	QDomElement pinElem = elem.firstChildElement("pin");
	while (!pinElem.isNull()) {
		QString svgId = pinElem.attribute("svgId");
		svgId = svgId.left(svgId.lastIndexOf(QRegExp("\\d"))+1);
		QString layer = pinElem.attribute("layer");
		SvgIdLayer * svgIdLayer = new SvgIdLayer();
		svgIdLayer->m_svgId = svgId;
		svgIdLayer->m_viewLayerID = ViewLayer::viewLayerIDFromXmlString(layer);

		QString terminalId = pinElem.attribute("terminalId");
		//DebugDialog::debug(QString("svg id view layer id %1, %2").arg(svgIdLayer->m_viewLayerID).arg(layer));
		svgIdLayer->m_terminalId = terminalId;
		m_pins.insert(viewId, svgIdLayer);

		pinElem = pinElem.nextSiblingElement("pin");
	}
}

void ConnectorStuff::setViewThing(ViewThing * viewThing) {
	m_viewThing = viewThing;
}

ViewThing * ConnectorStuff::viewThing() {
	return m_viewThing;
}

void ConnectorStuff::setBus(BusStuff * bus) {
	m_bus = bus;
}

BusStuff * ConnectorStuff::bus() {
	return m_bus;
}

const QString & ConnectorStuff::busID() {
	if (m_bus == NULL) return ___emptyString___;
		return m_bus->id();
}
