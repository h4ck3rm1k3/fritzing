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

#ifndef HOLE_H
#define HOLE_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QComboBox>
#include <QDoubleValidator>

#include "paletteitem.h"

class Hole : public PaletteItem 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	Hole(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~Hole();

	QString getProperty(const QString & key);
	void setProp(const QString & prop, const QString & value);
	void setHoleSize(QString holeSize, bool force);
	QString holeSize();
	bool collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	bool hasCustomSVG();
	PluralType isPlural();
	QString retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi); 

protected slots:
	void changeDiameter();
	void changeThickness();
	void changeUnits(const QString &);
	void changeHoleSize(const QString &);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	QString makeSvg(const QString & holeDiameter, const QString & ringThickness);
	void updateValidators();
	void updateEditTexts();
	void updateSizes();
	virtual QString makeID();
	virtual QPointF ringThicknessRange();
	virtual QPointF holeDiameterRange();
	virtual void setBoth(const QString & holeDiameter, const QString &  thickness);
	LayerKinPaletteItem * newLayerKinPaletteItem(PaletteItemBase * chief, ModelPart * modelPart, 
												 ViewIdentifierClass::ViewIdentifier viewIdentifier,
												 const ViewGeometry & viewGeometry, long id,
												 ViewLayer::ViewLayerID viewLayerID, 
												 ViewLayer::ViewLayerSpec viewLayerSpec, 
												 QMenu* itemMenu, const LayerHash & viewLayers);
protected:
	class FSvgRenderer * m_renderer;
	QString m_holeDiameter;
	QString m_ringThickness;
	QPointer<QDoubleValidator> m_diameterValidator;
	QPointer<QDoubleValidator> m_thicknessValidator;
	QPointer<QLineEdit> m_diameterEdit;
	QPointer<QLineEdit> m_thicknessEdit;
	QPointer<QComboBox> m_unitsComboBox;
	QPointer<QComboBox> m_sizesComboBox;

protected:
	static QHash<QString, QString> m_holeSizes;
	static QHash<QString, QString> m_holeSizeTranslations;
};

#endif
