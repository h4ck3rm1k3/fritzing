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

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef PARTLABEL_H
#define PARTLABEL_H

#include <QGraphicsTextItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QXmlStreamWriter>
#include <QDomElement>
#include <QTextDocument>
#include <QKeyEvent>
#include <QPointer>

#include "../viewlayer.h"

class PartLabelTextDocument : public QTextDocument
{
public:
	PartLabelTextDocument(long id, QObject * parent = 0);

	void addRef();
	void decRef();

protected:
	int m_refCount;
	long m_id;

protected:
	static QHash<long, PartLabelTextDocument *> AllTextDocuments;


	friend class PartLabel;
};


class PartLabel : public QGraphicsTextItem
{
 Q_OBJECT

public:
	PartLabel(class ItemBase * owner, const QString & text, QGraphicsItem * parent = 0 );   // itembase is not the parent
	~PartLabel();

	void showLabel(bool showIt, ViewLayer *, const QColor & textColor);
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void setPlainText(const QString & text);
	bool initialized();
	void ownerMoved(QPointF newPos);
	void setHidden(bool hide);
	bool hidden();
	ViewLayer::ViewLayerID viewLayerID();
	void saveInstance(QXmlStreamWriter & streamWriter);
	void restoreLabel(QDomElement & labelGeometry, ViewLayer::ViewLayerID);
	void moveLabel(QPointF newPos, QPointF newOffset);
	class ItemBase * owner();
	void rotateFlipLabel(qreal degrees, Qt::Orientations orientation);

protected:
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
	void keyPressEvent(QKeyEvent * event);
	void keyReleaseEvent(QKeyEvent * event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void temporaryMenuEvent(QGraphicsSceneMouseEvent * event);
	void transformLabel(QTransform currTransf);
	void focusInEvent(QFocusEvent * event);
	void focusOutEvent(QFocusEvent * event);
	bool eventFilter(QObject * object, QEvent * event);


protected slots:
	void contentsChangedSlot();

protected:
	QPointer<class ItemBase> m_owner;
	bool m_initialized;
	bool m_spaceBarWasPressed;
	bool m_doDrag;
	QPointF m_initialPosition;
	QPointF m_initialOffset;
	QPointF m_offset;
	ViewLayer::ViewLayerID m_viewLayerID;
	bool m_hidden;
};

#endif
