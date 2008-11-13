/*
 * (c) Fachhochschule Potsdam
 */

#ifndef PARTSEDITORVIEWIMAGEWIDGET_H_
#define PARTSEDITORVIEWIMAGEWIDGET_H_

#include "partseditorabstractviewimage.h"
#include "partseditorpaletteitem.h"

class PartsEditorViewImageWidget : public PartsEditorAbstractViewImage
{
Q_OBJECT

public:
	PartsEditorViewImageWidget(ItemBase::ViewIdentifier, QDir tempDir, QGraphicsItem *startItem=0, QWidget *parent=0, int size=150);
	void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	void loadSvgFile(ModelPart * modelPart);
	void loadFile();
	void copySvgFileToDestiny();

	const QString svgFilePath();
	const StringPair& svgFileSplit();

public slots:
	void loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart);

signals:
	void loadedFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
	void itemAddedToSymbols(ModelPart * modelPart, StringPair *svgFilePath);

protected:
	void setSvgFilePath(QString filePath);
	void copyToTempAndRenameIfNecessary(StringPair *filePathOrig);
	void mousePressEvent(QMouseEvent *event);

	StringPair *m_svgFilePath;
	QString m_originalSvgFilePath;
	QDir m_tempFolder;
};
#endif /* PARTSEDITORVIEWIMAGEWIDGET_H_ */
