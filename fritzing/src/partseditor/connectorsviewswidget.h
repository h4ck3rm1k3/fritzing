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



#ifndef CONNECTORSVIEWSWIDGET_H_
#define CONNECTORSVIEWSWIDGET_H_

#include <QFrame>
#include <QUndoStack>

#include "partseditorconnectorviewimagewidget.h"
#include "partseditorviewimagewidget.h"
#include "partsymbolswidget.h"

class ConnectorsViewsWidget : public QFrame {
	Q_OBJECT
	public:
		ConnectorsViewsWidget(PartSymbolsWidget *symbols, SketchModel *sketchModel, class WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info, QWidget *parent=0);
		void aboutToSave();

	public slots:
		void repaint();
		void drawConnector(Connector*, bool showTerminalPoint);
		void removeConnectorFrom(const QString&,ItemBase::ViewIdentifier);
		void showTerminalPoints(bool);

	protected:
		void createViewImageWidget(
				PartsEditorConnectorViewImageWidget *&viw, PartsEditorViewImageWidget* sister,
				SketchModel* sketchModel, class WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info,
				ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerId);

		PartsEditorConnectorViewImageWidget *m_breadView;
		PartsEditorConnectorViewImageWidget *m_schemView;
		PartsEditorConnectorViewImageWidget *m_pcbView;
};

#endif /* CONNECTORSVIEWSWIDGET_H_ */
