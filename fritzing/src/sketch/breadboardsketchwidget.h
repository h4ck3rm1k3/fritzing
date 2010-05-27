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



#ifndef BREADBOARDSKETCHWIDGET_H
#define BREADBOARDSKETCHWIDGET_H

#include "sketchwidget.h"

class BreadboardSketchWidget : public SketchWidget
{
	Q_OBJECT

public:
    BreadboardSketchWidget(ViewIdentifierClass::ViewIdentifier, QWidget *parent=0);

	void addViewLayers();
	void initWire(Wire *, int penWidth);
	bool canDisconnectAll();
	bool ignoreFemale();

protected slots:
	void schematicDisconnectWireSlot(ConnectorPairHash & moveItems, QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, QUndoCommand * parentCommand);

protected:
	void setWireVisible(Wire * wire);
	void collectFemaleConnectees(ItemBase *, QSet<ItemBase *> &);
	void findConnectorsUnder(ItemBase * item);
	bool disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, ConnectorPairHash &, bool doCommand, QUndoCommand * parentCommand);
	BaseCommand::CrossViewType wireSplitCrossView();
	bool shareBreadboard(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, ItemBase * & breadboardItemBase);
	bool canDropModelPart(ModelPart * modelPart); 
	void translateToLocalItems(ConnectorPairHash & foreignMoveItems, ConnectorPairHash & moveItems,	QMultiHash<PaletteItemBase *, ConnectorItem *> & bases);
	bool allowFemaleRotation(ItemBase *);
	void getLabelFont(QFont &, QColor &, ViewLayer::ViewLayerSpec);
	void setNewPartVisible(ItemBase *);
	double defaultGridSizeInches();
	ViewLayer::ViewLayerID getLabelViewLayerID(ViewLayer::ViewLayerSpec);

};

#endif
