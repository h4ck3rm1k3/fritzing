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

$Revision: 2776 $:
$Author: merunga $:
$Date: 2009-04-02 13:54:08 +0200 (Thu, 02 Apr 2009) $

********************************************************************/


#include <QMimeData>
#include <QDragEnterEvent>

#include "stackwidgetseparator.h"

StackWidgetSeparator::StackWidgetSeparator(QWidget *parent)
	:QFrame(parent)
{
	setMinimumHeight(10);
	setAcceptDrops(true);
}

void StackWidgetSeparator::dragEnterEvent(QDragEnterEvent *event) {
	// Only accept if it's an tab-reordering request
	const QMimeData* m = event->mimeData();
	QStringList formats = m->formats();
	if (formats.contains("action") && (m->data("action") == "tab-reordering")) {
		event->acceptProposedAction();
		expand();
		emit setReceptor(this);
	}
	QFrame::dragEnterEvent(event);
}

void StackWidgetSeparator::dragLeaveEvent(QDragLeaveEvent *event) {
	shrink();
	emit setReceptor(NULL);
	QFrame::dragLeaveEvent(event);
}

void StackWidgetSeparator::dropEvent(QDropEvent* event) {
	emit dropped();
	QFrame::dropEvent(event);
}

void StackWidgetSeparator::expand() {
	setMinimumHeight(200);
	resize(width(),200);
}

void StackWidgetSeparator::shrink() {
	setMinimumHeight(10);
	resize(width(),10);
}


