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

$Revision: 2189 $:
$Author: merunga $:
$Date: 2009-01-16 17:12:45 +0100 (Fri, 16 Jan 2009) $

********************************************************************/

#ifndef PARTSEDITORLAYERKINPALETTEITEM_H_
#define PARTSEDITORLAYERKINPALETTEITEM_H_

#include "partseditorconnectoritem.h"
#include "../layerkinpaletteitem.h"

class PartsEditorLayerKinPaletteItem : public LayerKinPaletteItem {
public:
	PartsEditorLayerKinPaletteItem(
		PaletteItemBase * chief, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier,const ViewGeometry & viewGeometry,
                long id, QMenu* itemMenu)
                : LayerKinPaletteItem(chief, modelPart, viewIdentifier, viewGeometry, id, itemMenu)
	{
	}
protected:
	ConnectorItem* newConnectorItem(Connector *connector) {
		return new PartsEditorConnectorItem(connector,this);
	}

	void hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
		setCursor(QCursor(Qt::OpenHandCursor));
		GraphicsSvgLineItem::hoverEnterEvent(event);
	}

	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
		GraphicsSvgLineItem::hoverLeaveEvent(event);
	}
};

#endif /* PARTSEDITORLAYERKINPALETTEITEM_H_ */
