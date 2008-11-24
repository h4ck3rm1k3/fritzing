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

$Revision: 1627 $:
$Author: merunga $:
$Date: 2008-11-24 12:33:07 +0100 (Mon, 24 Nov 2008) $

********************************************************************/

#ifndef EXPANDINGLABEL_H_
#define EXPANDINGLABEL_H_

#include <QTextEdit>

class ExpandingLabel : public QTextEdit {
public:
	ExpandingLabel(QWidget *parent) : QTextEdit(parent) {
		setReadOnly(true);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setStyleSheet("border: 0px; background-color: transparent; margin-top: 8px; margin-bottom: 5px;");
		setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
	}

	void setLabelText(const QString& theText) {
		setText(theText);
		setToolTip(theText);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
		setAlignment(Qt::AlignCenter);
		setContextMenuPolicy(Qt::NoContextMenu);
	}
};

#endif /* EXPANDINGLABEL_H_ */
