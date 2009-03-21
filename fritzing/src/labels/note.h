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

#ifndef NOTE_H
#define NOTE_H

#include <QGraphicsTextItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QXmlStreamWriter>
#include <QDomElement>
#include <QKeyEvent>
#include <QDialog>
#include <QLineEdit>

#include "../itembase.h"

class Note : public ItemBase
{
Q_OBJECT

public:
	Note(class ModelPart*, ViewIdentifierClass::ViewIdentifier, const ViewGeometry &, long id, QMenu * itemMenu);
	
	void saveGeometry();
	bool itemMoved();
	void saveInstanceLocation(QXmlStreamWriter &);
	void moveItem(ViewGeometry &);
	void findConnectorsUnder();
	bool resizing();
	void setText(const QString & text);
	void setText(const QDomElement & textElement);
	QString text();
	void setSize(const QSizeF & size);
	void setHidden(bool hide) ;

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void positionGrip();
	void mousePressEvent ( QGraphicsSceneMouseEvent * event );
	void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	bool eventFilter(QObject * object, QEvent * event);

protected slots:
	void contentsChangedSlot();
	void linkDialog();

public:
	static QString moduleIDName;
	static const int emptyMinWidth;
	static const int emptyMinHeight;
	static const int initialMinWidth;
	static const int initialMinHeight;
	static QString initialTextString;

protected:
	QRectF m_rect;
    QPen m_pen;
    QBrush m_brush;
	QGraphicsPixmapItem * m_resizeGrip;
	bool m_inResize;
	QPointF m_resizePos;
	QGraphicsTextItem * m_graphicsTextItem;

};

class LinkDialog : public QDialog
{
Q_OBJECT

public:
	LinkDialog(QWidget *parent = 0);
	~LinkDialog();

	void setUrl(const QString &);
	void setText(const QString &);
	QString text();
	QString url();

protected:
	QLineEdit * m_urlEdit;
	QLineEdit * m_textEdit;

};

#endif
