/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "legitem.h"
#include "../debugdialog.h"
#include "../items/itembase.h"

#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>


/////////////////////////////////////////////////////////


/*

TODO:

	bendableleg="yes" stroke="#6D6D6D" stroke-width="0.0347in"

	* show connectors under

	* undo moves & connections

	* adjust position so that connectorItem is in the center of connected-to connectorItem
		original connector probably needs to be a circle, or it should become circular when the line is stretched

	* save and load

	* alt/meta/ctrl to drag out a wire
		the leg is no longer bendable because the wire covers the drag area

	* export

	* bendable drag

	* update connections needs to be smarter (has to to with connecting to wires)
		look again at attachedMoved()

	* arrow moves

	* delete/undo delete

	* hover: trigger the usual part hover highlight

	* click selection behavior should be as if selecting the part

	move behavior: freeze the leg in the current position and drag the rest of the part along
		this may break a connection for the captured leg but not the other

	hover: the legs should highlight somehow: 
		how about a grey area like the rest of the part, but just around the legs, not filling in the whole space

	drag selection should work as normal

	bendable drag when part is stretched between two or more parts, 
			some not being dragged correctly

	swapping parts with bendable legs, can assume pins will always line up (unless legs can have diffent max distances)

	rotate/flip
		do not disconnect
		probably needs a matrix inversion, or can I just use mapToScene & mapToParent in order to hook the legs back up?

	clean up pixel turds


	fzp just has bendable and max length in units
		put the leg definition as a line in the svg, with connectorNleg
		then on loading, remove the leg, and change the viewbox height and width
		then draw the leg as now
		have to figure out which end is the anchor

	lots of new parts to define


-------------------------------------------------

bendable drag with auto-disconnect after a certain length is reached

parts editor support

*/

/////////////////////////////////////////////////////////

LegItem::LegItem(QGraphicsItem * parent) : QGraphicsLineItem(parent)
{
	setLine(0, 0, 0, 0);
	setVisible(true);
	setFlag(QGraphicsItem::ItemIsSelectable, false);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setAcceptedMouseButtons(ALLMOUSEBUTTONS);
	setAcceptHoverEvents(true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	this->setZValue(-999);			// keep the connector item above this
	
}

LegItem::~LegItem()
{
}

bool LegItem::sceneEvent(QEvent *event)
{
	if (remapItemPos(event, parentItem())) {
		return ((ItemBase *) parentItem())->sceneEvent(event);
	}

	return QGraphicsLineItem::sceneEvent(event);	
}

bool LegItem::remapItemPos(QEvent *event, QGraphicsItem *item)
{
	// copied from QGraphicsItemPrivate::remapItemPos

    switch (event->type()) {
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick: {
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        mouseEvent->setPos(item->mapFromItem(this, mouseEvent->pos()));
        mouseEvent->setLastPos(item->mapFromItem(this, mouseEvent->pos()));
        for (int i = 0x1; i <= 0x10; i <<= 1) {
            if (mouseEvent->buttons() & i) {
                Qt::MouseButton button = Qt::MouseButton(i);
                mouseEvent->setButtonDownPos(button, item->mapFromItem(this, mouseEvent->buttonDownPos(button)));
            }
        }
        return true;
    }
    case QEvent::GraphicsSceneWheel: {
        QGraphicsSceneWheelEvent *wheelEvent = static_cast<QGraphicsSceneWheelEvent *>(event);
        wheelEvent->setPos(item->mapFromItem(this, wheelEvent->pos()));
        return true;
    }
    case QEvent::GraphicsSceneContextMenu: {
        QGraphicsSceneContextMenuEvent *contextEvent = static_cast<QGraphicsSceneContextMenuEvent *>(event);
        contextEvent->setPos(item->mapFromItem(this, contextEvent->pos()));
        return true;
    }
    case QEvent::GraphicsSceneHoverMove: {
        QGraphicsSceneHoverEvent *hoverEvent = static_cast<QGraphicsSceneHoverEvent *>(event);
        hoverEvent->setPos(item->mapFromItem(this, hoverEvent->pos()));
        return true;
    }
    default:
        break;
    }

	return false;
}

