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




#include "sketchtoolbutton.h"

#include <QAction>
#include <QActionEvent>

SketchToolButton::SketchToolButton(QWidget *parent)
	: QToolButton(parent) {}

void SketchToolButton::actionEvent(QActionEvent *event) {
	switch (event->type()) {
		case QEvent::ActionChanged:
			if (event->action() == defaultAction()) {
				setEnabled(defaultAction()->isEnabled()); // update button state
			}
			break;
		default:
			QToolButton::actionEvent(event);
	}
}
