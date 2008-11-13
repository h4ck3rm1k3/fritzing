/*
 * (c) Fachhochschule Potsdam
 */

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
