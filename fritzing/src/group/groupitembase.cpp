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

$Revision: 2085 $:
$Author: cohen@irascible.com $:
$Date: 2009-01-06 12:15:02 +0100 (Tue, 06 Jan 2009) $

********************************************************************/

#include "groupitembase.h"


GroupItemBase::GroupItemBase( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, bool topLevel, QMenu * itemMenu) 
	: ItemBase( modelPart, viewIdentifier, viewGeometry, id, topLevel, itemMenu)
{
	this->setVisible(true);
	this->setFlag(QGraphicsItem::ItemIsSelectable);

	m_graphicsItemGroup = new FGraphicsItemGroup();
	m_graphicsItemGroup->setParentItem(this);
	m_graphicsItemGroup->setVisible(true);
	m_graphicsItemGroup->setFlag(QGraphicsItem::ItemIsSelectable);
}

void GroupItemBase::addToGroup(ItemBase * item, const LayerHash & layerHash) {
	if (m_zUninitialized) {
		this->setViewLayerID(item->viewLayerID(), layerHash);
		setZValue(this->z());
	}


	m_graphicsItemGroup->addToGroup(item);
}

void GroupItemBase::findConnectorsUnder() {
}

void GroupItemBase::saveGeometry() {
	m_viewGeometry.setLoc(this->pos());
}

bool GroupItemBase::itemMoved() {
	return (this->pos() != m_viewGeometry.loc());
}

void GroupItemBase::saveInstanceLocation(QXmlStreamWriter & streamWriter) {
}

void GroupItemBase::moveItem(ViewGeometry & viewGeometry) {
	this->setPos(viewGeometry.loc());
}

void GroupItemBase::rotateItem(qreal degrees) {
	Q_UNUSED(degrees);
}

void GroupItemBase::syncKinMoved(GroupItemBase * originator, QPointF newPos) {
	Q_UNUSED(newPos);
	Q_UNUSED(originator);
}

//////////////////////////////////////////////////

FGraphicsItemGroup::FGraphicsItemGroup() 
	: QGraphicsItemGroup()
{
}

QVariant FGraphicsItemGroup::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value)
{
	switch (change) {
		case QGraphicsItem::ItemSelectedChange:
			this->parentItem()->setSelected(value.toBool());
			break;
		default:
			break;
	}

	return QGraphicsItemGroup::itemChange(change, value);
}