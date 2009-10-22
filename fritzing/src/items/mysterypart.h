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

#ifndef MYSTERYPART_H
#define MYSTERYPART_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>

#include "paletteitem.h"

class MysteryPart : public PaletteItem 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	MysteryPart(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~MysteryPart();

	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	QString collectExtraInfoHtml(const QString & prop, const QString & value);
	QString getProperty(const QString & key);
	void setProp(const QString & prop, const QString & value);
	void setChipLabel(QString label, bool force);
	QString chipLabel();
	const QString & title();
	bool hasCustomSVG();

protected:
	QString makeSvg(const QString & chipLabel);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	QString replaceText(QString svg, const QString & chipLabel);

protected:
	class FSvgRenderer * m_renderer;
	QString m_chipLabel;
	QString m_title;
};

#endif
