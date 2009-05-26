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

#include "connector.h"
#include "connectorshared.h"
#include "connectoritem.h"
#include "debugdialog.h"
#include "modelpart.h"
#include "bus.h"
#include "fsvgrenderer.h"


QHash <Connector::ConnectorType, QString > Connector::names;

Connector::Connector( ConnectorShared * connectorShared, ModelPart * modelPart)
{
	m_external = false;
	m_modelPart = modelPart;
	m_connectorShared = connectorShared;
	m_bus = NULL;
}

Connector::~Connector() {
	//DebugDialog::debug(QString("deleting connector %1 %2").arg((long) this, 0, 16).arg(connectorSharedID()));
	foreach (ConnectorItem * connectorItem, m_connectorItems) {
		connectorItem->clearConnector();
	}
}

void Connector::initNames() {
	if (names.count() == 0) {
		names.insert(Connector::Male, "male");
		names.insert(Connector::Female, "female");
		names.insert(Connector::Wire, "wire");
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
	if (m_connectorShared != NULL) {
		return m_connectorShared->connectorType();
	}

	return Connector::Unknown;
}

ConnectorShared * Connector::connectorShared() {
	return m_connectorShared;
}

void Connector::addViewItem(ConnectorItem * item) {
	m_connectorItems.append(item);
	//DebugDialog::debug(QString("adding view %1 %2").arg(this->connectorShared()->name()).arg(m_connectorItems.count()) );
}

void Connector::removeViewItem(ConnectorItem * item) {
	m_connectorItems.removeOne(item);

	//DebugDialog::debug(QString("removing view %1 %2").arg(this->connectorShared()->name()).arg(m_connectorItems.count()) );
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

void Connector::saveAsPart(QXmlStreamWriter & writer) {
	writer.writeStartElement("connector");
	writer.writeAttribute("id", connectorShared()->id());
	writer.writeAttribute("type", connectorShared()->connectorTypeString());
	writer.writeAttribute("name", connectorShared()->name());
	writer.writeTextElement("description", connectorShared()->description());
	writer.writeStartElement("views");
	QMultiHash<ViewIdentifierClass::ViewIdentifier,SvgIdLayer *> pins = m_connectorShared->pins();
	foreach (ViewIdentifierClass::ViewIdentifier currView, pins.keys()) {
		writer.writeStartElement(ViewIdentifierClass::viewIdentifierXmlName(currView));
		foreach (SvgIdLayer * svgIdLayer, pins.values(currView)) {
			writer.writeStartElement("p");
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

void Connector::writeSvgIdAttr(QXmlStreamWriter &writer, ViewIdentifierClass::ViewIdentifier view, QString connId) {
	Q_UNUSED(view);
	writer.writeAttribute("svgId",connId);
}

void Connector::writeTerminalIdAttr(QXmlStreamWriter &writer, ViewIdentifierClass::ViewIdentifier view, QString terminalId) {
        if((view == ViewIdentifierClass::BreadboardView || view == ViewIdentifierClass::SchematicView)
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

const QString & Connector::connectorSharedID() {
	if (m_connectorShared == NULL) return ___emptyString___;

	return m_connectorShared->id();
}

const QString & Connector::connectorSharedName() {
	if (m_connectorShared == NULL) return ___emptyString___;

	return m_connectorShared->name();
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

bool Connector::setUpConnector(FSvgRenderer * renderer, const QString & moduleID, ViewIdentifierClass::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID, QRectF & connectorRect, QPointF & terminalPoint, qreal & radius, qreal & strokeWidth, bool ignoreTerminalPoint) {
	// this code is a bit more viewish than modelish...

	Q_UNUSED(moduleID);

	if (m_connectorShared == NULL) return false;

	SvgIdLayer * svgIdLayer = m_connectorShared->fullPinInfo(viewIdentifier, viewLayerID);
	if (svgIdLayer == NULL) {
		return false;
	}

	if (svgIdLayer->m_processed) {
		if (!svgIdLayer->m_visible) return false;

		connectorRect = svgIdLayer->m_rect;
		terminalPoint = svgIdLayer->m_point;
		radius = svgIdLayer->m_radius;
		strokeWidth = svgIdLayer->m_strokeWidth;
	}
	else {

		svgIdLayer->m_processed = true;

		QString connectorID = svgIdLayer->m_svgId; // + "pin" ;

		QRectF bounds;
		qreal rad = 0;
		qreal sw = 0;
		if (!renderer->getSvgConnectorInfo(viewLayerID, connectorID, bounds, rad, sw)) {
			//DebugDialog::debug(QString("bounds on element %1").arg(connectorID) );
			bounds = renderer->boundsOnElement(connectorID);
		}

		if (bounds.isNull()) {
			svgIdLayer->m_visible = false;
			return false;
		}

		QSizeF defaultSizeF = renderer->defaultSizeF();
		if ((bounds.width()) == defaultSizeF.width() && (bounds.height()) == defaultSizeF.height()) {
			svgIdLayer->m_visible = false;
			return false;
		}

		QMatrix matrix0 = renderer->matrixForElement(connectorID);
		QRectF viewBox = renderer->viewBoxF();

		if (rad != 0) {
			radius = svgIdLayer->m_radius = rad * defaultSizeF.width() / viewBox.width();
			strokeWidth = svgIdLayer->m_strokeWidth = sw * defaultSizeF.width() / viewBox.width();
		}

		// TODO: all parts should either have connectors with or without a matrix
		if (matrix0.isIdentity()) {
			/*DebugDialog::debug(QString("identity matrix %11 %1 %2, viewbox: %3 %4 %5 %6, bounds: %7 %8 %9 %10, size: %12 %13").arg(m_modelPart->title()).arg(connectorSharedID())
							   .arg(viewBox.x()).arg(viewBox.y()).arg(viewBox.width()).arg(viewBox.height())
							   .arg(bounds.x()).arg(bounds.y()).arg(bounds.width()).arg(bounds.height())
							   .arg(viewIdentifier)
							   .arg(defaultSizeF.width()).arg(defaultSizeF.height())
			);
			*/
			connectorRect.setRect(bounds.x() * defaultSizeF.width() / viewBox.width(), bounds.y() * defaultSizeF.height() / viewBox.height(), bounds.width() * defaultSizeF.width() / viewBox.width(), bounds.height() * defaultSizeF.height() / viewBox.height());
		}
		else {
			connectorRect = matrix0.mapRect(bounds);
		}
		svgIdLayer->m_visible = true;
		svgIdLayer->m_rect = connectorRect;
		svgIdLayer->m_point = terminalPoint = calcTerminalPoint(svgIdLayer->m_terminalId, renderer, connectorRect, svgIdLayer, ignoreTerminalPoint, viewBox);
	}

	return true;
}

QPointF Connector::calcTerminalPoint(const QString & terminalId, FSvgRenderer * renderer,
									const QRectF & connectorRect, SvgIdLayer * svgIdLayer, bool ignoreTerminalPoint,
									const QRectF & viewBox)
{
	// this code is a bit more viewish than modelish...

	QPointF terminalPoint = connectorRect.center() - connectorRect.topLeft();    // default spot is centered
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
	QSizeF defaultSizeF = renderer->defaultSizeF();
	if (tBounds.width() >= defaultSizeF.width() && tBounds.height() >= defaultSizeF.height()) {
		return terminalPoint;
	}


	//DebugDialog::debug(	QString("terminal %5 rect %1,%2,%3,%4").arg(tBounds.x()).
										//arg(tBounds.y()).
										//arg(tBounds.width()).
										//arg(tBounds.height()).
										//arg(terminalID) );

	QMatrix tMatrix = renderer->matrixForElement(terminalId);
	if (tMatrix.isIdentity()) {
		QPointF c = tBounds.center();
		QPointF q(c.x() * defaultSizeF.width() / viewBox.width(), c.y() * defaultSizeF.height() / viewBox.height());
		terminalPoint = q - connectorRect.topLeft();
		//QRectF terminalRect = tMatrix.mapRect(tBounds);
		//QPointF tp = terminalRect.center() - connectorRect.topLeft();
		//if (qAbs(tp.x() - terminalPoint.x()) > 1 || (qAbs(tp.y() - terminalPoint.y()) > 1)) {
			//DebugDialog::debug("got a big difference in terminalrect");
		//}
	}
	else {
		QRectF terminalRect = tMatrix.mapRect(tBounds);
		terminalPoint = terminalRect.center() - connectorRect.topLeft();
	}
	svgIdLayer->m_point = terminalPoint;
	//DebugDialog::debug(	QString("terminalagain %3 rect %1,%2 ").arg(terminalPoint.x()).
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

int Connector::connectorItemCount() {
	return m_connectorItems.count();
}

void Connector::setExternal(bool external) {
	if (external == true) {
		DebugDialog::debug("external");
	}

	m_external = external;
}

bool Connector::external() {
	return m_external;
}
