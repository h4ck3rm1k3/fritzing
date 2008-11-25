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

$Revision$:
$Author$:
$Date$

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
		virtual void removePart(const QString &moduleID) = 0;

		virtual ModelPart *selected() = 0;

		bool alreadyIn(QString moduleID);

	protected:
		virtual void doClear();
		void setItem(ModelPart * modelPart);
		void mousePressOnItem(
				const QString &moduleId, const QSize &size,
				const QPointF &dataPoint = QPointF(0,0), const QPoint &hotspot = QPoint(0,0));

		virtual void setItemAux(ModelPart * modelPart) = 0;

		QHash<QString /*moduleId*/,ModelPart*> m_partHash;
};

#endif /* PARTSBINVIEW_H_ */
