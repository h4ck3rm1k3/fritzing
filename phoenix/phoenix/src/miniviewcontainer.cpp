
#include <QScrollBar>

#include "miniviewcontainer.h"
#include "debugdialog.h"

MiniViewContainer::MiniViewContainer( QWidget * parent ) 
	: QWidget(parent)
{
	m_miniView = new MiniView(this);
	connect(m_miniView, SIGNAL(rectChangedSignal()), this, SLOT(updateFrame()) );
	m_miniView->resize(this->size());
		
	QBrush brush1(QColor(0,0,0));
	m_outerFrame = new MiniViewFrame(brush1, false, this);
	m_outerFrame->resize(this->size());
	m_outerFrame->setUpdatesEnabled(false);
	
	QBrush brush2(QColor(128,0,0));
	m_frame = new MiniViewFrame(brush2, true, this);
	m_frame->resize(this->size());
		
	m_mask = new QWidget(this);
	QPalette p = m_mask->palette();
	p.setBrush(QPalette::Window, QBrush(QColor(0,0,0)));
	m_mask->setPalette(p);
	m_mask->setAutoFillBackground(true);
	m_mask->resize(this->size());
	
	
}

void MiniViewContainer::setView(QGraphicsView * view) 
{
	QGraphicsView * oldView = m_miniView->view();
	if (oldView == view) return;
	
	if (oldView != NULL) {
		disconnect(oldView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateFrame()));
		disconnect(oldView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateFrame()));		
		disconnect(oldView->scene(), SIGNAL(sceneRectChanged(QRectF)), this, SLOT(updateFrame()));		
		disconnect(oldView, SIGNAL(resizeSignal()), this, SLOT(updateFrame()));		
		disconnect(m_frame, SIGNAL(scrollChangeSignal(double, double)), oldView, SLOT(navigatorScrollChange(double, double)));
	}

	QPalette p = m_mask->palette();
	p.setBrush(QPalette::Window, view->backgroundBrush());
	m_mask->setPalette(p);

	m_miniView->setView(view);
	updateFrame();
	
	bool succeeded = connect(view->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateFrame()));
	succeeded = connect(view->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateFrame()));
	succeeded = connect(view->scene(), SIGNAL(sceneRectChanged(QRectF)), this, SLOT(updateFrame()));
	succeeded = connect(view, SIGNAL(resizeSignal()), this, SLOT(updateFrame()));
	succeeded = connect(m_frame, SIGNAL(scrollChangeSignal(double, double)), view, SLOT(navigatorScrollChange(double, double)));

	// force a resize on a view change because otherwise some size or sceneRect isn't updated and the navigator is off
	m_miniView->resize(this->size() / 2);
	m_miniView->resize(this->size());
}


void MiniViewContainer::resizeEvent ( QResizeEvent * event ) 
{
	QWidget::resizeEvent(event);
	m_miniView->resize(this->size());	
}

void MiniViewContainer::mousePressEvent(QMouseEvent *) {
}

void MiniViewContainer::updateFrame() 
{
	QGraphicsView * view = m_miniView->view();
	QSize vSize = view->size();

	bool vVis = false;
	bool hVis = false;
	if (view->verticalScrollBar()->isVisible()) {
		vSize.setWidth(view->width() - view->verticalScrollBar()->width());
		vVis = true;
	}
	if (view->horizontalScrollBar()->isVisible()) {
		vSize.setHeight(view->height() - view->horizontalScrollBar()->height());
		hVis = true;
	}

	QPointF topLeft = view->mapToScene(QPoint(0, 0));
	QPointF bottomRight = view->mapToScene(QPoint(vSize.width(), vSize.height()));

	QRectF sceneRect = view->sceneRect();
	if (sceneRect.width() >= 1 && sceneRect.height() >= 1) {
		if (sceneRect.width() < vSize.width()) {
			vSize.setWidth((int) sceneRect.width());			
		}
		if (sceneRect.height() < vSize.height()) {
			vSize.setHeight((int) sceneRect.height());
		}
	}
	if (vVis || hVis) {
		m_frame->setVisible(true);
		m_outerFrame->setVisible(true);	
		//m_mask->setVisible(true);
	}
	else {
		// scrollbars not visible
		m_frame->setVisible(false);
		m_outerFrame->setVisible(false);
		//m_mask->setVisible(false);
	}
	
	int tw = sceneRect.width();
	int th = sceneRect.height();
	
	int w = m_miniView->width();
	int h = m_miniView->height();
		
	if (tw > 0 && th > 0) {
	// deal with aspect ratio
		int trueH = w * th / tw;
		if (trueH <= h) {
			m_mask->setGeometry(0, trueH, w, h);
			h = trueH;
		}
		else {
			int trueW = h * tw / th;
			m_mask->setGeometry(trueW, 0, w, h);
			w = trueW;		
		}
	}
	
	//DebugDialog::debug(tr("mask %1 %2 %3 %4").arg(m_mask->geometry().x())
		//.arg(m_mask->geometry().y())
		//.arg(m_mask->geometry().width())
		//.arg(m_mask->geometry().height()) );
	
	m_outerFrame->resize(w, h);
	m_frame->setMaxDrag(w, h);
	
	if (th > 0 && tw > 0) {
		int newW = w * (bottomRight.x() - topLeft.x())  / tw;
		int newH = h * (bottomRight.y() - topLeft.y())  / th;
		int newX = w * (topLeft.x() - sceneRect.x()) / tw;
		int newY = h * (topLeft.y() - sceneRect.y()) / th;
		
		//DebugDialog::debug(tr("minivp %1 %2").arg(w).arg(h) );

		m_frame->setGeometry(newX, newY, newW, newH);
	}
}


MiniViewFrame::MiniViewFrame(QBrush & brush, bool draggable, QWidget * parent) 
	: QFrame(parent)
	
{	
	m_brush = brush;
	m_pen.setBrush(m_brush);
	m_pen.setWidth(4);
	m_draggable = draggable;
}

void MiniViewFrame::paintEvent(QPaintEvent * ) {
   	QPainter painter(this);
   	painter.setPen(m_pen);
    painter.setOpacity(0.33);
    painter.drawRect(0,0, this->size().width(), this->size().height());
}

void MiniViewFrame::mousePressEvent(QMouseEvent * event) {
	if (m_draggable) {
		m_dragOffset = event->globalPos();
		m_originalPos = this->pos();
		m_inDrag = true;
	}
	else {
		QFrame::mousePressEvent(event);
	}
}

void MiniViewFrame::mouseMoveEvent(QMouseEvent * event) {
	if (m_inDrag) {
		QRect r = this->geometry();
		QPoint newPos = m_originalPos + event->globalPos() - m_dragOffset;
		if (newPos.x() < 0) {
			newPos.setX(0);
		}
		if (newPos.y() < 0) {
			newPos.setY(0);
		}
		if (newPos.x() + r.width() > m_maxDrag.width()) {
			newPos.setX(m_maxDrag.width() - r.width());
		}
		if (newPos.y() + r.height() > m_maxDrag.height()) {
			newPos.setY(m_maxDrag.height() - r.height());
		}
		r.moveTopLeft(newPos);
		if (r != this->geometry()) {
			this->setGeometry(r);
			emit scrollChangeSignal(newPos.x() / (double) (m_maxDrag.width() - r.width()),
									newPos.y() / (double) (m_maxDrag.height() - r.height()) );
		}
	}
	else {
		QFrame::mouseMoveEvent(event);
	}
}

void MiniViewFrame::mouseReleaseEvent(QMouseEvent * event) {
	if (m_inDrag) {
		m_inDrag = false;
	}
	else {
		QFrame::mouseReleaseEvent(event);
	}
}

void MiniViewFrame::setMaxDrag(int x, int y) {
	m_maxDrag.setWidth(x);
	m_maxDrag.setHeight(y);
}

