/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
	bool collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	QString getProperty(const QString & key);
	void setProp(const QString & prop, const QString & value);
	void setChipLabel(QString label, bool force);
	QString chipLabel();
	const QString & title();
	bool hasCustomSVG();
	void setSpacing(QString spacing, bool force);
	const QString & spacing();
	bool onlySpacingChanges(QMap<QString, QString> & propsMap);
	PluralType isPlural();

public slots:
	void chipLabelEntry();

protected:
	QString makeSvg(const QString & chipLabel);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	ConnectorItem* newConnectorItem(class Connector *connector);
	QStringList collectValues(const QString & family, const QString & prop, QString & value);
	virtual bool isDIP();
	virtual bool otherPropsChange(const QMap<QString, QString> & propsMap);
	virtual const QStringList & spacings();

protected:
	class FSvgRenderer * m_renderer;
	QString m_chipLabel;
	QString m_title;
	bool m_changingSpacing;
	QString m_spacing;
};

#endif
