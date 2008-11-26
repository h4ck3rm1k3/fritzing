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

$Revision: 1617 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-22 20:32:44 +0100 (Sat, 22 Nov 2008) $

********************************************************************/



#ifndef PCBSKETCHWIDGET_H
#define PCBSKETCHWIDGET_H

#include "pcbschematicsketchwidget.h"

class PCBSketchWidget : public PCBSchematicSketchWidget
{
	Q_OBJECT

public:
    PCBSketchWidget(ItemBase::ViewIdentifier, QWidget *parent=0, int size=500, int minSize=100);

	void addViewLayers(); 
	QString renderToSVG(qreal printerScale);

protected:
	void cleanUpWire(Wire * wire, QList<Wire *> & wires);
	void makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire);
	void checkAutorouted();
	ViewLayer::ViewLayerID multiLayerGetViewLayerID(ModelPart * modelPart, QString & layerName);
};

#endif
