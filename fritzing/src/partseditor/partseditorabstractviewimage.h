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

$Revision$:
$Author$:
$Date$

********************************************************************/


#ifndef PARTSEDITORABSTRACTVIEWIMAGE_H_
#define PARTSEDITORABSTRACTVIEWIMAGE_H_

#include "../sketchwidget.h"
#include "../connectorstuff.h"
#include "partseditorpaletteitem.h"

class PartsEditorAbstractViewImage : public SketchWidget {
	Q_OBJECT

	public:
		PartsEditorAbstractViewImage(ItemBase::ViewIdentifier, bool showsTerminalPoints, QWidget *parent=0, int size=150);

	public slots:
		virtual void loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
		virtual void addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath);

	protected:
		void clearScene();
		virtual void fitCenterAndDeselect();
		void removeConnectors();

		void wheelEvent(QWheelEvent* event);

		ItemBase * addItemAux(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id, PaletteItem* paletteItem, bool doConnectors);

		ModelPart *createFakeModelPart(const QString &svgpath, const QString &relativepath);
		ModelPart *createFakeModelPart(const QHash<QString,StringPair*> &connIds, const QStringList &layers, QString svgFilePath);
		const QHash<QString,StringPair*> getConnectorIds(const QString &path);
		void getConnectorIdsAux(QHash<QString,StringPair*> &retval, QDomElement &docElem);
		const QStringList getLayers(const QString &path);

		PartsEditorPaletteItem *m_item; // just one item per view

		bool m_showsTerminalPoints;
};

#endif /* PARTSEDITORABSTRACTVIEWIMAGE_H_ */
