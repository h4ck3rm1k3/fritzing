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

#ifndef GROUPITEM_H
#define GROUPITEM_H

#include "groupitembase.h"

class GroupItem : public GroupItemBase
{
	Q_OBJECT

public:
	GroupItem(ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu);

	void doneAdding(const LayerHash &, ViewLayer::ViewLayerID);
	const QList<ItemBase *> & layerKin();
	void syncKinMoved(GroupItemBase *, QPointF newPos);
	void rotateItem(qreal degrees);
	void flipItem(Qt::Orientations orientation);
	void collectWireConnectees(QSet<class Wire *> & wires);
	void collectFemaleConnectees(QSet<ItemBase *> & items);
	void removeLayerKin();
	void resetID();
	void clearModelPart();
	void moveItem(ViewGeometry & viewGeometry);
	void setTransforms();
	bool isLowerLayerVisible(GroupItemBase *);
	ItemBase * lowerConnectorLayerVisible(ItemBase *);
	void blockSyncKinMoved(bool block);
	void setHidden(bool hide);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateConnections();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void figureHover();
	void addToList(ItemBase * candidate, ViewLayer::ViewLayerID, const LayerHash &, qint64 & id);

protected:
	QList<ItemBase *> m_layerKin;
	bool m_blockSyncKinMoved;
	
};


#endif
