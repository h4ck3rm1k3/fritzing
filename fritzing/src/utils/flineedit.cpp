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

#include "flineedit.h"

FLineEdit::FLineEdit(QWidget * parent) : QLineEdit(parent)
{
	editingFinishedSlot();
	connect(this, SIGNAL(editingFinished()), this, SLOT(editingFinishedSlot()));
}

FLineEdit::~FLineEdit()
{
}

void FLineEdit::editingFinishedSlot() {
	setReadOnly(true);
	emit editable(false);
	setCursor(Qt::IBeamCursor);
}

void FLineEdit::mousePressEvent ( QMouseEvent * event ) {
	if (isReadOnly()) {
		setReadOnly(false);
		emit editable(true);
	}

	QLineEdit::mousePressEvent(event);
}

void FLineEdit::enterEvent(QEvent * event) {
	QLineEdit::enterEvent(event);
	if (isReadOnly()) {
		emit mouseEnter();
	}
}

void FLineEdit::leaveEvent(QEvent * event) {
	QLineEdit::leaveEvent(event);
	if (isReadOnly()) {
		emit mouseLeave();
	}
}

