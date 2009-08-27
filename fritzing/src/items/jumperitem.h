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

$Revision: 2986 $:
$Author: cohen@irascible.com $:
$Date: 2009-05-21 11:21:17 +0200 (Thu, 21 May 2009) $

********************************************************************/

#ifndef JUMPERITEM_H
#define JUMPERITEM_H

#include "wire.h"

class JumperItem : public Wire
{

public:
	JumperItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier,  const ViewGeometry & , long id, QMenu* itemMenu  ); 

    QPainterPath shape() const;
    QPainterPath hoverShape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	class FSvgRenderer * setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash & viewLayers, class InfoGraphicsView *);
    void setLine(const QLineF &line);

protected:
	void initEnds(const ViewGeometry &, QRectF defaultRect, class InfoGraphicsView *);
};

#endif
