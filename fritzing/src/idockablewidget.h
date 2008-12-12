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

$Revision: 1745 $:
$Author: cohen@irascible.com $:
$Date: 2008-12-06 18:49:37 +0100 (Sat, 06 Dec 2008) $

********************************************************************/



#ifndef IDOCKABLEWIDGET_H_
#define IDOCKABLEWIDGET_H_

class IDockableWidget {
public:
	virtual void addSizeGrip(QSizeGrip *sizeGrip) = 0;
	virtual void removeSizeGrip(QSizeGrip *sizeGrip) = 0;
};

#endif /* IDOCKABLEWIDGET_H_ */
