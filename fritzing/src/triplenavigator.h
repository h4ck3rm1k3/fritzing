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

$Revision: 1668 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-27 22:51:05 +0100 (Thu, 27 Nov 2008) $

********************************************************************/

#ifndef TRIPLENAVIGATOR_H
#define TRIPLENAVIGATOR_H

#include <QFrame>
#include <QSplitter>
#include <QLabel>

#include "miniviewcontainer.h"

class TripleNavigator : public QFrame
{
	Q_OBJECT
	
public:
	TripleNavigator(QWidget * parent = 0);
	void addView(MiniViewContainer *);	
	
protected:
	QSplitter * m_splitter;
};

class TripleNavigatorLabel : public QLabel 
{
};

#endif
