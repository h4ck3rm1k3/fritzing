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



#ifndef PARTSEDITORSPECIFICATIONSVIEW_H_
#define PARTSEDITORSPECIFICATIONSVIEW_H_

#include "partseditorabstractview.h"
#include "partseditorpaletteitem.h"

class PartsEditorSpecificationsView : public PartsEditorAbstractView
{
Q_OBJECT

public:
	PartsEditorSpecificationsView(ItemBase::ViewIdentifier, QDir tempDir, QGraphicsItem *startItem=0, QWidget *parent=0, int size=150);
	void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	void loadSvgFile(ModelPart * modelPart);
	void loadFile();
	void copySvgFileToDestiny();

	const QString svgFilePath();
	const StringPair& svgFileSplit();

public slots:
	void loadFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
	void loadSvgFile(const QString& origPath);

signals:
	void loadedFromModel(PaletteModel *paletteModel, ModelPart * modelPart);
	void itemAddedToSymbols(ModelPart * modelPart, StringPair *svgFilePath);

protected:
	void setSvgFilePath(const QString &filePath);
	void copyToTempAndRenameIfNecessary(StringPair *filePathOrig);
	void mousePressEvent(QMouseEvent *event);
	void fitCenterAndDeselect();
	QString createSvgFromImage(const QString &filePath);

	QGraphicsItem *m_startItem;

	StringPair *m_svgFilePath;
	QString m_originalSvgFilePath;
	QDir m_tempFolder;
};
#endif /* PARTSEDITORSPECIFICATIONSVIEW_H_ */
