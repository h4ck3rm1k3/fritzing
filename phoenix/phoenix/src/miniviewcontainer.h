#ifndef MINIVIEWCONTAINER_H
#define MINIVIEWCONTAINER_H

#include <QWidget>
#include <QPaintEvent>

#include "miniview.h"

class MiniViewContainer : public QWidget
{
	Q_OBJECT
	
public:
	MiniViewContainer(QWidget * parent = 0);
	void setView(QGraphicsView *);	
	void resizeEvent ( QResizeEvent * event ); 
	void mousePressEvent(QMouseEvent *event);
	
protected slots:
	void updateFrame();
	
protected:
	MiniView * m_miniView;
	class MiniViewFrame * m_frame;
	class MiniViewFrame * m_outerFrame;
	QWidget * m_mask;
	
};


class MiniViewFrame : public QFrame
{
	Q_OBJECT
	
public:
	MiniViewFrame(QBrush &, bool draggable, QWidget * parent = 0);
	
	void setMaxDrag(int x, int y);
	
protected:
	void paintEvent(QPaintEvent * event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	
protected:
	QBrush m_brush;
	QPen m_pen;
	QPoint m_dragOffset;
	QPoint m_originalPos;
	bool m_inDrag;
	bool m_draggable;
	QSize m_maxDrag;
	
signals:
	void scrollChangeSignal(double x, double y);
};

#endif
