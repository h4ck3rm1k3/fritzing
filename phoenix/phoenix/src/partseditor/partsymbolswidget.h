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



#ifndef PARTSYMBOLSWIDGET_H_
#define PARTSYMBOLSWIDGET_H_

#include <QWidget>

#include "partseditorviewimagewidget.h"
#include "connectorsinfowidget.h"

class PartSymbolsWidget : public QFrame {
Q_OBJECT
	public:
		PartSymbolsWidget(SketchModel *sketchModel, class WaitPushUndoStack *undoStack, QWidget *parent);
		void copySvgFilesToDestiny();
		void loadViewsImagesFromModel(PaletteModel *paletteModel, ModelPart *modelPart);
		const QDir& tempDir();

	signals:
		void connectorsFound(QList<Connector *>);

	protected:
		void createViewImageWidget(
				SketchModel* sketchModel, class WaitPushUndoStack *undoStack, PartsEditorViewImageWidget *&viw,
				ItemBase::ViewIdentifier viewId, QString iconFileName, QString startText
			);

		friend class ConnectorsViewsWidget;
		PartsEditorViewImageWidget *m_breadView;
		PartsEditorViewImageWidget *m_schemView;
		PartsEditorViewImageWidget *m_pcbView;
};

#endif /* PARTSYMBOLSWIDGET_H_ */
