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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

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

void FDockWidget::changeEvent(QEvent * event) {
	if (event) {
		if (event->type() == QEvent::ActivationChange) {
			DebugDialog::debug(QObject::tr("change activation in dock"));
			emit dockChangeActivationSignal(this);
		}
	}
	QDockWidget::changeEvent(event);
}
