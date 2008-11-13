#ifndef GRAPHICSSVGLINEITEM_H
#define GRAPHICSSVGLINEITEM_H

#include <QGraphicsSvgItem>
#include <QPen>
#include <QLine>
#include <QPainter>
#include <QStyleOptionGraphicsItem>


// combines QGraphicsLineItem and QGraphicsSvgItem so all parts and wires can inherit from the same class

class GraphicsSvgLineItem : public QGraphicsSvgItem
{
Q_OBJECT
public:
	GraphicsSvgLineItem(QGraphicsItem * parent = 0);
    ~GraphicsSvgLineItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QLineF line() const;
    void setLine(const QLineF &line);
    inline void setLine(qreal x1, qreal y1, qreal x2, qreal y2)
    	{ setLine(QLineF(x1, y1, x2, y2)); }

    QRectF boundingRect() const;
    QPainterPath shape() const;
    QPainterPath hoverShape() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

	bool hasLine();

protected:
	QLineF	m_line;
	QPen	m_pen;	
	bool	m_hasLine;
};

#endif
