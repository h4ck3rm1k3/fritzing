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
	Resistor(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel, LayerHash &);
	~Resistor();

	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	void collectExtraInfoValues(const QString & prop, QString & value, QStringList & extraValues, bool & ignoreValues);
	QString collectExtraInfoHtml(const QString & prop, const QString & value);
	QString getProperty(const QString & key);
	void setResistance(QString resistance, QString pinSpacing, LayerHash &, bool force);
	QString resistance();
	QString pinSpacing();
	const QString & title();

protected:
	QString makeBreadboardSvg(const QString & ohms);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updateResistances(QString r);
	ConnectorItem* newConnectorItem(class Connector *connector);

public:
	static qreal toOhms(const QString & ohmsString);

protected:
	class FSvgRenderer * m_renderer;
	QString m_ohms;
	QString m_pinSpacing;
	QString m_title;
	LayerHash m_layers;
	bool m_changingPinSpacing;
};

#endif
