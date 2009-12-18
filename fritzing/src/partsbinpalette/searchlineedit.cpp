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

#include "searchlineedit.h"

#include <QPainter>
#include <QImage>

static QString InitialText;

SearchLineEdit::SearchLineEdit(QWidget * parent) : QLineEdit(parent)
{
	if (InitialText.isEmpty()) {
		InitialText = tr("type to search");
	}

	m_initialState = true;
	connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(setPost(const QString &)));
}

SearchLineEdit::~SearchLineEdit()
{
}

void SearchLineEdit::setPost(const QString & text) {
	Q_UNUSED(text);
	disconnect(this, SIGNAL(textEdited(const QString &)), this, SLOT(setPost(const QString &)));
	m_initialState = false;
	update();
}

void SearchLineEdit::paintEvent(QPaintEvent * event) {
	QLineEdit::paintEvent(event);
	if (m_initialState) {
		QPainter painter(this);
		painter.setOpacity(.5);
		QSize sz = size();
		painter.drawText(0, 0, sz.width(), sz.height(), Qt::AlignVCenter + Qt::AlignRight + Qt::TextSingleLine, InitialText);
	}
}

void SearchLineEdit::syncText(const QString & txt) {
    setText(txt);
}


