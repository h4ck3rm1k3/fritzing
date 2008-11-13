/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef CONNECTORVIEWTHING_H
#define CONNECTORVIEWTHING_H

#include "viewthing.h"
#include "viewlayer.h"
#include "itembase.h"

class ConnectorViewThing : public ViewThing
{

public:
	ConnectorViewThing();
	
	void setVisibleInView(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID, bool);
	void setVisibleInView(ItemBase::ViewIdentifier, bool);
	bool visibleInView(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID);
	QRectF rectInView(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID);
	void setRectInView(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID, QRectF);
	void setProcessed(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID, bool);
	bool processed(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID);
	QPointF terminalPointInView(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID);
	void setTerminalPointInView(ItemBase::ViewIdentifier, ViewLayer::ViewLayerID, QPointF);
	
protected:
	bool m_visible[ItemBase::ViewCount][ViewLayer::ViewLayerCount];
	bool m_processed[ItemBase::ViewCount][ViewLayer::ViewLayerCount];
	QRectF m_rect[ItemBase::ViewCount][ViewLayer::ViewLayerCount];		
	QPointF m_point[ItemBase::ViewCount][ViewLayer::ViewLayerCount];	
};

#endif
