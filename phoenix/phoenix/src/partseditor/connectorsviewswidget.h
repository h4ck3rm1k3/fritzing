/*
 * (c) Fachhochschule Potsdam
 */

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
