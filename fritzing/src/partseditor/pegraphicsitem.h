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



#ifndef PEGRAPHICSITEM_H_
#define PEGRAPHICSITEM_H_

#include <QGraphicsRectItem>
#include <QDomElement>
#include <QGraphicsSceneHoverEvent>

class PEGraphicsItem : public QObject, public QGraphicsRectItem 
{
    Q_OBJECT
public:
	PEGraphicsItem(double x, double y, double width, double height);

	void hoverEnterEvent(QGraphicsSceneHoverEvent *);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    void wheelEvent(QGraphicsSceneWheelEvent *);
    void setHighlighted(bool);
    bool highlighted();
    void setElement(QDomElement &);
    QDomElement & element();
    void setOffset(QPointF);
    QPointF offset();

signals:
    void highlightSignal(PEGraphicsItem *);

protected:
    bool m_highlighted;
    QDomElement  m_element;
    QPointF m_offset;
};

#endif /* PEGRAPHICSITEM_H_ */
