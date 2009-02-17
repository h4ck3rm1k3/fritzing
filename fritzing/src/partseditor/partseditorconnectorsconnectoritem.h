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

$Revision: 2248 $:
$Author: merunga $:
$Date: 2009-01-22 19:47:17 +0100 (Thu, 22 Jan 2009) $

********************************************************************/


#ifndef PARTSEDITORCONNECTORSCONNECTORITEM_H_
#define PARTSEDITORCONNECTORSCONNECTORITEM_H_

#include "partseditorconnectoritem.h"
#include "../itemselection/resizablerectitem.h"

class PartsEditorConnectorsConnectorItem : public PartsEditorConnectorItem, public ResizableRectItem {
friend class ConnectorRectangle;
public:
	PartsEditorConnectorsConnectorItem(Connector * conn, ItemBase* attachedTo, bool showingTerminalPoint);
	PartsEditorConnectorsConnectorItem(Connector * conn, ItemBase* attachedTo, bool showingTerminalPoint, const QRectF &bounds);
	~PartsEditorConnectorsConnectorItem();

	void highlight(const QString &connId);
	void setConnector(Connector *connector);
	void setMismatching(bool isMismatching);

	void setShowTerminalPoint(bool show);
	bool showingTerminalPoint();
	void setTerminalPoint(QPointF);
	void resetTerminalPoint();
	void updateTerminalPoint();
	TerminalPointItem *terminalPointItem();

	qreal minWidth();
	qreal minHeight();

	QRectF mappedRect();

protected:
	void init(bool resizable);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

	void resizeRect(qreal x, qreal y, qreal width, qreal height);

	void removeErrorIcon();
	void addErrorIcon();
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	void drawDottedRect(QPainter *painter, const QColor &color1, const QColor &color2, const QRectF &rect);
	QPen drawDottedLine(
		Qt::Orientations orientation, QPainter *painter, const QPen &pen1, const QPen &pen2,
		qreal pos1, qreal pos2, qreal fixedAxis, const QPen &lastUsedPen = QPen()
	);
	QPen drawDottedLineAux(
		Qt::Orientations orientation, QPainter *painter, const QPen &firstPen, const QPen &secondPen,
		qreal pos, qreal fixedAxis, qreal dotSize, int dotCount
	);

	void informChange();

	QGraphicsSvgItem *m_errorIcon;

	bool m_showingTerminalPoint; // important only if m_showsTerminalPoints == true

	TerminalPointItem *m_terminalPointItem;

	QRectF m_resizedRect;
	bool m_geometryHasChanged;
	bool m_inFileDefined;
	bool m_centerHasChanged;

	static qreal MinWidth;
	static qreal MinHeight;
};

#endif /* PARTSEDITORCONNECTORSCONNECTORITEM_H_ */
