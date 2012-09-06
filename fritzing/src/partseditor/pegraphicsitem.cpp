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


#include "pegraphicsitem.h"
#include "../debugdialog.h"

#include <QBrush>
#include <QColor>

PEGraphicsItem::PEGraphicsItem(double x, double y, double w, double h) : QGraphicsRectItem(x, y, w, h) {
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptHoverEvents(true);
    //setFlag(QGraphicsItem::ItemIsSelectable, true );
    setOpacity(0.001);
    setBrush(QBrush(QColor(0, 0, 255)));
}

void PEGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
	setOpacity(0.4);
    update();
}

void PEGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
	setOpacity(0.001);
    update();
}

void PEGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent * event) {
    DebugDialog::debug("wheel event");
}

