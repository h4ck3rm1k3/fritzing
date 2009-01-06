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

$Revision: 1627 $:
$Author: merunga $:
$Date: 2008-11-24 12:33:07 +0100 (Mon, 24 Nov 2008) $

********************************************************************/

#ifndef EXPANDINGLABEL_H_
#define EXPANDINGLABEL_H_

#include <QTextEdit>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>

#include "debugdialog.h"

class ExpandingLabel : public QTextEdit {
	Q_OBJECT
public:
	ExpandingLabel(QWidget *parent, int minSize=100) : QTextEdit(parent) {
		setMinimumWidth(minSize);
		setReadOnly(true);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setStyleSheet("border: 0px; background-color: transparent; margin-top: 8px; margin-bottom: 5px;");
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	void setLabelText(const QString& theText) {
		QTextDocument *doc = new QTextDocument(this);
		doc->setHtml(theText);
		setDocument(doc);
		setToolTip(theText);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		setAlignment(Qt::AlignCenter);
		setContextMenuPolicy(Qt::NoContextMenu);
	}

public slots:
	void allTextVisible() {
		QTextDocument *doc = document();
		doc->setTextWidth(width());
		int height = doc->documentLayout()->documentSize().toSize().height();
		setStyleSheet("border: 0px; background-color: transparent; margin-top: 0px; margin-bottom: 0px;");
		setFixedHeight(height);
	}

protected:
	void mouseMoveEvent(QMouseEvent * event) {
		QAbstractScrollArea::mouseMoveEvent(event);
	}
	void mousePressEvent(QMouseEvent *event) {
		QAbstractScrollArea::mousePressEvent(event);
	}
	void mouseReleaseEvent(QMouseEvent *event) {
		QAbstractScrollArea::mouseReleaseEvent(event);
	}
};

#endif /* EXPANDINGLABEL_H_ */
