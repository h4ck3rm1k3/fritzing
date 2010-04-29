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

#include "viewlayer.h"
#include "debugdialog.h"
#include <qmath.h>

qreal ViewLayer::zIncrement = 0.00001;  // 0.000000001;

QHash<ViewLayer::ViewLayerID, StringPair * > ViewLayer::names;
QMultiHash<ViewLayer::ViewLayerID, ViewLayer::ViewLayerID> ViewLayer::alternatives;
QMultiHash<ViewLayer::ViewLayerID, ViewLayer::ViewLayerID> ViewLayer::unconnectables;
QHash<QString, ViewLayer::ViewLayerID> ViewLayer::xmlHash;

const QString ViewLayer::Copper0Color = "#ff9400";   
const QString ViewLayer::Copper1Color = "#ffbf00";
const QString ViewLayer::SilkscreenColor = "#ffffff";
const QString ViewLayer::Silkscreen0Color = "#bbbbcc";

ViewLayer::ViewLayer(ViewLayerID viewLayerID, bool visible, qreal initialZ )
{
	m_viewLayerID = viewLayerID;
	m_visible = visible;
	m_action = NULL;
	m_initialZ = m_nextZ = initialZ;	
	m_parentLayer = NULL;
}

ViewLayer::~ViewLayer() {
}

void ViewLayer::initNames() {
	if (names.count() == 0) {
		// xmlname, displayname
		names.insert(ViewLayer::Icon, new StringPair("icon", QObject::tr("Icon")));
		names.insert(ViewLayer::BreadboardBreadboard, new StringPair("breadboardbreadboard", QObject::tr("Breadboard")));
		names.insert(ViewLayer::Breadboard, new StringPair("breadboard", QObject::tr("Parts")));
		names.insert(ViewLayer::BreadboardWire,  new StringPair("breadboardWire", QObject::tr("Wires")));
		names.insert(ViewLayer::BreadboardLabel,  new StringPair("breadboardLabel", QObject::tr("Part Labels")));
		names.insert(ViewLayer::BreadboardNote,  new StringPair("breadboardNote", QObject::tr("Notes")));
		names.insert(ViewLayer::BreadboardRuler,  new StringPair("breadboardRuler", QObject::tr("Rulers")));

		names.insert(ViewLayer::Schematic,  new StringPair("schematic", QObject::tr("Parts")));
		names.insert(ViewLayer::SchematicWire,  new StringPair("schematicWire",QObject::tr("Nets")));
		names.insert(ViewLayer::SchematicTrace,  new StringPair("schematicTrace",QObject::tr("Wires")));
		names.insert(ViewLayer::SchematicLabel,  new StringPair("schematicLabel", QObject::tr("Part Labels")));
		names.insert(ViewLayer::SchematicNote,  new StringPair("schematicNote", QObject::tr("Notes")));
		names.insert(ViewLayer::SchematicRuler,  new StringPair("schematicRuler", QObject::tr("Rulers")));

		names.insert(ViewLayer::Board,  new StringPair("board", QObject::tr("Board")));
		names.insert(ViewLayer::Ratsnest, new StringPair("ratsnest", QObject::tr("Rat's nest")));
		names.insert(ViewLayer::Silkscreen,  new StringPair("silkscreen", QObject::tr("Silkscreen Top")));			// really should be silkscreen1
		names.insert(ViewLayer::SilkscreenLabel,  new StringPair("silkscreenLabel", QObject::tr("Silkscreen Top (Part Labels)")));
		names.insert(ViewLayer::GroundPlane,  new StringPair("groundplane", QObject::tr("Ground Plane")));
		names.insert(ViewLayer::Copper0,  new StringPair("copper0", QObject::tr("Copper Bottom")));
		names.insert(ViewLayer::Copper0Trace,  new StringPair("copper0trace", QObject::tr("Copper Bottom Trace")));
		names.insert(ViewLayer::Copper1,  new StringPair("copper1", QObject::tr("Copper Top")));
		names.insert(ViewLayer::Copper1Trace,  new StringPair("copper1trace", QObject::tr("Copper Top Trace")));
		names.insert(ViewLayer::Silkscreen0,  new StringPair("silkscreen0", QObject::tr("Silkscreen Bottom")));
		names.insert(ViewLayer::Silkscreen0Label,  new StringPair("silkscreen0Label", QObject::tr("Silkscreen Bottom (Part Labels)")));
		names.insert(ViewLayer::Soldermask,  new StringPair("soldermask",  QObject::tr("Solder mask")));
		names.insert(ViewLayer::Outline,  new StringPair("outline",  QObject::tr("Outline")));
		names.insert(ViewLayer::Vias, new StringPair("vias", QObject::tr("Vias")));
		names.insert(ViewLayer::Keepout, new StringPair("keepout", QObject::tr("Keep out")));
		names.insert(ViewLayer::Jumperwires, new StringPair("jumperwires", QObject::tr("Jumper wires")));
		names.insert(ViewLayer::PcbNote,  new StringPair("pcbNote", QObject::tr("Notes")));
		names.insert(ViewLayer::PcbRuler,  new StringPair("pcbRuler", QObject::tr("Rulers")));

		foreach (ViewLayerID key, names.keys()) {
			xmlHash.insert(names.value(key)->first, key);
		}

		names.insert(ViewLayer::UnknownLayer,  new StringPair("unknown", QObject::tr("Unknown Layer")));

		alternatives.insert(ViewLayer::Copper0, ViewLayer::Copper1);
		alternatives.insert(ViewLayer::Copper1, ViewLayer::Copper0);

		unconnectables.insert(ViewLayer::Copper0, ViewLayer::Copper1);
		unconnectables.insert(ViewLayer::Copper0, ViewLayer::Copper1Trace);
		unconnectables.insert(ViewLayer::Copper0Trace, ViewLayer::Copper1);
		unconnectables.insert(ViewLayer::Copper0Trace, ViewLayer::Copper1Trace);
		unconnectables.insert(ViewLayer::Copper1, ViewLayer::Copper0);
		unconnectables.insert(ViewLayer::Copper1, ViewLayer::Copper0Trace);
		unconnectables.insert(ViewLayer::Copper1Trace, ViewLayer::Copper0);
		unconnectables.insert(ViewLayer::Copper1Trace, ViewLayer::Copper0Trace);
	}

}

QString & ViewLayer::displayName() {
	return names[m_viewLayerID]->second;
}

void ViewLayer::setAction(QAction * action) {
	m_action = action;
}

QAction* ViewLayer::action() {
	return m_action;
}

bool ViewLayer::visible() {
	return m_visible;
}

void ViewLayer::setVisible(bool visible) {
	m_visible = visible;
	if (m_action) {
		m_action->setChecked(visible);
	}
}

qreal ViewLayer::nextZ() {
	qreal temp = m_nextZ;
	m_nextZ += zIncrement;
	return temp;
}

ViewLayer::ViewLayerID ViewLayer::viewLayerIDFromXmlString(const QString & viewLayerName) {
	return xmlHash.value(viewLayerName, ViewLayer::UnknownLayer);
}

ViewLayer::ViewLayerID ViewLayer::viewLayerID() {
	return m_viewLayerID;
}

qreal ViewLayer::incrementZ(qreal z) {
	return (z + zIncrement);
}

qreal ViewLayer::getZIncrement() {
	return zIncrement;
}

const QString & ViewLayer::viewLayerNameFromID(ViewLayerID viewLayerID) {
	StringPair * sp = names.value(viewLayerID);
	if (sp == NULL) return ___emptyString___;

	return sp->second;
}

const QString & ViewLayer::viewLayerXmlNameFromID(ViewLayerID viewLayerID) {
	StringPair * sp = names.value(viewLayerID);
	if (sp == NULL) return ___emptyString___;

	return sp->first;
}

ViewLayer * ViewLayer::parentLayer() {
	return m_parentLayer;
}

void ViewLayer::setParentLayer(ViewLayer * parent) {
	m_parentLayer = parent;
	if (parent) {
		parent->m_childLayers.append(this);
	}
}

const QList<ViewLayer *> & ViewLayer::childLayers() {
	return m_childLayers;
}


bool ViewLayer::alreadyInLayer(qreal z) {
	return (z >= m_initialZ && z <= m_nextZ);
}

void ViewLayer::cleanup() {
	foreach (StringPair * sp, names.values()) {
		delete sp;
	}
	names.clear();
}

void ViewLayer::resetNextZ(qreal z) {
	m_nextZ = qFloor(m_initialZ) + z - floor(z);
}

LayerList ViewLayer::findAlternativeLayers(ViewLayer::ViewLayerID id)
{
	LayerList alts = alternatives.values(id);
	return alts;
}

bool ViewLayer::canConnect(ViewLayer::ViewLayerID v1, ViewLayer::ViewLayerID v2) {
	if (v1 == v2) return true;

 	LayerList uncs = unconnectables.values(v1);
	return (!uncs.contains(v2));
}
