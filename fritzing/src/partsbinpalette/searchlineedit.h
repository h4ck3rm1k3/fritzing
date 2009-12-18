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

$Revision: 3774 $:
$Author: cohen@irascible.com $:
$Date: 2009-11-25 11:14:43 +0100 (Wed, 25 Nov 2009) $

********************************************************************/

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QMouseEvent>

class SearchLineEdit : public QLineEdit {
Q_OBJECT

public:
	SearchLineEdit(QWidget * parent = NULL);
	~SearchLineEdit();

protected:
	void paintEvent(QPaintEvent *);

protected slots:
	void setPost(const QString & text);

protected:
	bool m_initialState;

};


#endif
