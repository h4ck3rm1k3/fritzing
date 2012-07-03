/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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
	const QString & form();
	PluralType isPlural();
	void addedToScene(bool temporary);
	bool collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);

protected:
	QStringList collectValues(const QString & family, const QString & prop, QString & value);


public slots:
	void swapEntry(const QString & text);

public:
	static QString FemaleFormString;
	static QString FemaleRoundedFormString;
	static QString MaleFormString;
	static QString ShroudedFormString;
	static QString LongPadFormString;
	static QString MolexFormString;

public:
	static void initNames();
	static QString genFZP(const QString & moduleid);
	static QString makePcbSvg(const QString & expectedFileName, const QString & moduleID);
	static QString makePcbShroudedSvg(int pins);
	static QString makePcbLongPadSvg(int pins, bool lock);
	static QString makePcbMolexSvg(int pins);
	static QString makePcbSMDSvg(const QString & expectedFileName, const QString & moduleID);
	static QString makeSchematicSvg(const QString & expectedFileName, const QString & moduleID);
	static QString makeBreadboardSvg(const QString & expectedFileName, const QString & moduleID);
	static QString makeBreadboardShroudedSvg(int pins);
    static QString makeBreadboardDoubleSvg(const QString & expectedFileName, int pins);

protected:
	static const QStringList & forms();
	static void initSpacings();
	static QString genModuleID(QMap<QString, QString> & currPropsMap);

protected:
	QString m_form;
};

#endif
