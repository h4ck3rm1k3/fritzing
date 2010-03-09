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
#include <QScrollBar>
#include <QSettings>

#include "zoomablegraphicsview.h"
#include "../utils/zoomslider.h"

bool ZoomableGraphicsView::m_useWheelForZoom = true;
bool FirstTime = true;

ZoomableGraphicsView::ZoomableGraphicsView( QWidget * parent )
	: QGraphicsView(parent)
{
	m_scaleValue = 100;
	m_maxScaleValue = 2000;
	m_minScaleValue = 1;
	m_acceptWheelEvents = true;
	if (FirstTime) {
		FirstTime = false;
		QSettings settings;
		m_useWheelForZoom = settings.value("useWheelForZoom", "true").toBool();
	}
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event) {
	if (!m_acceptWheelEvents) {
		QGraphicsView::wheelEvent(event);
		return;
	}

	bool control = event->modifiers() & Qt::ControlModifier;

	if ((m_useWheelForZoom && !control) || (!m_useWheelForZoom && control)) {
		qreal delta = ((qreal) event->delta() / 120) * ZoomSlider::ZoomStep;
		if (delta == 0) return;

		// Scroll zooming relative to the current size
		relativeZoom(delta, true);

		emit wheelSignal();
	}
	else {
		int numSteps = event->delta() / 8;
		if (event->orientation() == Qt::Horizontal) {
			horizontalScrollBar()->setValue( horizontalScrollBar()->value() - numSteps);
		} else {
			verticalScrollBar()->setValue( verticalScrollBar()->value() - numSteps);
		}
	}
}

void ZoomableGraphicsView::relativeZoom(qreal step, bool centerOnCursor) {
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
	//QPointF r = this->mapToScene(q);

	QMatrix matrix;
	matrix.scale(tempScaleValue, tempScaleValue);
	if (centerOnCursor) {
		//this->setMatrix(QMatrix().translate(-r.x(), -r.y()) * matrix * QMatrix().translate(r.x(), r.y()));
        this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	}
	else {
		this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	}
	this->setMatrix(matrix);

	emit zoomChanged(m_scaleValue);
}

void ZoomableGraphicsView::absoluteZoom(qreal percent) {
	relativeZoom(percent-m_scaleValue, false);
}

qreal ZoomableGraphicsView::currentZoom() {
	return m_scaleValue;
}

void ZoomableGraphicsView::setAcceptWheelEvents(bool accept) {
	m_acceptWheelEvents = accept;
}

void ZoomableGraphicsView::setUseWheelForZoom(bool use) {
	m_useWheelForZoom = use;
}

bool ZoomableGraphicsView::useWheelForZoom() {
	return m_useWheelForZoom;
}

