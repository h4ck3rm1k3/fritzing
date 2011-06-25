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

#ifndef STRIPBOARD_H
#define STRIPBOARD_H

#include <QRectF>
#include <QPainterPath>

#include "perfboard.h"

class Stripbit : public QGraphicsPathItem
{
public:
	Stripbit(const QPainterPath & path, QGraphicsItem * parent);
	~Stripbit();


};

class Stripboard : public Perfboard 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	Stripboard(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~Stripboard();

	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi);
	bool collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	void addedToScene();
	void setProp(const QString & prop, const QString & value);

public:
	static QString genFZP(const QString & moduleID);
	static QString makeBreadboardSvg(const QString & size);
	static QString genModuleID(QMap<QString, QString> & currPropsMap);


protected slots:
	void changeBoardSize();
};

#endif
