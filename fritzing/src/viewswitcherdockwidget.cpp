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

$Revision: 2136 $:
$Author: merunga $:
$Date: 2009-01-09 18:26:29 +0100 (Fri, 09 Jan 2009) $

********************************************************************/

#include <QtGui>
#include <QPainter>

#include "viewswitcherdockwidget.h"
#include "debugdialog.h"
#include "viewswitcher.h"

const double inactiveOpacity = 0.6;
const double activeOpacity = 1.0;

ViewSwitcherDockWidget::ViewSwitcherDockWidget(const QString & title, QWidget * parent)
    : FDockWidget(title, parent)
{
	m_bitmap = NULL;
	m_viewSwitcher = NULL;

	setFloating(true);
	QPoint initial(10,50);

#ifdef Q_WS_MAC
	initial.setY(34);
#else
	#ifdef Q_WS_X11
		setFloating(false);
		initial.setY(60);
	#endif
#endif

	m_offsetFromParent.setX(initial.x());
	m_offsetFromParent.setY(initial.y());
}

void ViewSwitcherDockWidget::calcWithin()
{
	QRect rw = parentWidget()->frameGeometry();
	QRect rt = this->frameGeometry();
	m_within = rw.contains(rt);
	DebugDialog::debug(QString("tabwindow docked %1").arg(m_within));
	if (m_within) {
		m_offsetFromParent = rt.topLeft() - rw.topLeft();
	}
}

void ViewSwitcherDockWidget::windowMoved(QWidget * widget) {
	if (!this->isFloating()) return;

	Q_UNUSED(widget);
	if (m_within) {
		QRect rw = parentWidget()->frameGeometry();
		this->move(rw.topLeft() + m_offsetFromParent);
	}
	else {
		calcWithin();
	}
}

bool ViewSwitcherDockWidget::event(QEvent *event)
{
	bool result = FDockWidget::event(event);
	if (isFloating()) {
		switch (event->type()) {
			case QEvent::MouseButtonRelease:
				calcWithin();
				break;
			default:
				break;
		}

	}
	return result;
}

void ViewSwitcherDockWidget::setViewSwitcher(ViewSwitcher * viewSwitcher) 
{
	m_viewSwitcher = viewSwitcher;
	setTitleBarWidget(viewSwitcher);
}

void ViewSwitcherDockWidget::resizeEvent(QResizeEvent * event)
{
	FDockWidget::resizeEvent(event);

	if (m_viewSwitcher == NULL) return;

	const QBitmap * mask = m_viewSwitcher->getMask();
	if (mask == NULL) return;

	QSize maskSize = mask->size();

	QSize sz = event->size();
	QBitmap * bitmap = new QBitmap(sz);
	bitmap->fill(Qt::white);

	QPainter painter(bitmap);
	QPointF p((sz.width() - maskSize.width()) / 2.0, (sz.height() - maskSize.height()) / 2.0);
	painter.drawPixmap(p, *mask);
	painter.end();

	this->setMask(*bitmap);
	if (m_bitmap) delete m_bitmap;
	m_bitmap = bitmap;

}

/*

void TabWindow::viewSwitched(int index) {
	if (index != m_viewIndex) {
		m_viewIndex = index;
	}
}

void TabWindow::enterEvent(QEvent *event) {
	QWidget::enterEvent(event);
	setWindowOpacity(m_docked ? activeOpacity : 1.0);
}

void TabWindow::leaveEvent(QEvent *event) {
	QWidget::leaveEvent(event);
	setWindowOpacity(m_docked ? inactiveOpacity : 1.0);
}

*/


