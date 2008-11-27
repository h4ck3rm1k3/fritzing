/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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
	// for some reason, this mousePressEvent isn't detected by setting an event filter on the miniview
	// maybe because the event happens in the scene and get swallowed before the filter gets it?
	emit miniViewMousePressedSignal();
}

QGraphicsView * MiniView::view() {
	return m_otherView;
}

