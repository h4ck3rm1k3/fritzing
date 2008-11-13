/*
* (c) Fachhochschule Potsdam
*/

#include <QtGui>

#include "graphicsflowlayout.h"

GraphicsFlowLayout::GraphicsFlowLayout(QGraphicsLayoutItem *parent, int spacing)
	: QGraphicsLinearLayout(parent)
{
	setSpacing(spacing);
}

void GraphicsFlowLayout::widgetEvent(QEvent * e) {
	Q_UNUSED(e)
}

void GraphicsFlowLayout::setGeometry(const QRectF &rect) {
	QGraphicsLinearLayout::setGeometry(rect);
	doLayout(rect);
}

int GraphicsFlowLayout::heightForWidth(int width) {
	int height = doLayout(QRectF(0, 0, width, 0));
	return height;
}


int GraphicsFlowLayout::doLayout(const QRectF &rect) {
	qreal x = rect.x();
	qreal y = rect.y();
	qreal lineHeight = 0;

	for(int i=0; i < count(); i++) {
		QGraphicsLayoutItem* item = itemAt(i);
		int nextX = x + item->preferredSize().width() + spacing();
		if (nextX - spacing() > rect.right() && lineHeight > 0) {
			x = rect.x();
			y = y + lineHeight + spacing();
			nextX = x + item->preferredSize().width() + spacing();
			lineHeight = 0;
		}
		item->setGeometry(QRectF(QPoint(x, y), item->preferredSize()));

		x = nextX;
		lineHeight = qMax(lineHeight, item->preferredSize().height());
	}

	m_lastWidth = rect.width();
	return y + lineHeight - rect.y();
}
