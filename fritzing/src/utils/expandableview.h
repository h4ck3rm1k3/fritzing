/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef EXPANDABLEVIEW_H
#define EXPANDABLEVIEW_H

#include <QGroupBox>
#include <QVBoxLayout>

#include "clickablelabel.h"


class ExpandableView : public QGroupBox {
Q_OBJECT

public:
	ExpandableView(const QString & text = "", QWidget * parent = NULL);

	void setChildFrame(QFrame *);

signals:
	void expanded(bool);

public slots:
	void expanderClicked();

protected:
	QVBoxLayout * m_vLayout;
	ClickableLabel * m_expander;
	QFrame * m_childFrame;
};


#endif
