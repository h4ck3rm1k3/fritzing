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

$Revision: 2879 $:
$Author: cohen@irascible.com $:
$Date: 2009-04-30 12:26:40 +0200 (Thu, 30 Apr 2009) $

********************************************************************/

#ifndef CLIPABLEWIRE_H
#define CLIPABLEWIRE_H

#include "wire.h"

class ClipableWire : public Wire
{

public:
	ClipableWire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier,  const ViewGeometry & , long id, QMenu* itemMenu  ); 
	
	void setClipEnds(bool);
	const QLineF & getPaintLine();	
	bool filterMousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

protected:
	QPointF findIntersection(ConnectorItem * connectorItem, QPointF original);
	virtual qreal calcClipRadius(ConnectorItem *);

protected:
	bool m_clipEnds;
	QLineF m_cachedLine;
	QLineF m_cachedOriginalLine;
	QGraphicsSceneMouseEvent * m_justFilteredEvent;
};

#endif
