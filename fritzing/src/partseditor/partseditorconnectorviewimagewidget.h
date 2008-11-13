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
