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

	fzp hack for setting up a bendable leg:  bendableleg="yes" stroke="#6D6D6D" stroke-width="0.0347in"

	* show connectors under

	* undo moves & connections

	* adjust position so that connectorItem is in the center of connected-to connectorItem
		original connector probably needs to be a circle, or it should become circular when the line is stretched

	* save and load

	* alt/meta/ctrl to drag out a wire
		the leg is no longer bendable because the wire covers the drag area

	* bendable drag

	* update connections needs to be smarter (has to to with connecting to wires)
		look again at attachedMoved()

	* arrow key moves

	* delete/undo delete

	* hover: trigger the usual part hover highlight

	* click selection behavior should be as if selecting the part

	* drag selection should work as normal

	* clean up pixel turds

	* rotate/flip
		do not disconnect
		handle this as a post-facto undocommand
		should transform around center of the itemBase with no legs

	* make sure all leg functions work when itembase is rotated

	* bendable drag when part is stretched between two or more parts, some not being dragged correctly

	* swapping parts with bendable legs, can assume pins will always line up (unless legs can have diffent max distances)
		* no-no
		* no-yes
		* yes-no
		* yes-yes
		* original is rotated

	* if a part is locked, dragging the leg is disabled

	* fzp  has "legId" (and someday? "maxlength" or is that in the svg) in <p> element
		put the leg definition as a line in the svg, with connectorNleg
		then on loading, remove the leg, and change the viewbox height and width
			this is tricky.  Better if the leg extends outside the viewbox, then can easily tell which end is draggable
		then draw the leg as now

	figure out how to make the connector longer or its clickable area bigger, or if you click on the wire within a few pixels..

	bad crash when converting back to unbendable.  probably some kind of boundingRect issue...

	move behavior: what to do when dragging a leg?
		
	need some kind of fast disconnect

	export: retrieve svg probably won't clean up the bendable <line> element

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
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptHoverEvents(false);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
	this->setZValue(-999);			// keep the connector item above this
	DebugDialog::debug(QString("new leg %1").arg((long) dynamic_cast<QGraphicsItem *>(this), 0, 16));
}

LegItem::~LegItem()
{
	DebugDialog::debug(QString("deleting leg %1").arg((long) dynamic_cast<QGraphicsItem *>(this), 0, 16));
}

void LegItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// never draw selection outline
	Q_UNUSED(option);
	Q_UNUSED(widget);
    painter->setPen(pen());
    painter->drawLine(line());

}