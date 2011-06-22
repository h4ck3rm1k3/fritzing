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

#ifndef PINHEADER_H
#define PINHEADER_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>

#include "paletteitem.h"

class PinHeader : public PaletteItem 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	PinHeader(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~PinHeader();

	QString getProperty(const QString & key);
	void setProp(const QString & prop, const QString & value);
	void setForm(QString form, bool force);
	const QString & form();
	bool onlyFormChanges(QMap<QString, QString> & propsMap);
    bool hasCustomSVG();
	PluralType isPlural();
	void addedToScene();

protected:
	ConnectorItem* newConnectorItem(class Connector *connector);
	ConnectorItem* newConnectorItem(ItemBase * layerkin, Connector *connector);
	QStringList collectValues(const QString & family, const QString & prop, QString & value);
	const QStringList & forms();

public:
	static QString FemaleFormString;
	static QString FemaleRoundedFormString;
	static QString MaleFormString;
	static void initNames();
	static QString genFZP(const QString & moduleid);
	static QString genModuleID(QMap<QString, QString> & currPropsMap);
	static QString makePcbSvg(const QString & moduleID);

protected:
	static void initSpacings();

protected:
	class FSvgRenderer * m_renderer;
	QString m_form;
	bool m_changingForm;
};

#endif
