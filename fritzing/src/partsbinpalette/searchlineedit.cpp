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
#include "../debugdialog.h"

#include <QPainter>
#include <QImage>
#include <QPixmap>

static QString InitialText;
static QPixmap * SearchFieldPixmap;

SearchLineEdit::SearchLineEdit(QWidget * parent) : QLineEdit(parent)
{
    m_syncSelectionCount = m_syncCursorCount = 0;
	if (InitialText.isEmpty()) {
		InitialText = tr("type to search");
	}

    SearchFieldPixmap = NULL;
    connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(preSyncCursor(int, int)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(preSyncSelection()));
}

SearchLineEdit::~SearchLineEdit()
{
}

void SearchLineEdit::mousePressEvent(QMouseEvent * event) {
    QLineEdit::mousePressEvent(event);
    emit clicked();
}

void SearchLineEdit::paintEvent(QPaintEvent * event) {
	QLineEdit::paintEvent(event);
    if (SearchFieldPixmap == NULL) {
        SearchFieldPixmap = new QPixmap(":/resources/images/icons/searchField.png");
    }
    if (SearchFieldPixmap == NULL) return;
    if (SearchFieldPixmap->isNull()) return;

    QPainter painter(this);
    QSize sz = size();
    int x = sz.width() - SearchFieldPixmap->width() - 2;
    int y = (sz.height() - SearchFieldPixmap->height()) / 2;
    painter.drawPixmap(x, y, SearchFieldPixmap->width(), SearchFieldPixmap->height(), *SearchFieldPixmap);

}

void SearchLineEdit::syncText(const QString & txt) {
    setText(txt);
}


void SearchLineEdit::syncCursor(int newpos) {
    if (m_syncCursorCount > 0) return;

    setCursorPosition(newpos);
    DebugDialog::debug(QString("setcursorposition %1 %2").arg((long) this, 0, 16).arg((long) sender(), 0, 16));
}

void SearchLineEdit::syncSelection(int selStart, int selLength) {
    if (m_syncSelectionCount > 0) return;

    DebugDialog::debug(QString("sync selection %1 %2").arg((long) this, 0, 16).arg((long) sender(), 0, 16));

    this->setSelection(selStart, selLength);
}

void SearchLineEdit::preSyncCursor(int oldPos, int newPos) {
    if (m_syncCursorCount > 0) return;

    Q_UNUSED(oldPos);
    m_syncCursorCount++;
    DebugDialog::debug(QString("emit cursorposition %1").arg((long) this, 0, 16));
    emit cursorPositionChanged2(newPos);
    m_syncCursorCount--;
}

void SearchLineEdit::preSyncSelection() {
    if (m_syncSelectionCount > 0) return;

    m_syncSelectionCount++;
    DebugDialog::debug(QString("emit selection changed %1").arg((long) this, 0, 16));
    emit selectionChanged2(this->selectionStart(), this->selectedText().length());
    m_syncSelectionCount--;
}

