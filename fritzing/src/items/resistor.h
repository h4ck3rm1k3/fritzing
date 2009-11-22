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

#ifndef RESISTOR_H
#define RESISTOR_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>

#include "paletteitem.h"

class Resistor : public PaletteItem 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	Resistor(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~Resistor();

	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	bool collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue);
	QString getProperty(const QString & key);
	void setResistance(QString resistance, QString pinSpacing, bool force);
	QString resistance();
	QString pinSpacing();
	const QString & title();
	bool hasCustomSVG();
	bool canEditPart();
	QObject * createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

protected:
	QString makeBreadboardSvg(const QString & ohms);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateResistances(QString r);
	ConnectorItem* newConnectorItem(class Connector *connector);
	QStringList collectValues(const QString & family, const QString & prop, QString & value);

public slots:
	void resistanceEntry(const QString & text);


public:
	static qreal toOhms(const QString & ohmsString);

protected:
	class FSvgRenderer * m_renderer;
	QString m_ohms;
	QString m_pinSpacing;
	QString m_title;
	bool m_changingPinSpacing;
};

#endif
