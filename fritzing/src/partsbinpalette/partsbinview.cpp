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


#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>

#include "partsbinview.h"
#include "../itemdrag.h"
#include "../utils/misc.h"

void PartsBinView::setPaletteModel(PaletteModel * model, bool clear) {
	if(clear) {
		doClear();
	}

	if (model->root() == NULL) return;

	setItemAux(model->root());
	setItem(model->root());
}

void PartsBinView::reloadParts(PaletteModel * model) {
	setPaletteModel(model, true);
}

void PartsBinView::doClear() {
	m_partHash.clear();
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

void PartsBinView::addPart(ModelPart * model, int position) {
	setItemAux(model, position);
}

void PartsBinView::mousePressOnItem(const QPoint &dragStartPos, const QString &moduleId, const QSize &size, const QPointF &dataPoint, const QPoint &hotspot) {
	m_dragStartPos = dragStartPos;

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);

	dataStream << moduleId << dataPoint;

	QMimeData *mimeData = new QMimeData;
	mimeData->setData("application/x-dnditemdata", itemData);
	mimeData->setData("action", "part-reordering");

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


	drag->exec();

	/*if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction) {
	}
	else {
	}*/

	ItemDrag::_dragIsDone();
}

void PartsBinView::dragMoveEventAux(QDragMoveEvent* event) {
	// Only accept if it's an icon-reordering request
	const QMimeData* m = event->mimeData();
	QStringList formats = m->formats();
	if (formats.contains("action") && (m->data("action") == "part-reordering")) {
		event->acceptProposedAction();
	}
}

void PartsBinView::dropEventAux(QDropEvent* event) {
	int fromIndex = itemIndexAt(m_dragStartPos);
	int toIndex = itemIndexAt(event->pos());

	moveItem(fromIndex,toIndex);
	event->acceptProposedAction();
}

bool PartsBinView::alreadyIn(QString moduleID) {
	return m_partHash.contains(moduleID);
}

void PartsBinView::setInfoViewOnHover(bool infoViewOnHover) {
	m_infoViewOnHover = infoViewOnHover;
}
