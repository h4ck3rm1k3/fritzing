/*
 * (c) Fachhochschule Potsdam
 */

#include <QMimeData>
#include <QDrag>

#include "partsbinview.h"
#include "../itemdrag.h"

void PartsBinView::setPaletteModel(PaletteModel * model, bool clear) {
	if(clear) {
		doClear();
	}

	setModel(model);

	if (model->root() == NULL) return;

	setItemAux(model->root());
	setItem(model->root());
}

void PartsBinView::reloadParts(PaletteModel * model) {
	setPaletteModel(model, true);
}

void PartsBinView::setItem(ModelPart * modelPart) {
	QList<QObject *>::const_iterator i;
    for (i = modelPart->children().constBegin(); i != modelPart->children().constEnd(); ++i) {
		setItemAux(dynamic_cast<ModelPart *>(*i));
	}
    for (i = modelPart->children().constBegin(); i != modelPart->children().constEnd(); ++i) {
		setItem(dynamic_cast<ModelPart *>(*i));
	}
}

void PartsBinView::addPart(ModelPart * model) {
	setItemAux(model);
}

void PartsBinView::mousePressOnItem(const QString &moduleId, const QSize &size, const QPointF &dataPoint, const QPoint &hotspot) {
	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);

	dataStream << moduleId << dataPoint;

	QMimeData *mimeData = new QMimeData;
	mimeData->setData("application/x-dnditemdata", itemData);

	QDrag * drag = new QDrag(dynamic_cast<QWidget*>(this));

	drag->setMimeData(mimeData);

	QPixmap pixmap(size);
	pixmap.fill(Qt::transparent);
	QPainter painter;
	painter.begin(&pixmap);
	QPen pen(QColor(0,0,0,127));
	pen.setStyle(Qt::DashLine);
	pen.setWidth(1);
	painter.setPen(pen);
	painter.drawRect(0,0,pixmap.width() - 1, pixmap.height() - 1);
	painter.end();
	drag->setPixmap(pixmap);
	drag->setHotSpot(hotspot);

	// can set the pixmap, but can't hide it
	//QPixmap * pixmap = pitem->pixmap();
	//if (pixmap != NULL) {
		//drag.setPixmap(*pixmap);
		//drag.setHotSpot(mts.toPoint() - pitem->pos().toPoint());
	//

	// setDragCursor doesn't seem to help
	//drag.setDragCursor(*pitem->pixmap(), Qt::MoveAction);
	//drag.setDragCursor(*pitem->pixmap(), Qt::CopyAction);
	//drag.setDragCursor(*pitem->pixmap(), Qt::LinkAction);
	//drag.setDragCursor(*pitem->pixmap(), Qt::IgnoreAction);


	if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction) {
	}
	else {
	}

	ItemDrag::_dragIsDone();
}
