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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/



#ifndef PARTSBINVIEW_H_
#define PARTSBINVIEW_H_

#include "../palettemodel.h"
#include "../paletteitem.h"

class PartsBinView {
	public:
		virtual ~PartsBinView() {};				// removes compiler warnings

		virtual void setPaletteModel(PaletteModel * model, bool clear = false);
		void reloadParts(PaletteModel * model);
		void addPart(ModelPart * model);

		virtual PaletteItem *selected() = 0;

	protected:
		void setItem(ModelPart * modelPart);
		void mousePressOnItem(
				const QString &moduleId, const QSize &size,
				const QPointF &dataPoint = QPointF(0,0), const QPoint &hotspot = QPoint(0,0));

		virtual void doClear() = 0;
		virtual void setModel(PaletteModel * model) = 0;
		virtual void setItemAux(ModelPart * modelPart) = 0;
};

#endif /* PARTSBINVIEW_H_ */
