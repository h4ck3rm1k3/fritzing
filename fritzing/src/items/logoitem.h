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

#ifndef LOGOITEM_H
#define LOGOITEMD_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QCheckBox>

#include "resizableboard.h"

class LogoItem : public ResizableBoard 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	LogoItem(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel = true);
	~LogoItem();

	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	bool collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue);
	void saveParams();
	void getParams(QPointF &, QSizeF &);
	QObject * createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
	QStringList collectValues(const QString & family, const QString & prop, QString & value);
	void resizeMM(qreal w, qreal h, const LayerHash & viewLayers);
	QString getProperty(const QString & key);
	void setLogo(QString logo, bool force);
	const QString & logo();
	bool canEditPart();
	void setProp(const QString & prop, const QString & value);
	bool hasPartLabel();

protected slots:
	void prepLoadImage();
	void logoEntry();
	void widthEntry();
	void heightEntry();
	void keepAspectRatio(bool checkState);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	bool hasGrips();
	QString hackSvg(const QString & svg, const QString & logo);
	void unableToLoad(const QString & fileName);

protected:
	QString m_logo;
	bool m_hasLogo;
	QString m_originalFilename;
	QCheckBox * m_aspectRatioCheck;
};

#endif
