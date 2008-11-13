#include "miniview.h"
#include "debugdialog.h"


MiniView::MiniView(QWidget *parent ) 
	: QGraphicsView(parent)
{
	m_otherView = NULL;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	


}

MiniView::~MiniView()
{
}

void MiniView::setView(QGraphicsView * view) {
	m_otherView = view;
	this->setBackgroundBrush(view->backgroundBrush());
	QGraphicsView::setScene(view->scene());
}

void MiniView::updateSceneRect ( const QRectF & rect ) {
	QGraphicsView::updateSceneRect(rect);
	fitInView ( rect, Qt::KeepAspectRatio );
	QMatrix matrix = this->matrix();
	if (matrix.m11() < 0 && matrix.m22() < 0) {
		// bug (in Qt?) scales the matrix negatively;  try flipping the scale
		//DebugDialog::debug(QString("updatescenerect rect:%1 %2 %3 %4' m11:%5 m22:%6")
						   //.arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height())
						   //.arg(matrix.m11()).arg(matrix.m22()) );
		QMatrix m2(-matrix.m11(), matrix.m12(), matrix.m21(), -matrix.m22(), matrix.dx(), matrix.dy());
		setMatrix(m2);
	}
	
	emit rectChangedSignal();
}

void MiniView::resizeEvent ( QResizeEvent * event ) 
{
	QGraphicsView::resizeEvent(event);
	fitInView(sceneRect(), Qt::KeepAspectRatio);	
	emit rectChangedSignal();
}

void MiniView::mousePressEvent(QMouseEvent *) {
	// ignore mouse presses
}

QGraphicsView * MiniView::view() {
	return m_otherView;
}

