/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef SCHEMATIC_FRAME_H
#define SCHEMATIC_FRAME_H

#include <QRectF>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>

#include "resizableboard.h"

class SchematicFrame : public ResizableBoard 
{
	Q_OBJECT

public:
	// after calling this constructor if you want to render the loaded svg (either from model or from file), MUST call <renderImage>
	SchematicFrame(ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel);
	~SchematicFrame();

	QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi);
	bool collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	bool canEditPart();
	void setProp(const QString & prop, const QString & value);
	bool hasPartLabel();
	bool stickyEnabled();
	PluralType isPlural();
	void addedToScene(bool temporary);
	bool rotationAllowed();
	bool rotation45Allowed();
	bool hasPartNumberProperty();
	void setInitialSize();

protected slots:
	void propEntry();
	void dateTimeEntry(QDateTime);
	void sheetEntry(int);
	void incSheet();
	void incDate();

protected:
	bool hasGrips();
	qreal minWidth();
	qreal minHeight();
	ViewIdentifierClass::ViewIdentifier theViewIdentifier();
	void loadTemplates();
	QString makeLayerSvg(ViewLayer::ViewLayerID viewLayerID, qreal mmW, qreal mmH, qreal milsW, qreal milsH);
	QString makeFirstLayerSvg(qreal mmW, qreal mmH, qreal milsW, qreal milsH);
	QString makeNextLayerSvg(ViewLayer::ViewLayerID, qreal mmW, qreal mmH, qreal milsW, qreal milsH);
	bool makeLineEdit(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	QTimer m_sheetTimer;
	QTimer m_dateTimer;
	QTextEdit * m_textEdit;
	bool m_wrapInitialized;

protected:
};



#endif
