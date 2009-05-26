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

$Revision: 1617 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-22 20:32:44 +0100 (Sat, 22 Nov 2008) $

********************************************************************/

#include "schematicsketchwidget.h"
#include "autoroute/autorouter1.h"
#include "debugdialog.h"
#include "items/virtualwire.h"
#include "connectoritem.h"

#include <limits>

static QColor schematicColor;

SchematicSketchWidget::SchematicSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : PCBSketchWidget(viewIdentifier, parent)
{
	m_viewName = QObject::tr("Schematic View");
	m_traceColor = "black";
}

void SchematicSketchWidget::addViewLayers() {
	addSchematicViewLayers();
}

ViewLayer::ViewLayerID SchematicSketchWidget::getWireViewLayerID(const ViewGeometry & viewGeometry) {
	if (viewGeometry.getTrace()) {
		return ViewLayer::SchematicTrace;
	}

	return SketchWidget::getWireViewLayerID(viewGeometry);
}

void SchematicSketchWidget::initWire(Wire * wire, int penWidth) {
	Q_UNUSED(penWidth);
	wire->setColorString("schematicGrey", Wire::UNROUTED_OPACITY);
	wire->setPenWidth(2);
}

const QColor * SchematicSketchWidget::getRatsnestColor() 
{
	if (!schematicColor.isValid()) {
		Wire::getColor(schematicColor, "schematicGrey");
	}
	return &schematicColor;
}
