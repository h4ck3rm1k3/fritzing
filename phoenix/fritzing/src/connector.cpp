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

#include "connector.h"
#include "connectorstuff.h"
#include "connectoritem.h"
#include "debugdialog.h"
#include "modelpart.h"
#include "bus.h"
#include "connectorviewthing.h"


QHash <Connector::ConnectorType, QString > Connector::names;

Connector::Connector( ConnectorStuff * connectorStuff, ModelPart * modelPart)
{
	m_modelPart = modelPart;
	m_connectorStuff = connectorStuff;
	m_bus = NULL;
}

void Connector::initNames() {
	if (names.count() == 0) {
		names.insert(Connector::Male, "male");
		names.insert(Connector::Female, "female");
		names.insert(Connector::Wire, "wire");
		names.insert(Connector::BusType, "bus");
	}
}

Connector::ConnectorType Connector::connectorTypeFromName(const QString & name) {
	QHashIterator<ConnectorType, QString> i(names);
    while (i.hasNext()) {
        i.next();
		if (name.compare(i.value(), Qt::CaseInsensitive ) == 0) {
			return i.key();
		}
    }

	return Connector::Unknown;
}

const QString & Connector::connectorNameFromType(ConnectorType type) {
	return names[type];
}

Connector::ConnectorType Connector::connectorType() {
	if (m_connectorStuff != NULL) {
		return m_connectorStuff->connectorType();
	}
	else {
		return Connector::BusType;
	}
}

ConnectorStuff * Connector::connectorStuff() {
	return m_connectorStuff;
}

void Connector::addViewItem(ConnectorItem * item) {
	m_connectorItems.append(item);
	//DebugDialog::debug(QObject::tr("adding view %1 %2").arg(this->connectorStuff()->name()).arg(m_connectorItems.count()) );
}

void Connector::removeViewItem(ConnectorItem * item) {
	m_connectorItems.removeOne(item);

	//DebugDialog::debug(QObject::tr("removing view %1 %2").arg(this->connectorStuff()->name()).arg(m_connectorItems.count()) );
}

void Connector::connectTo(Connector * connector) {

	if (this->modelPart() == NULL) {
		DebugDialog::debug("connecting bus connector 1");
	}
	else if (connector->modelPart() == NULL) {
		DebugDialog::debug("connecting bus connector 2");
	}

	if (!m_toConnectors.contains(connector)) {
		m_toConnectors.append(connector);
	}
	if (!connector->m_toConnectors.contains(this)) {
		connector->m_toConnectors.append(this);
	}
}

void Connector::disconnectFrom(Connector * connector) {
	m_toConnectors.removeOne(connector);
	connector->m_toConnectors.removeOne(this);
}

/*
void Connector::saveInstances(QXmlStreamWriter & writer) {
	if (m_toConnectors.count() <= 0) {
		// no need to save if there's no connection
		return;
	}

	writer.writeStartElement("connector");
	writer.writeAttribute("connectorId", connectorStuffID());
	writer.writeStartElement("views");
	foreach (ConnectorItem * connectorItem, m_connectorItems) {
		connectorItem->saveInstance(writer);
	}
	writer.writeEndElement();
	foreach (Connector * connector, this->m_toConnectors) {
		writer.writeStartElement("connect");
		writer.writeAttribute("connectorId", connector->connectorStuffID());
		writer.writeAttribute("busId", connector->busID());
		writer.writeAttribute("modelIndex", QString::number(connector->modelIndex()));
		writer.writeEndElement();
	}

	writer.writeEndElement();
}
*/

void Connector::saveAsPart(QXmlStreamWriter & writer) {
	writer.writeStartElement("connector");
	writer.writeAttribute("id", connectorStuff()->id());
	writer.writeAttribute("type", connectorStuff()->connectorTypeString());
	writer.writeAttribute("name", connectorStuff()->name());
	writer.writeTextElement("description", connectorStuff()->description());
	writer.writeStartElement("views");
	QMultiHash<ItemBase::ViewIdentifier,SvgIdLayer *> pins = m_connectorStuff->pins();
	foreach (ItemBase::ViewIdentifier currView, pins.keys()) {
		writer.writeStartElement(ItemBase::viewIdentifierXmlName(currView));
		foreach (SvgIdLayer * svgIdLayer, pins.values(currView)) {
			writer.writeStartElement("pin");
			writeLayerAttr(writer, svgIdLayer->m_viewLayerID);
			writeSvgIdAttr(writer, currView, svgIdLayer->m_svgId);
			writeTerminalIdAttr(writer, currView, svgIdLayer->m_terminalId);
			writer.writeEndElement();
		}
		writer.writeEndElement();
	}
	writer.writeEndElement();
	writer.writeEndElement();
}

void Connector::writeLayerAttr(QXmlStreamWriter &writer, ViewLayer::ViewLayerID viewLayerID) {
	writer.writeAttribute("layer",ViewLayer::viewLayerXmlNameFromID(viewLayerID));
}

void Connector::writeSvgIdAttr(QXmlStreamWriter &writer, ItemBase::ViewIdentifier view, QString connId) {
	QString attrValue = "";
	if(view == ItemBase::BreadboardView || view == ItemBase::SchematicView) {
		attrValue = connId+"pin";
	} else if(view == ItemBase::PCBView) {
		attrValue = connId+"pad";
	} else {
		return;
	}
	writer.writeAttribute("svgId",attrValue);
}

void Connector::writeTerminalIdAttr(QXmlStreamWriter &writer, ItemBase::ViewIdentifier view, QString terminalId) {
        if((view == ItemBase::BreadboardView || view == ItemBase::SchematicView)
            &&
           (!terminalId.isNull() && !terminalId.isEmpty()) ) {
		writer.writeAttribute("terminalId",terminalId);
	} else {
		return;
	}
}

const QList<Connector *> & Connector::toConnectors() {
	return m_toConnectors;
}

ConnectorItem * Connector::connectorItem(QGraphicsScene * scene) {
	foreach (ConnectorItem * connectorItem, m_connectorItems) {
		if (connectorItem->scene() == scene) return connectorItem;
	}

	return NULL;
}

bool Connector::connectionIsAllowed(Connector* that)
{
	Connector::ConnectorType thisConnectorType = connectorType();
	Connector::ConnectorType thatConnectorType = that->connectorType();
	if (thisConnectorType == Connector::Unknown) return false;
	if (thatConnectorType == Connector::Unknown) return false;		// unknowns are celebate

	if (thisConnectorType == Connector::Wire) return true;
	if (thatConnectorType == Connector::Wire) return true;		// wires are bisexual

	return (thisConnectorType != thatConnectorType);		// otherwise heterosexual
}

const QString & Connector::connectorStuffID() {
	if (m_connectorStuff == NULL) return ___emptyString___;

	return m_connectorStuff->id();
}


const QString & Connector::busID() {
	if (m_bus == NULL) return ___emptyString___;

	return m_bus->id();
}

Bus * Connector::bus() {
	return m_bus;
}

void Connector::setBus(Bus * bus) {
	m_bus = bus;
}

bool Connector::setUpConnector(QSvgRenderer * renderer, ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID, QRectF & connectorRect, QPointF & terminalPoint, bool ignoreTerminalPoint) {
	// this code is a bit more viewish than modelish...

	if (m_connectorStuff == NULL) return false;

	ConnectorViewThing * connectorViewThing = dynamic_cast<ConnectorViewThing *>(m_connectorStuff->viewThing());
	if (connectorViewThing == NULL) {
		connectorViewThing = new ConnectorViewThing();
		m_connectorStuff->setViewThing(connectorViewThing);
	}

	if (connectorViewThing->processed(viewIdentifier,viewLayerID)) {
		if (!connectorViewThing->visibleInView(viewIdentifier, viewLayerID)) return false;

		connectorRect = connectorViewThing->rectInView(viewIdentifier, viewLayerID);
		terminalPoint = connectorViewThing->terminalPointInView(viewIdentifier, viewLayerID);
	}
	else {
		connectorViewThing->setProcessed(viewIdentifier, viewLayerID, true);

		const SvgIdLayer * svgIdLayer = m_connectorStuff->fullPinInfo(viewIdentifier, viewLayerID);
		if (svgIdLayer == NULL) {
			return false;
		}

		QString connectorID = svgIdLayer->m_svgId + "pin" ;

		//DebugDialog::debug(QString("bounds on element %1").arg(connectorID) );
		QRectF bounds = renderer->boundsOnElement(connectorID);
		if (bounds.isNull()) {
			connectorViewThing->setVisibleInView(viewIdentifier, viewLayerID, false);
						return false;
		}
		QSize defaultSize = renderer->defaultSize();
		if (((int) bounds.width()) == defaultSize.width() && ((int) bounds.height()) == defaultSize.height()) {
			connectorViewThing->setVisibleInView(viewIdentifier, viewLayerID, false);
			return false;
		}

		QMatrix matrix0 = renderer->matrixForElement(connectorID);

		// TODO: all parts should either have connectors with or without a matrix
		if (matrix0.isIdentity() && viewIdentifier == ItemBase::PCBView) {
			QRectF viewBox = renderer->viewBoxF();
			QSize defaultSize = renderer->defaultSize();
			/*DebugDialog::debug(QString("identity matrix %11 %1 %2, viewbox: %3 %4 %5 %6, bounds: %7 %8 %9 %10, size: %12 %13").arg(m_modelPart->title()).arg(connectorStuffID())
							   .arg(viewBox.x()).arg(viewBox.y()).arg(viewBox.width()).arg(viewBox.height())
							   .arg(bounds.x()).arg(bounds.y()).arg(bounds.width()).arg(bounds.height())
							   .arg(viewIdentifier)
							   .arg(defaultSize.width()).arg(defaultSize.height())
			);
			*/
			connectorRect.setRect(bounds.x() * defaultSize.width() / viewBox.width(), bounds.y() * defaultSize.height() / viewBox.height(), bounds.width() * defaultSize.width() / viewBox.width(), bounds.height() * defaultSize.height() / viewBox.height());
		}
		else {
			connectorRect = matrix0.mapRect(bounds);
		}
		connectorViewThing->setRectInView(viewIdentifier, viewLayerID, connectorRect);
		connectorViewThing->setTerminalPointInView(viewIdentifier, viewLayerID, connectorRect.center() - connectorRect.topLeft());		// default spot is centered
		terminalPoint = calcTerminalPoint(svgIdLayer->m_terminalId, renderer, viewIdentifier, viewLayerID, connectorRect, connectorViewThing, ignoreTerminalPoint);
	}

	return true;
}

QPointF Connector::calcTerminalPoint(const QString & terminalId, QSvgRenderer * renderer, ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID,
									const QRectF & connectorRect, ConnectorViewThing * connectorViewThing, bool ignoreTerminalPoint)
{
	// this code is a bit more viewish than modelish...

	QPointF terminalPoint = connectorRect.center() - connectorRect.topLeft();
	connectorViewThing->setTerminalPointInView(viewIdentifier, viewLayerID, terminalPoint);
	if (ignoreTerminalPoint) {
		return terminalPoint;
	}
	if (terminalId.isNull() || terminalId.isEmpty()) {
		return terminalPoint;
	}

	QRectF tBounds = renderer->boundsOnElement(terminalId);
	if (tBounds.isNull()) {
		return terminalPoint;
	}
	QSize defaultSize = renderer->defaultSize();
	if (((int) tBounds.width()) >= defaultSize.width() && ((int) tBounds.height()) >= defaultSize.height()) {
		return terminalPoint;
	}


	//DebugDialog::debug(	tr("terminal %5 rect %1,%2,%3,%4").arg(tBounds.x()).
										//arg(tBounds.y()).
										//arg(tBounds.width()).
										//arg(tBounds.height()).
										//arg(terminalID) );

	QMatrix tMatrix = renderer->matrixForElement(terminalId);
	QRectF terminalRect = tMatrix.mapRect(tBounds);
	terminalPoint = terminalRect.center() - connectorRect.topLeft();
	connectorViewThing->setTerminalPointInView(viewIdentifier, viewLayerID, terminalPoint);
	//DebugDialog::debug(	tr("terminalagain %3 rect %1,%2 ").arg(terminalPoint.x()).
										//arg(terminalPoint.y()).
										//arg(terminalID) );

	return terminalPoint;
}

long Connector::modelIndex() {
	if (m_modelPart != NULL) return m_modelPart->modelIndex();

	DebugDialog::debug(QString("saving bus connector item: how is this supposed to work?"));
	return 0;
}


ModelPart * Connector::modelPart() {
	return m_modelPart;
}

