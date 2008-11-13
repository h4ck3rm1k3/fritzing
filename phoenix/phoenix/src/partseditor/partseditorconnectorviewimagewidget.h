/*
 * (c) Fachhochschule Potsdam
 */

#ifndef PARTSEDITORCONNECTORVIEWIMAGEWIDGET_H_
#define PARTSEDITORCONNECTORVIEWIMAGEWIDGET_H_

#include "partseditorabstractviewimage.h"

class PartsEditorConnectorViewImageWidget: public PartsEditorAbstractViewImage {
	Q_OBJECT
	public:
		PartsEditorConnectorViewImageWidget(ItemBase::ViewIdentifier, QWidget *parent=0, int size=150);

	public slots:
		void informConnectorSelection(const QString& connId);
		virtual void loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
		virtual void addItemInPartsEditor(ModelPart * modelPart, StringPair * svgFilePath);
		void setMismatching(ItemBase::ViewIdentifier viewId, const QString &id, bool mismatching);

	signals:
		void connectorsFound(ItemBase::ViewIdentifier viewId, const QList<Connector*> &conns);

	protected:
		void mousePressEvent(QMouseEvent *event);
		void connectItem();
};

#endif /* PARTSEDITORCONNECTORVIEWIMAGEWIDGET_H_ */
