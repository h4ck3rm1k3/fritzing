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
