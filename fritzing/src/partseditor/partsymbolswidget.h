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



#ifndef PARTSYMBOLSWIDGET_H_
#define PARTSYMBOLSWIDGET_H_

#include <QWidget>

#include "partseditorspecificationsview.h"
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
				SketchModel* sketchModel, class WaitPushUndoStack *undoStack, PartsEditorSpecificationsView *&viw,
				ItemBase::ViewIdentifier viewId, QString iconFileName, QString startText
			);
		void init();

		friend class ConnectorsViewsWidget;
		PartsEditorSpecificationsView *m_breadView;
		PartsEditorSpecificationsView *m_schemView;
		PartsEditorSpecificationsView *m_pcbView;

		QLabel *m_guidelines;

	protected:
		static QString EmptyBreadViewText;
		static QString EmptySchemViewText;
		static QString EmptyPcbViewText;
};

#endif /* PARTSYMBOLSWIDGET_H_ */
