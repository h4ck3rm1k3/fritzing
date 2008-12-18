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



#ifndef PARTSEDITORCONNECTORITEM_H_
#define PARTSEDITORCONNECTORITEM_H_

#include "../connectoritem.h"


class PartsEditorConnectorItem: public ConnectorItem {
	public:
		PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo);
		PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo, const QRectF &bounds);
		void highlight(const QString &connId);
		void removeFromModel();
		void setConnector(Connector *connector);
		void setMismatching(bool isMismatching);

	protected:
		void setSelectedColor(const QColor &color = selectedColor);
		void setNotSelectedColor(const QColor &color = notSelectedColor);
		void removeErrorIcon();
		void addErrorIcon();
		void addBorder();
		void removeBorder();
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
		void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
		void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
		void mousePressEvent(QGraphicsSceneMouseEvent *event);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

		/*Qt::Corner*/ int updateCursor(const QPointF &mousePos, const QCursor &defaultCursor=QCursor());
		/*Qt::Corner*/ int closeToCorner(const QPointF &pos);

		QGraphicsSvgItem *m_errorIcon;
		bool m_withBorder;
		bool m_resizable;
		bool m_resizing;
	public:
		static QColor selectedColor;
		static QColor notSelectedColor;
		static QColor selectedPenColor;
		static qreal selectedPenWidth;
};

#endif /* PARTSEDITORCONNECTORITEM_H_ */
