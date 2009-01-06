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

#include "fdockwidget.h"
#include "debugdialog.h"

FDockWidget::FDockWidget( const QString & title, QWidget * parent)
	: QDockWidget(title, parent)
{
	setObjectName(title.trimmed().toLower().remove(" "));
	setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
}

void FDockWidget::saveState() {
	m_state = this->isFloating() && this->isVisible();
}

void FDockWidget::restoreState() {
	if (m_state) {
		this->setVisible(true);
	}
}

bool FDockWidget::event(QEvent * e) {
	switch (e->type()) {
		case QEvent::WindowActivate:
			emit dockChangeActivationSignal(true);
			break;
		case QEvent::WindowDeactivate:
			emit dockChangeActivationSignal(false);
			break;
		default:
			break;
	}
	return QDockWidget::event(e);
}

void FDockWidget::moveEvent(QMoveEvent *event) {
	QDockWidget::moveEvent(event);
	emit positionChanged();
}
