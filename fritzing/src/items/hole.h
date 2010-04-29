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

#ifndef HOLE_H
#define HOLE_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QComboBox>
#include <QDoubleValidator>

#include "paletteitem.h"

struct HoleWidgetSet {
	QComboBox * valueEditor;
	class BoundedRegExpValidator * validator;
	QComboBox * unitsEditor;
	QStringList * values;
};

class Hole : public PaletteItem 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	Hole(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~Hole();

	QString getProperty(const QString & key);
	void setProp(const QString & prop, const QString & value);
	void setHoleDiameter(QString diameter, bool force);
	void setRingThickness(QString thickness, bool force);
	bool collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue);
	bool hasCustomSVG();
	QObject * createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
	PluralType isPlural();
	QString retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi); 

protected slots:
	void valueEntry();
	void unitsEntry(const QString &);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	QStringList collectValues(const QString & family, const QString & prop, QString & value);
	void setBoth(const QString & holeDiameter, const QString &  thickness);
	const QStringList & holeDiameters();
	const QStringList & ringThicknesses();
	QString makeSvg(const QString & holeDiameter, const QString & ringThickness);
	QFrame * makePlugin(const QString & propName, const QString & otherPropName, const QStringList & values, QWidget * parent, const QStringList &paramNames, const QStringList &paramValues); 
	void setValidatorBounds(class BoundedRegExpValidator * validator, const QString & otherPropName, int units);
	void setValidatorBounds(QComboBox *, const QString & propName);
	void setCurrentValue(QComboBox * comboBox, const QString & propName);

protected:
	class FSvgRenderer * m_renderer;
	QString m_holeDiameter;
	QString m_ringThickness;
	QHash<QString, HoleWidgetSet *> m_holeWidgetSets;
};

#endif
