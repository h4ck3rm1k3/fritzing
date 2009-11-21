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

$Revision: 2829 $:
$Author: cohen@irascible.com $:
$Date: 2009-04-17 00:22:27 +0200 (Fri, 17 Apr 2009) $

********************************************************************/

#ifndef RESIZABLEBOARD_H
#define RESIZABLEBOARD_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>

#include "paletteitem.h"

class ResizableBoard : public PaletteItem 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	ResizableBoard(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel = true);
	~ResizableBoard();

	bool setUpImage(ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID, bool doConnectors);
	void resizeMM(qreal w, qreal h, const LayerHash & viewLayers);
	void resizePixels(qreal w, qreal h, const LayerHash & viewLayers);
 	void loadLayerKin(const LayerHash & viewLayers);
	void setInitialSize();
	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	void rotateItem(qreal degrees);
	bool collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue);
	void saveParams();
	void getParams(QPointF &, QSizeF &);
	bool hasCustomSVG();
	QObject * createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

public:
	static QString customShapeTranslated;

public slots:
	void widthEntry();
	void heightEntry();

protected slots:
	void handleMousePressSlot(QGraphicsSceneMouseEvent * event, class ResizeHandle * resizeHandle);
	void handleZoomChangedSlot(qreal scale);

protected:
	void positionGrips();
	void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	QString makeBoardSvg(qreal mmW, qreal mmH, qreal milsW, qreal milsH);
	QString makeSilkscreenSvg(qreal mmW, qreal mmH, qreal milsW, qreal milsH);
	QStringList collectValues(const QString & family, const QString & prop, QString & value);
	
protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	class ResizeHandle * m_resizeGripTL;
	class ResizeHandle * m_resizeGripTR;
	class ResizeHandle * m_resizeGripBL;
	class ResizeHandle * m_resizeGripBR;
	class ResizeHandle * m_inResize;
	class FSvgRenderer * m_renderer;
	class FSvgRenderer * m_silkscreenRenderer;
	QSizeF m_boardSize;
	QPointF m_boardPos;
	QRectF m_originalRect;
	QPointer<QLineEdit> m_widthEditor;
	QPointer<QLineEdit> m_heightEditor;
};

#endif
