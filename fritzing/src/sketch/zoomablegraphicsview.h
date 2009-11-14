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

#ifndef ZOOMABLEGRAPHICSVIEW_H
#define ZOOMABLEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMenu>
#include <QHash>
#include <QList>


class ZoomableGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	ZoomableGraphicsView(QWidget* parent = 0);

    void relativeZoom(qreal step);
    void absoluteZoom(qreal percent);
 	qreal currentZoom();
	void setAcceptWheelEvents(bool);

	virtual void ensureFixedToBottomRightItems() {}

signals:
	void zoomChanged(qreal zoom);
	void zoomOutOfRange(qreal zoom);
	void wheelSignal();

protected:
	virtual void wheelEvent(QWheelEvent* event);

protected:
	qreal m_scaleValue;
	int m_maxScaleValue;
	int m_minScaleValue;
	bool m_acceptWheelEvents;
};

#endif
