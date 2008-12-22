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

$Revision: 1794 $:
$Author: merunga $:
$Date: 2008-12-11 14:50:11 +0100 (Thu, 11 Dec 2008) $

********************************************************************/


#include "fsizegrip.h"

FSizeGrip::FSizeGrip(MainWindow *parent) : QSizeGrip(parent) {
	m_mainWindow = parent;
	resize(sizeHint());
	rearrange();
}

void FSizeGrip::rearrange() {
	int x = m_mainWindow->width()-this->width();
	int y = m_mainWindow->height()-this->height();
	move(x,y);
	raise();
}
