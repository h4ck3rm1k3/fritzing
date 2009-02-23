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

$Revision: 2292 $:
$Author: cohen@irascible.com $:
$Date: 2009-01-31 23:08:01 +0100 (Sat, 31 Jan 2009) $

********************************************************************/

#include <QWheelEvent>

#include "zoomablegraphicsview.h"
#include "zoomcombobox.h"

ZoomableGraphicsView::ZoomableGraphicsView( QWidget * parent )
	: QGraphicsView(parent)
{
	m_scaleValue = 100;
	m_maxScaleValue = 2000;
	m_minScaleValue = 1;
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event) {
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

	QMatrix matrix;
	matrix.scale(tempScaleValue, tempScaleValue);
	this->setMatrix(matrix);

	emit zoomChanged(m_scaleValue);
}

void ZoomableGraphicsView::absoluteZoom(qreal percent) {
	relativeZoom(percent-m_scaleValue);
}

qreal ZoomableGraphicsView::currentZoom() {
	return m_scaleValue;
}

