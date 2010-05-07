/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "groundplane.h"
#include "../connectors/connectoritem.h"
#include "../fsvgrenderer.h"
#include "../model/modelpart.h"
#include "../utils/graphicsutils.h"
#include "../svg/svgfilesplitter.h"
#include "../svg/groundplanegenerator.h"

#include <QPainterPathStroker>

/////////////////////////////////////////////////////////

GroundPlane::GroundPlane( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel) 
	: PaletteItem(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu, doLabel)
{
	m_renderer = NULL;
	m_connector0 = NULL;
}

bool GroundPlane::setUpImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors)
{
	bool result = PaletteItem::setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, doConnectors);

	if (doConnectors) {
		foreach (QGraphicsItem * childItem, childItems()) {
			ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
			if (item == NULL) continue;

			if (item->connectorSharedName().contains('0')) {
				m_connector0 = item;
				break;
			}
		}
	}

	return result;
}

void GroundPlane::saveParams() {
}

void GroundPlane::getParams() {
}

QString GroundPlane::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	QString xml = "";
	if (viewLayerID == ViewLayer::GroundPlane) {
		xml = modelPart()->prop("svg").toString();
	}

	if (!xml.isEmpty()) {
		QString xmlName = ViewLayer::viewLayerXmlNameFromID(viewLayerID);
		SvgFileSplitter splitter;
		bool result = splitter.splitString(xml, xmlName);
		if (!result) {
			return ___emptyString___;
		}
		result = splitter.normalize(dpi, xmlName, blackOnly);
		if (!result) {
			return ___emptyString___;
		}
		return splitter.elementString(xmlName);
	}


	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

ConnectorItem * GroundPlane::connector0() {
	return m_connector0;
}

bool GroundPlane::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::PCBView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

void GroundPlane::setProp(const QString & prop, const QString & value) {
	if (prop.compare("svg", Qt::CaseInsensitive) == 0) {
		setSvg(value);
		return;
	}

	PaletteItemBase::setProp(prop, value);
}

QVariant GroundPlane::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSceneHasChanged:
			if (this->scene()) {
				setSvgAux(modelPart()->prop("svg").toString());
			}
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}


void GroundPlane::setSvg(const QString & svg) {
	modelPart()->setProp("svg", svg);
	setSvgAux(svg);
}

void GroundPlane::setSvgAux(const QString & svg) {
	QString xmlName = ViewLayer::viewLayerXmlNameFromID(ViewLayer::GroundPlane);
	SvgFileSplitter	splitter;
	QString cpy = svg;
	bool result = splitter.splitString(cpy, xmlName);
	if (result) {
		if (m_renderer == NULL) {
			m_renderer = new FSvgRenderer(this);
		}
		//DebugDialog::debug(svg);

		bool result = m_renderer->fastLoad(svg.toUtf8());
		if (result) {
			setSharedRenderer(m_renderer);
		}

		QPainterPath painterPath = splitter.painterPath(FSvgRenderer::printerScale(), GroundPlaneGenerator::ConnectorName);
		if (m_connector0) {
			m_connector0->setRect(this->boundingRect());
			//QMatrix m;
			//m.scale(0.75, 0.75);
			//QPainterPath pp = m.map(painterPath);
			//m_connector0->setShape(pp);  // can this be made hollow?

			//QPainterPathStroker stroker;
			//stroker.setWidth(10);
			//QPainterPath pp = stroker.createStroke(painterPath);
			//m_connector0->setShape(pp);

			m_connector0->setShape(painterPath);
		}
		this->setShape(painterPath);  

	}
}

QString GroundPlane::svg() {
	return modelPart()->prop("svg").toString();
}

bool GroundPlane::hasPartLabel() {
	
	return false;
}
