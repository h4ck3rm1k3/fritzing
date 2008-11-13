#ifndef VIRTUALWIRE_H
#define VIRTUALWIRE_H

#include "wire.h"

class VirtualWire : public Wire
{

public:
	VirtualWire( ModelPart * modelPart, ItemBase::ViewIdentifier,  const ViewGeometry & , long id, QMenu* itemMenu  ); 
	
	void setHidden(bool hidden);
	void tempRemoveAllConnections();
	void setChained(ConnectorItem * item, bool chained);
	
protected:
	void paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget );	
	void connectionChange(ConnectorItem *);
 	QSvgRenderer * setUpConnectors(class ModelPart *, ItemBase::ViewIdentifier);
	void hideConnectors();	
};

#endif
