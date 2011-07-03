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


#ifndef SCHEMATICSKETCHWIDGET_H
#define SCHEMATICSKETCHWIDGET_H

#include "pcbsketchwidget.h"

class SchematicSketchWidget : public PCBSketchWidget
{
	Q_OBJECT

public:
    SchematicSketchWidget(ViewIdentifierClass::ViewIdentifier, QWidget *parent=0);

	void addViewLayers();
	ViewLayer::ViewLayerID getWireViewLayerID(const ViewGeometry & viewGeometry, ViewLayer::ViewLayerSpec);
	ViewLayer::ViewLayerID getDragWireViewLayerID(ConnectorItem *);
	void initWire(Wire *, int penWidth);
	bool autorouteTypePCB();
	qreal getKeepout();
	void tidyWires();
	void ensureTraceLayersVisible();
	void ensureTraceLayerVisible();
	bool usesJumperItem();
	void setClipEnds(ClipableWire * vw, bool);
	void getBendpointWidths(class Wire *, qreal w, qreal & w1, qreal & w2, bool & negativeOffsetRect);
	void getLabelFont(QFont &, QColor &, ViewLayer::ViewLayerSpec);
	void setNewPartVisible(ItemBase *);
	bool canDropModelPart(ModelPart * modelPart); 
	bool includeSymbols();
	bool hasBigDots();
	void changeConnection(long fromID,
						  const QString & fromConnectorID,
						  long toID, const QString & toConnectorID,
						  ViewLayer::ViewLayerSpec,
						  bool connect, bool doEmit, 
						  bool updateConnections);
	double defaultGridSizeInches();
	const QString & traceColor(ConnectorItem * forColor);
	const QString & traceColor(ViewLayer::ViewLayerSpec);
	long setUpSwap(ItemBase *, long newModelIndex, const QString & newModuleID,  ViewLayer::ViewLayerSpec, bool doEmit, bool noFinalChangeWiresCommand, QUndoCommand * parentCommand);
	bool isInLayers(ConnectorItem *, ViewLayer::ViewLayerSpec);
	bool routeBothSides();
	void addDefaultParts();
	bool sameElectricalLayer2(ViewLayer::ViewLayerID, ViewLayer::ViewLayerID);
	bool acceptsTrace(const ViewGeometry &);
	ViewGeometry::WireFlag getTraceFlag();
	qreal getTraceWidth();
	qreal getAutorouterTraceWidth();
	QString generateCopperFillUnit(ItemBase * itemBase, QPointF whereToStart);


public slots:
	void setVoltage(qreal voltage, bool doEmit);

protected slots:
	void updateBigDots();

protected:
	qreal getRatsnestOpacity(bool);
	AddItemCommand * newAddItemCommand(BaseCommand::CrossViewType crossViewType, 
										QString moduleID, ViewLayer::ViewLayerSpec, ViewGeometry & viewGeometry, qint64 id, 
										bool updateInfoView, long modelIndex, QUndoCommand *parent);
	ViewLayer::ViewLayerID getLabelViewLayerID(ViewLayer::ViewLayerSpec);
	QPoint calcFixedToCenterItemOffset(const QRect & viewPortRect, const QSizeF & helpSize);
	void extraRenderSvgStep(ItemBase *, QPointF offset, qreal dpi, qreal printerScale, QString & outputSvg);
	QString makeCircleSVG(QPointF p, qreal r, QPointF offset, qreal dpi, qreal printerScale);


protected:
	QTimer m_updateDotsTimer;

};

#endif
