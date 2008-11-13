/*
 * (c) Fachhochschule Potsdam
 */

#ifndef GRAPHICSFLOWLAYOUT_H_
#define GRAPHICSFLOWLAYOUT_H_

#include <QGraphicsLayoutItem>
#include <QGraphicsLinearLayout>
#include <QRect>
#include <QWidgetItem>

class GraphicsFlowLayout : public QGraphicsLinearLayout {
	public:
		GraphicsFlowLayout(QGraphicsLayoutItem *parent = 0, int spacing = 3);
		void setGeometry(const QRectF &rect);
		int heightForWidth(int width);

	protected:
		void widgetEvent(QEvent * e);
		int doLayout(const QRectF &rect);

		qreal m_lastWidth;
};

#endif /* GRAPHICSFLOWLAYOUT_H_ */
