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

#ifndef LAYERKINPALETTEITEM_H
#define LAYERKINPALETTEITEM_H

#include "paletteitembase.h"
#include <QVariant>

class LayerKinPaletteItem : public PaletteItemBase
{
Q_OBJECT

public:       
	LayerKinPaletteItem(PaletteItemBase * chief, ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu);
	void setOffset(qreal x, qreal y);
	ItemBase * layerKinChief();
	bool ok();
	void setHidden(bool hidden);
	void clearModelPart();
	ItemBase * lowerConnectorLayerVisible(ItemBase *);
	void init(ViewLayer::ViewLayerID viewLayerID, const LayerHash &viewLayers);
	QString toolTip2();
	bool sticky();
	void setSticky(bool);
	void addSticky(ItemBase *, bool stickem);
	ItemBase * stickingTo();
	QList<ItemBase *> & stickyList();
	bool alreadySticking(ItemBase * itemBase);
	bool stickyEnabled();
	void resetID();
	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateConnections();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

protected:
	QPointer<PaletteItemBase> m_layerKinChief;
	bool m_ok;
};

#endif
