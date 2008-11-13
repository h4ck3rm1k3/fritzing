#ifndef MINIVIEW_H
#define MINIVIEW_H

#include <QGraphicsView>
#include <QBrush>

class MiniView : public QGraphicsView
{
	Q_OBJECT
	
public:
	MiniView(QWidget *parent=0);
	virtual ~MiniView();

	void setView(QGraphicsView *);	
	QGraphicsView* view();
	
protected:
	void resizeEvent ( QResizeEvent * event ); 
	void mousePressEvent(QMouseEvent *event);
		
public slots:
	void updateSceneRect ( const QRectF & rect );
	
signals:
	void rectChangedSignal();
	
protected:
	QGraphicsView * m_otherView;

};

#endif
