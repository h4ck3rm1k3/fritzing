/*
 * (c) Fachhochschule Potsdam
 */

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
