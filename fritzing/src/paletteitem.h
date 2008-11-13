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

$Revision$:
$Author$:
$Date$

********************************************************************/


#ifndef PALETTEITEM_H
#define PALETTEITEM_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>

#include "paletteitembase.h"
#include "viewlayer.h"

class PaletteItem : public PaletteItemBase {
public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <reanderImage>
	PaletteItem(ModelPart *, ItemBase::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu);

	QPixmap * pixmap() const;
	void removeLayerKin();
	void addLayerKin(class LayerKinPaletteItem * lkpi);
	QList<class LayerKinPaletteItem *> & layerKin();
 	void loadLayerKin(const LayerHash & viewLayers);
	void rotateItemAnd(qreal degrees);
	void flipItemAnd(Qt::Orientations orientation);
	void moveItem(ViewGeometry & viewGeometry);
	void setItemPos(QPointF & pos);
	ItemBase * layerKinChief();
	void sendConnectionChangedSignal(ConnectorItem * from, ConnectorItem * to, bool connect);

	bool renderImage(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool renderPixmap, bool doConnectors);

	void restoreConnections(QDomElement & instance, QHash<long, ItemBase *> & newItems);
	void setTransforms();
	void syncKinMoved(QPointF offset, QPointF loc);

	void setInstanceTitleAndTooltip(const QString&);
	void updateTooltip();

	bool swap(PaletteItem* other, const LayerHash &layerHash);
	bool swap(ModelPart* newModelPart, const LayerHash &layerHash);
	QString family();

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void syncKinSelection(bool selected, PaletteItemBase * originator);
 	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateConnections();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void invalidateConnectors();
	void cleanupConnectors();

 protected:
 	QPixmap *m_pixmap;
 	QList<class LayerKinPaletteItem *> m_layerKin;

};

#endif
