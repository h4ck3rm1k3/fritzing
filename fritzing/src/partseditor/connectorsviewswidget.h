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



#ifndef CONNECTORSVIEWSWIDGET_H_
#define CONNECTORSVIEWSWIDGET_H_

#include <QFrame>
#include <QUndoStack>

#include "partseditorconnectorsview.h"
#include "partseditorspecificationsview.h"
#include "partsymbolswidget.h"

class ConnectorsViewsWidget : public QFrame {
	Q_OBJECT
	public:
		ConnectorsViewsWidget(PartSymbolsWidget *symbols, SketchModel *sketchModel, class WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info, QWidget *parent=0);
		void aboutToSave();
		QCheckBox *showTerminalPointsCheckBox();

	public slots:
		void repaint();
		void drawConnector(Connector*);
		void removeConnectorFrom(const QString&,ItemBase::ViewIdentifier);
		void showHideTerminalPoints(int checkState);
		void informConnectorSelection(const QString &connId);

	signals:
		void connectorSelectedInView(const QString& connId);

	protected:
		void createViewImageWidget(
				PartsEditorConnectorsView *&viw, PartsEditorSpecificationsView* sister,
				SketchModel* sketchModel, class WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info,
				ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerId);
		QWidget *addZoomControls(PartsEditorConnectorsView *view);

		bool showingTerminalPoints();
		bool checkStateToBool(int checkState);

		void connectPair(PartsEditorConnectorsView *v1, PartsEditorConnectorsView *v2);
		void connectToThis(PartsEditorConnectorsView *v);

		PartsEditorConnectorsView *m_breadView;
		PartsEditorConnectorsView *m_schemView;
		PartsEditorConnectorsView *m_pcbView;

		QCheckBox *m_showTerminalPointsCheckBox;
};

#endif /* CONNECTORSVIEWSWIDGET_H_ */
