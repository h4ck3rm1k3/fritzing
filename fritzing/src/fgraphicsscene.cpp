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

#include "fgraphicsscene.h"
#include "items/paletteitembase.h"
#include "connectors/connectoritem.h"
#include "sketch/infographicsview.h"

#include <QToolTip>

FGraphicsScene::FGraphicsScene( QObject * parent) : QGraphicsScene(parent)
{
	//setItemIndexMethod(QGraphicsScene::NoIndex);
}

void FGraphicsScene::helpEvent(QGraphicsSceneHelpEvent *helpEvent)
{
    // Find the first item that does tooltips
    QList<QGraphicsItem *> itemsAtPos = items(helpEvent->scenePos());
    QString text;
    QPoint point;
    for (int i = 0; i < itemsAtPos.size(); ++i) {
        QGraphicsItem *tmp = itemsAtPos.at(i);
		ItemBase * itemBase = dynamic_cast<ItemBase *>(tmp);
		if (itemBase == NULL) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(tmp);
			if (connectorItem && connectorItem->attachedTo()->hidden()) continue;

			if (!tmp->toolTip().isEmpty()) {
				text = tmp->toolTip();
				point = helpEvent->screenPos();
				break;
			}
		}
		else {
			if (itemBase->hidden()) continue;

			if (!itemBase->acceptHoverEvents()) continue;
		
			if (!itemBase->toolTip2().isEmpty()) {
				text = itemBase->toolTip2();
				point = helpEvent->screenPos();
				break;
			}
		}
    }

    // Show or hide the tooltip
    QToolTip::showText(point, text);

}

void FGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
	m_lastContextMenuPos = contextMenuEvent->scenePos();
	QGraphicsScene::contextMenuEvent(contextMenuEvent);
}

QPointF FGraphicsScene::lastContextMenuPos() {
	return m_lastContextMenuPos;
}

