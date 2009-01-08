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

#include <QtGui>

#include "tabwindow.h"
#include "viewswitcher.h"
#include "debugdialog.h"

const double inactiveOpacity = 0.6;
const double activeOpacity = 1.0;

TabWindow::TabWindow(QWidget *parent)
    : QWidget(parent)
{
	m_viewSwitcher = NULL;
	m_docked = true;
    m_toggleViewAction = new QAction(this);
    m_toggleViewAction->setCheckable(true);
    m_toggleViewAction->setText(windowTitle());
    QObject::connect(m_toggleViewAction, SIGNAL(triggered(bool)), this, SLOT(toggleMe(bool)));

	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
	resize(20, 20);
	m_hLayout = new QHBoxLayout;
	this->setLayout(m_hLayout);
	m_hLayout->setSpacing(0);
	m_hLayout->setMargin(0);
	setWindowOpacity(inactiveOpacity);

	m_viewIndex = 999;


}

void TabWindow::addViewSwitcher(ViewSwitcher * viewSwitcher) {
	m_viewSwitcher = viewSwitcher;
	viewSwitcher->setParent(this);
	m_hLayout->addWidget(viewSwitcher);
	QApplication::processEvents();
	connect(viewSwitcher, SIGNAL(viewSwitched(int)), this, SLOT(viewSwitched(int)), Qt::DirectConnection);
}

void TabWindow::mousePressEvent(QMouseEvent *event)
{
	m_inDrag = true;
	m_dragStartPos = event->globalPos() - this->pos();
	m_movedEnoughPos = event->globalPos();
	m_movedEnough = false;
}

void TabWindow::mouseReleaseEvent(QMouseEvent *   event )
{
	Q_UNUSED(event);
	m_inDrag = false;
	m_movedEnough = false;
	calcDocked();
}

void TabWindow::calcDocked() 
{
	QRect rw = this->parentWidget()->frameGeometry();
	QRect rt = this->frameGeometry();
	m_docked = rw.contains(rt);
	DebugDialog::debug(QString("tabwindow docked %1").arg(m_docked));
	if (m_docked) {
		m_offsetFromParent = rt.topLeft() - rw.topLeft();
	}
}

void TabWindow::parentMoved() {
	if (m_docked) {
		QRect rw = this->parentWidget()->frameGeometry();
		this->move(rw.topLeft() + m_offsetFromParent);
	}
	else {
		calcDocked();
	}
}

void TabWindow::parentStoppedMoving() {
	if (!m_docked) {
		calcDocked();
	}
}

void TabWindow::mouseMoveEvent(QMouseEvent *event)
{
	if (m_inDrag) {
		QPoint pos = event->globalPos();
		if (m_movedEnough) {
			this->move(pos - m_dragStartPos);
		}
		else {
			QPoint d = pos - m_movedEnoughPos;
			m_movedEnough =(d.x() * d.x()) + (d.y() * d.y()) >= 25;
		}
	}
} 

void TabWindow::toggleMe(bool b)
{
    if (b == isHidden()) {
        if (b)
            show();
        else
            close();
    }
}


bool TabWindow::event(QEvent *event)
{
	switch (event->type()) {
    case QEvent::Hide:
        m_toggleViewAction->setChecked(false);
        break;
    case QEvent::Show:
        m_toggleViewAction->setChecked(true);
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

QAction * TabWindow::toggleViewAction() const
{
    return m_toggleViewAction;
}

void TabWindow::setWindowTitle(const QString & title) {
	m_toggleViewAction->setText(title);
	QWidget::setWindowTitle(title);
}

void TabWindow::viewSwitched(int index) {
	if (index != m_viewIndex) {
		m_viewIndex = index;
	}
}

void TabWindow::saveState() {
	m_state = this->isVisible();
}

void TabWindow::restoreState() {
	//DebugDialog::debug(QString("tab window restore state %1").arg(m_state));
	if (m_state) {
		this->setVisible(true);
	}
}

void TabWindow::setMask() {
	const QBitmap * mask = m_viewSwitcher->getMask();
	if (mask != NULL) {
		QWidget::setMask(*mask);
	}
}

void TabWindow::enterEvent(QEvent *event) {
	QWidget::enterEvent(event);
	setWindowOpacity(activeOpacity);
}

void TabWindow::leaveEvent(QEvent *event) {
	QWidget::leaveEvent(event);
	setWindowOpacity(inactiveOpacity);
}

bool TabWindow::isDocked() {
	return m_docked;
}
