/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

ZoomableGraphicsView::WheelMapping ZoomableGraphicsView::m_wheelMapping = MapNoZCtrlVAltH;
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
		m_wheelMapping = (WheelMapping) settings.value("wheelMapping", m_wheelMapping).toInt();
	}
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event) {
	if (!m_acceptWheelEvents) {
		QGraphicsView::wheelEvent(event);
		return;
	}

	bool doZoom = false;
	bool doHorizontal = false;
	bool doVertical = false;

	bool control = event->modifiers() & Qt::ControlModifier;
	bool alt = event->modifiers() & Qt::AltModifier;

	switch (m_wheelMapping) {
		case MapNoZCtrlVAltH:
			if (control) doVertical = true;
			else if (alt) doHorizontal = true;
			else doZoom = true;
			break;
		case MapNoZCtrlHAltV:
			if (control) doHorizontal = true;
			else if (alt) doVertical = true;
			else doZoom = true;
			break;
		case MapNoVCtrlZAltH:
			if (control) doZoom = true;
			else if (alt) doHorizontal = true;
			else doVertical = true;
			break;
		case MapNoVCtrlHAltZ:
			if (control) doHorizontal = true;
			else if (alt) doZoom = true;
			else doVertical = true;
			break;
		case MapNoHCtrlVAltZ:
			if (control) doVertical = true;
			else if (alt) doZoom = true;
			else doHorizontal = true;
			break;
		case MapNoHCtrlZAltV:
			if (control) doZoom = true;
			else if (alt) doVertical = true;
			else doHorizontal = true;
			break;
		default:
			// shouldn't happen
			return;
	}

	int numSteps = event->delta() / 8;
	if (doZoom) {
		qreal delta = ((qreal) event->delta() / 120) * ZoomSlider::ZoomStep;
		if (delta == 0) return;

		// Scroll zooming relative to the current size
		relativeZoom(delta, true);

		emit wheelSignal();
	}
	else if (doVertical) {
		verticalScrollBar()->setValue( verticalScrollBar()->value() - numSteps);
	}
	else if (doHorizontal) {
		horizontalScrollBar()->setValue( horizontalScrollBar()->value() - numSteps);
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

void ZoomableGraphicsView::setWheelMapping(WheelMapping wm) {
	m_wheelMapping = wm;
}

ZoomableGraphicsView::WheelMapping ZoomableGraphicsView::wheelMapping() {
	return m_wheelMapping;
}

