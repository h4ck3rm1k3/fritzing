#ifndef LAYERKINPALETTEITEM_H
#define LAYERKINPALETTEITEM_H

#include "paletteitembase.h"
#include <QSvgRenderer>
#include <QVariant>

class LayerKinPaletteItem : public PaletteItemBase
{
Q_OBJECT
public:
	LayerKinPaletteItem(PaletteItemBase * chief, ModelPart *, ItemBase::ViewIdentifier, const ViewGeometry & viewGeometry, long id,
						ViewLayer::ViewLayerID viewLayer, QMenu * itemMenu, const LayerHash & viewLayers);
	void setOffset(qreal x, qreal y);
	ItemBase * layerKinChief();
	bool ok();

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateConnections();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);


protected:
	PaletteItemBase * m_layerKinChief;
	bool m_ok;
};

#endif
