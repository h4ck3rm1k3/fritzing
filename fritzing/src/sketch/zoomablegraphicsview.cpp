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

#include <QWheelEvent>

#include "zoomablegraphicsview.h"
#include "../zoomcombobox.h"

ZoomableGraphicsView::ZoomableGraphicsView( QWidget * parent )
	: QGraphicsView(parent)
{
	m_scaleValue = 100;
	m_maxScaleValue = 2000;
	m_minScaleValue = 1;
	m_acceptWheelEvents = true;
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event) {
	if (!m_acceptWheelEvents) {
		QGraphicsView::wheelEvent(event);
		return;
	}

	QPointF mousePosition = event->pos();
	qreal delta = ((qreal)event->delta() / 120) * ZoomComboBox::ZoomStep;
	if (delta == 0) return;

	// Scroll zooming throw the combobox options
	/*if(delta < 0) {
		emit zoomOut(-1*delta);
	} else {
		emit zoomIn(delta);
	}*/

	// Scroll zooming relative to the current size
	relativeZoom(delta);

	//this->verticalScrollBar()->setValue(verticalScrollBar()->value() + 3);
	//this->horizontalScrollBar()->setValue(horizontalScrollBar()->value() + 3);


	//to do: center zoom around mouse location



	//QPointF pos = event->pos();
	//QPointF spos = this->mapToScene((int) pos.x(), (int) pos.y());


	//DebugDialog::debug(QString("translate %1 %2").arg(spos.x()).arg(spos.y()) );

	emit wheelSignal();
}

void ZoomableGraphicsView::relativeZoom(qreal step) {
	qreal tempSize = m_scaleValue + step;
	if (tempSize < m_minScaleValue) {
		m_scaleValue = m_minScaleValue;
		emit zoomOutOfRange(m_scaleValue);
		return;
	}
	if (tempSize > m_maxScaleValue) {
		m_scaleValue = m_maxScaleValue;
		emit zoomOutOfRange(m_scaleValue);
		return;
	}
	qreal tempScaleValue = tempSize/100;

	m_scaleValue = tempSize;

	//QPoint p = QCursor::pos();
	//QPoint q = this->mapFromGlobal(p);
	QPoint q(this->viewport()->size().width() / 2, this->viewport()->size().height() / 2);
	QPointF r = this->mapToScene(q);

	QMatrix matrix;
	matrix.scale(tempScaleValue, tempScaleValue);
	this->setMatrix(matrix);
	//this->centerOn(r);

	emit zoomChanged(m_scaleValue);
}

void ZoomableGraphicsView::absoluteZoom(qreal percent) {
	relativeZoom(percent-m_scaleValue);
}

qreal ZoomableGraphicsView::currentZoom() {
	return m_scaleValue;
}

void ZoomableGraphicsView::setAcceptWheelEvents(bool accept) {
	m_acceptWheelEvents = accept;
}
