/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

#ifndef VIEWLAYER_H
#define VIEWLAYER_H

#include <QString>
#include <QAction>
#include <QHash>

#include "misc.h"

class ViewLayer
{
public:
	enum ViewLayerID {
		Icon,
		BreadboardBreadboard,
		Breadboard,
		BreadboardWire,
		BreadboardRuler,
		Schematic,
		SchematicWire,
		SchematicRuler,
		Board,
		Copper1,
		Copper0,
		Keepout,
		Vias,
		Soldermask,
		Silkscreen,
		Outline,
		Jumperwires,
		Ratsnest,
		PcbRuler,
		UnknownLayer,
		ViewLayerCount
	};

protected:
	static qreal zIncrement;
	static QHash<ViewLayerID, StringPair *> names;

public:
	ViewLayer(ViewLayerID, bool visible, qreal initialZ);

	void setAction(QAction *);
	QAction* action();
	QString & displayName();
	bool visible();
	void setVisible(bool);
	qreal nextZ();
	ViewLayerID viewLayerID();
	qreal incrementZ(qreal);

	static ViewLayerID viewLayerIDFromString(const QString &);
	static ViewLayerID viewLayerIDFromXmlString(const QString &);
	static const QString & viewLayerNameFromID(ViewLayerID);
	static const QString & viewLayerXmlNameFromID(ViewLayerID);
	static void initNames();
	static qreal getZIncrement();

protected:
	bool m_visible;
	ViewLayerID m_viewLayerID;
	QAction* m_action;
	qreal m_nextZ;
};


typedef QHash<ViewLayer::ViewLayerID, ViewLayer *> LayerHash;
#endif
