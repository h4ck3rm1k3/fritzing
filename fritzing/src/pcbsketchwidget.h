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



#ifndef PCBSKETCHWIDGET_H
#define PCBSKETCHWIDGET_H

#include "pcbschematicsketchwidget.h"

class PCBSketchWidget : public PCBSchematicSketchWidget
{
	Q_OBJECT

public:
    PCBSketchWidget(ViewIdentifierClass::ViewIdentifier, QWidget *parent=0);

	void addViewLayers();
	bool canDeleteItem(QGraphicsItem * item);
	bool canCopyItem(QGraphicsItem * item);
	void createJumper();
	void createTrace();
	void excludeFromAutoroute(bool exclude);
	bool ratsAllRouted();
	void selectAllWires(ViewGeometry::WireFlag);
	void selectAllExcludedTraces();
	void makeChangeRoutedCommand(Wire * wire, bool routed, qreal opacity, QUndoCommand * parentCommand);
	void clearRouting(QUndoCommand * parentCommand);
	void updateRatsnestStatus(CleanUpWiresCommand*, QUndoCommand *);
	void forwardRoutingStatusSignal(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumperCount);


protected:
	void setWireVisible(Wire * wire);
	void makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire, RatsnestCommand *);
	// void checkAutorouted();
	ViewLayer::ViewLayerID multiLayerGetViewLayerID(ModelPart * modelPart, QString & layerName);
	bool canChainWire(Wire *);
	void createJumperOrTrace(const QString & commandString, ViewGeometry::WireFlag, const QString & colorString);
	void createOneJumperOrTrace(Wire * wire, ViewGeometry::WireFlag flag, bool allowAny, QList<Wire *> & done, 
								QUndoCommand * & parentCommand, const QString & commandString, const QString & colorString);
	const QColor & getLabelTextColor();
	const QString & hoverEnterPartConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	bool modifyNewWireConnections(Wire * dragWire, ConnectorItem * fromOnWire, ConnectorItem * from, ConnectorItem * to, QUndoCommand * parentCommand);



protected:
	int m_netCount;
	int m_netRoutedCount;
	int m_connectorsLeftToRoute;
	int m_jumperCount;

};

#endif
