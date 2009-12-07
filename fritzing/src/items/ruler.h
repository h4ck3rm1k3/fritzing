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

#ifndef RULER_H
#define RULER_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>

#include "paletteitem.h"

class Ruler : public PaletteItem 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	Ruler(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel = true);
	~Ruler();

	virtual void resizeMM(qreal w, qreal h, const LayerHash & viewLayers);
 	void loadLayerKin(const LayerHash & viewLayers);
	void setInitialSize();
	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	bool collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue);
	void saveParams();
	void getParams(QPointF &, QSizeF &);
	bool hasCustomSVG();
	QObject * createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);


public slots:
	void widthEntry();

protected:
	QString makeSvg(qreal mmW, qreal milsW);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	
protected:
	static qreal convertToInches(const QString & string);

protected:
	class FSvgRenderer * m_renderer;
	QPointer<QLineEdit> m_widthEditor;
};

#endif
