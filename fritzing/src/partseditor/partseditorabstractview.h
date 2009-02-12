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


#ifndef PARTSEDITORABSTRACTVIEW_H_
#define PARTSEDITORABSTRACTVIEW_H_

#include "../sketchwidget.h"
#include "../connectorstuff.h"
#include "partseditorpaletteitem.h"

class PartsEditorAbstractView : public SketchWidget {
	Q_OBJECT

	public:
		PartsEditorAbstractView(ItemBase::ViewIdentifier, QDir tempDir, bool deleteModelPartOnSceneClear, QWidget *parent=0, int size=150);
		QDir tempFolder();
		bool isEmpty();

	public slots:
		virtual void loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
		virtual void addItemInPartsEditor(ModelPart * modelPart, SvgAndPartFilePath * svgFilePath);

	protected:
		virtual PartsEditorPaletteItem *newPartsEditorPaletteItem(ModelPart * modelPart);
		virtual PartsEditorPaletteItem *newPartsEditorPaletteItem(ModelPart * modelPart, SvgAndPartFilePath *path);

		void setDefaultBackground();
		void clearScene();
		virtual void fitCenterAndDeselect();
		void removeConnectors();

		void wheelEvent(QWheelEvent* event);

		ItemBase * addItemAux(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id, PaletteItem* paletteItem, bool doConnectors);

		ModelPart *createFakeModelPart(SvgAndPartFilePath *svgpath);
		ModelPart *createFakeModelPart(const QHash<QString,StringPair*> &connIds, const QStringList &layers, QString svgFilePath);
		//void replaceMyViewNode(QDomDocument *domDoc, const QString &nodeContent);

		const QHash<QString,StringPair*> getConnectorIds(const QString &path);
		void getConnectorIdsAux(QHash<QString,StringPair*> &retval, QDomElement &docElem);
		const QStringList getLayers(const QString &path);

		QString getOrCreateViewFolderInTemp();

		PartsEditorPaletteItem *m_item; // just one item per view
		QDir m_tempFolder;
		bool m_deleteModelPartOnSceneClear;
};

#endif /* PARTSEDITORABSTRACTVIEW_H_ */
