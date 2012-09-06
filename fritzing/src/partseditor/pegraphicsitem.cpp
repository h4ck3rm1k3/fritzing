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
#include <QGraphicsScene>

PEGraphicsItem::PEGraphicsItem(double x, double y, double w, double h) : QGraphicsRectItem(x, y, w, h) {
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptHoverEvents(true);
    //setFlag(QGraphicsItem::ItemIsSelectable, true );
    setHighlighted(false);
    setBrush(QBrush(QColor(0, 0, 255)));
}

void PEGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
	setHighlighted(true);
}

void PEGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
	setHighlighted(false);
}

void PEGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent * event) {
    if (event->orientation() != Qt::Vertical) return;

    // delta one click forward = 120; delta one click backward = -120

    int steps = event->delta() / 120;
    QList<PEGraphicsItem *> items;
    foreach (QGraphicsItem * item, scene()->items(event->scenePos())) {
        PEGraphicsItem * pegi = dynamic_cast<PEGraphicsItem *>(item);
        if (pegi) items.append(pegi);
    }

    if (items.count() < 2) return;

    int ix = -1;
    int mix = -1;
    for (int i = 0; i < items.count(); i++) {
        if (items.at(i)->highlighted()) {
            ix = i;
            break;
        }
        if (items.at(i) == this) {
            mix = i;
        }
    }

    if (ix == -1) ix = mix;
    if (ix == -1) {
        // shouldn't happen
        return;
    }

    ix = (ix + steps) % items.count();
    if (ix < 0) {
        ix = items.count() + ix - 1;
    }
    
    items.at(ix)->setHighlighted(true);
}

void PEGraphicsItem::setHighlighted(bool highlighted) {
    if (highlighted) {
        m_highlighted = true;
        setOpacity(0.4);
        foreach (QGraphicsItem * item, scene()->items()) {
            PEGraphicsItem * pegi = dynamic_cast<PEGraphicsItem *>(item);
            if (pegi == NULL) continue;
            if (pegi == this) continue;
            if (!pegi->highlighted()) continue;
             
            pegi->setHighlighted(false);
        }
        emit highlightSignal(this);
    }
    else {
        m_highlighted = false;
        setOpacity(0.001);
    }
    update();
}

bool PEGraphicsItem::highlighted() {
    return m_highlighted;
}

void PEGraphicsItem::setElement(QDomElement & el)
{
    m_element = el;
}

QDomElement & PEGraphicsItem::element() {
    return m_element;
}

void PEGraphicsItem::setOffset(QPointF p) {
    m_offset = p;
}

QPointF PEGraphicsItem::offset() {
    return m_offset;
}
