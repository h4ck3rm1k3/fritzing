/*
 * (c) Fachhochschule Potsdam
 */

#ifndef PARTSEDITORABSTRACTVIEWIMAGE_H_
#define PARTSEDITORABSTRACTVIEWIMAGE_H_

#include "../sketchwidget.h"
#include "../connectorstuff.h"
#include "partseditorpaletteitem.h"

class PartsEditorAbstractViewImage : public SketchWidget {
	Q_OBJECT

	public:
		PartsEditorAbstractViewImage(ItemBase::ViewIdentifier, QWidget *parent=0, int size=150);

	public slots:
		virtual void loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
		virtual void addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath);

	protected:
		void clearScene();
		void fitCenterAndDeselect();
		void removeConnectors();

		void wheelEvent(QWheelEvent* event);

		ItemBase * addItemAux(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id, PaletteItem* paletteItem, bool doConnectors);

		ModelPart *createFakeModelPart(const QString &svgpath, const QString &relativepath);
		ModelPart *createFakeModelPart(const QHash<QString,StringPair*> &connIds, const QStringList &layers, QString svgFilePath);
		const QHash<QString,StringPair*> getConnectorIds(const QString &path);
		void getConnectorIdsAux(QHash<QString,StringPair*> &retval, QDomElement &docElem);
		const QStringList getLayers(const QString &path);

		PartsEditorPaletteItem *m_item; // just one item per view
};

#endif /* PARTSEDITORABSTRACTVIEWIMAGE_H_ */
