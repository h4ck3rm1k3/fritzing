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

#ifndef GROUPITEMBASE_H
#define GROUPITEMBASE_H

#include <QGraphicsItemGroup>
#include <QVariant>
#include <QPainter>

#include "../itembase.h"

class GroupItemBase : public ItemBase
{
	Q_OBJECT

public:
	GroupItemBase(ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu);

	void addToGroup(ItemBase *);
	virtual void syncKinMoved(GroupItemBase *, QPointF newPos);
	virtual void doneAdding(const LayerHash &);

	void findConnectorsUnder();
	void saveGeometry();
	bool itemMoved();
	void saveInstanceLocation(QXmlStreamWriter &);
	void moveItem(ViewGeometry &);
	QRectF boundingRect() const;
	void collectWireConnectees(QSet<class Wire *> & wires);
	void collectFemaleConnectees(QSet<ItemBase *> & items);
	void collectExternalConnectorItems();
	void collectConnectors(QList<ConnectorItem *> & connectors);

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addToGroup(ItemBase *, const LayerHash &);


protected:
	QRectF m_boundingRect;
	QList<ItemBase *> m_itemsToAdd;
	QList<class ConnectorItem *> m_externalConnectorItems;
};


#endif
