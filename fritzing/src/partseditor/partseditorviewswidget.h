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

$Revision: 2715 $:
$Author: merunga $:
$Date: 2009-03-24 18:18:30 +0100 (Tue, 24 Mar 2009) $

********************************************************************/


#ifndef PARTSEDITORVIEWSWIDGET_H_
#define PARTSEDITORVIEWSWIDGET_H_

#include <QWidget>

#include "partseditorview.h"
#include "connectorsinfowidget.h"

class PartsEditorViewsWidget : public QFrame {
Q_OBJECT
	public:
		PartsEditorViewsWidget(SketchModel *sketchModel, class WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info, QWidget *parent);
		void copySvgFilesToDestiny(const QString &partFileName);
		void loadViewsImagesFromModel(PaletteModel *paletteModel, ModelPart *modelPart);
		const QDir& tempDir();
		void aboutToSave();
		QCheckBox *showTerminalPointsCheckBox();

		bool imagesLoadedInAllViews();

		PartsEditorView *breadboardView();
		PartsEditorView *schematicView();
		PartsEditorView *pcbView();

		void connectTerminalRemoval(const ConnectorsInfoWidget* connsInfo);
		bool connectorsPosOrSizeChanged();

	public slots:
		void repaint();
		void drawConnector(Connector*);
		void drawConnector(ViewIdentifierClass::ViewIdentifier, Connector*);
		void removeConnectorFrom(const QString&,ViewIdentifierClass::ViewIdentifier);
		void showHideTerminalPoints(int checkState);
		void informConnectorSelection(const QString &connId);
		void setMismatching(ViewIdentifierClass::ViewIdentifier viewId, const QString &id, bool mismatching);

	signals:
		void connectorsFound(QList<Connector *>);
		void connectorSelectedInView(const QString& connId);

	protected:
		void createViewImageWidget(
			SketchModel* sketchModel, class WaitPushUndoStack *undoStack, PartsEditorView *&viw,
			ViewIdentifierClass::ViewIdentifier viewId, QString iconFileName, QString startText,
			ConnectorsInfoWidget* info, ViewLayer::ViewLayerID viewLayerId
		);
		void init();

		QWidget *addZoomControlsAndBrowseButton(PartsEditorView *view);

		bool showingTerminalPoints();
		bool checkStateToBool(int checkState);

		void connectPair(PartsEditorView *v1, PartsEditorView *v2);
		void connectToThis(PartsEditorView *v);

		PartsEditorView *m_breadView;
		PartsEditorView *m_schemView;
		PartsEditorView *m_pcbView;
		QHash<ViewIdentifierClass::ViewIdentifier,PartsEditorView*> m_views;

		QCheckBox *m_showTerminalPointsCheckBox;
		QLabel *m_guidelines;

		bool m_connsPosChanged;

	protected:
		static QString EmptyBreadViewText;
		static QString EmptySchemViewText;
		static QString EmptyPcbViewText;
};

#endif /* PARTSEDITORVIEWSWIDGET_H_ */
