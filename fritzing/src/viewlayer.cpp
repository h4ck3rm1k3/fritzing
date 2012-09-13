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

#include "viewlayer.h"
#include "debugdialog.h"
#include <qmath.h>

double ViewLayer::zIncrement = 0.00001;  // 0.000000001;

QHash<ViewLayer::ViewLayerID, NamePair * > ViewLayer::names;
QMultiHash<ViewLayer::ViewLayerID, ViewLayer::ViewLayerID> ViewLayer::alternatives;
QMultiHash<ViewLayer::ViewLayerID, ViewLayer::ViewLayerID> ViewLayer::unconnectables;
QHash<QString, ViewLayer::ViewLayerID> ViewLayer::xmlHash;

const QString ViewLayer::Copper0Color = "#F28A00";//"#ff9400";
const QString ViewLayer::Copper1Color = "#FFCB33"; //#ffbf00";
const QString ViewLayer::Silkscreen1Color = "#ffffff";
const QString ViewLayer::Silkscreen0Color = "#bbbbcc";
const QString ViewLayer::BoardColor = "#338040";

static LayerList CopperBottomLayers;
static LayerList CopperTopLayers;
static LayerList NonCopperLayers;  // just NonCopperLayers in pcb view

static LayerList ii;
static LayerList bb;
static LayerList ss;
static LayerList pp;
static LayerList ee;

NamePair::NamePair(QString xml, QString display)
{
    xmlName = xml;
    displayName = display;
}

//////////////////////////////////////////////

class NameTriple {

public:
	NameTriple(const QString & _xmlName, const QString & _viewName, const QString & _naturalName) {
		m_xmlName = _xmlName;
		m_viewName = _viewName;
		m_naturalName = _naturalName;
	}

	QString & xmlName() {
		return m_xmlName;
	}

	QString & viewName() {
		return m_viewName;
	}

	QString & naturalName() {
		return m_naturalName;
	}

protected:
	QString m_xmlName;
	QString m_naturalName;
	QString m_viewName;
};

//////////////////////////////////////////////

ViewLayer::ViewLayer(ViewLayerID viewLayerID, bool visible, double initialZ )
{
	m_viewLayerID = viewLayerID;
	m_visible = visible;
	m_action = NULL;
	m_initialZ = m_nextZ = initialZ;	
	m_parentLayer = NULL;
	m_active = true;
	m_includeChildLayers = true;

}

ViewLayer::~ViewLayer() {
}

void ViewLayer::initNames() {
	if (CopperBottomLayers.isEmpty()) {
		CopperBottomLayers << ViewLayer::GroundPlane0 << ViewLayer::Copper0 << ViewLayer::Copper0Trace;
	}
	if (CopperTopLayers.isEmpty()) {
		CopperTopLayers << ViewLayer::GroundPlane1 << ViewLayer::Copper1 << ViewLayer::Copper1Trace;
	}
	if (NonCopperLayers.isEmpty()) {
		NonCopperLayers << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label
						<< ViewLayer::Silkscreen1 << ViewLayer::Silkscreen1Label 
						<< ViewLayer::PartImage;
	}

	if (names.count() == 0) {
		// xmlname, displayname
		names.insert(ViewLayer::Icon, new NamePair("icon", QObject::tr("Icon")));
		names.insert(ViewLayer::BreadboardBreadboard, new NamePair("breadboardbreadboard", QObject::tr("Breadboard")));
		names.insert(ViewLayer::Breadboard, new NamePair("breadboard", QObject::tr("Parts")));
		names.insert(ViewLayer::BreadboardWire,  new NamePair("breadboardWire", QObject::tr("Wires")));
		names.insert(ViewLayer::BreadboardLabel,  new NamePair("breadboardLabel", QObject::tr("Part Labels")));
		names.insert(ViewLayer::BreadboardRatsnest, new NamePair("breadboardRatsnest", QObject::tr("Ratsnest")));
		names.insert(ViewLayer::BreadboardNote,  new NamePair("breadboardNote", QObject::tr("Notes")));
		names.insert(ViewLayer::BreadboardRuler,  new NamePair("breadboardRuler", QObject::tr("Rulers")));

		names.insert(ViewLayer::SchematicFrame,  new NamePair("schematicframe", QObject::tr("Frame")));
		names.insert(ViewLayer::Schematic,  new NamePair("schematic", QObject::tr("Parts")));
		names.insert(ViewLayer::SchematicWire,  new NamePair("schematicWire",QObject::tr("Ratsnest")));
		names.insert(ViewLayer::SchematicTrace,  new NamePair("schematicTrace",QObject::tr("Wires")));
		names.insert(ViewLayer::SchematicLabel,  new NamePair("schematicLabel", QObject::tr("Part Labels")));
		names.insert(ViewLayer::SchematicNote,  new NamePair("schematicNote", QObject::tr("Notes")));
		names.insert(ViewLayer::SchematicRuler,  new NamePair("schematicRuler", QObject::tr("Rulers")));

		names.insert(ViewLayer::Board,  new NamePair("board", QObject::tr("Board")));
		names.insert(ViewLayer::Silkscreen1,  new NamePair("silkscreen", QObject::tr("Silkscreen Top")));			// really should be silkscreen1
		names.insert(ViewLayer::Silkscreen1Label,  new NamePair("silkscreenLabel", QObject::tr("Silkscreen Top (Part Labels)")));
		names.insert(ViewLayer::GroundPlane0,  new NamePair("groundplane", QObject::tr("Copper Fill Bottom")));
		names.insert(ViewLayer::Copper0,  new NamePair("copper0", QObject::tr("Copper Bottom")));
		names.insert(ViewLayer::Copper0Trace,  new NamePair("copper0trace", QObject::tr("Copper Bottom Trace")));
		names.insert(ViewLayer::GroundPlane1,  new NamePair("groundplane1", QObject::tr("Copper Fill Top")));
		names.insert(ViewLayer::Copper1,  new NamePair("copper1", QObject::tr("Copper Top")));
		names.insert(ViewLayer::Copper1Trace,  new NamePair("copper1trace", QObject::tr("Copper Top Trace")));
		names.insert(ViewLayer::PcbRatsnest, new NamePair("ratsnest", QObject::tr("Ratsnest")));
		names.insert(ViewLayer::Silkscreen0,  new NamePair("silkscreen0", QObject::tr("Silkscreen Bottom")));
		names.insert(ViewLayer::Silkscreen0Label,  new NamePair("silkscreen0Label", QObject::tr("Silkscreen Bottom (Part Labels)")));
		//names.insert(ViewLayer::Soldermask,  new NamePair("soldermask",  QObject::tr("Solder mask")));
		//names.insert(ViewLayer::Outline,  new NamePair("outline",  QObject::tr("Outline")));
		//names.insert(ViewLayer::Keepout, new NamePair("keepout", QObject::tr("Keep out")));
		names.insert(ViewLayer::PartImage, new NamePair("partimage", QObject::tr("Part Image")));
		names.insert(ViewLayer::PcbNote,  new NamePair("pcbNote", QObject::tr("Notes")));
		names.insert(ViewLayer::PcbRuler,  new NamePair("pcbRuler", QObject::tr("Rulers")));

		foreach (ViewLayerID key, names.keys()) {
			xmlHash.insert(names.value(key)->xmlName, key);
		}

		names.insert(ViewLayer::UnknownLayer,  new NamePair("unknown", QObject::tr("Unknown Layer")));

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

	if (ViewIdentifierNames.count() == 0) {
		ViewIdentifierNames.insert(ViewLayer::IconView, new NameTriple("iconView", QObject::tr("icon view"), "icon"));
		ViewIdentifierNames.insert(ViewLayer::BreadboardView, new NameTriple("breadboardView", QObject::tr("breadboard view"), "breadboard"));
		ViewIdentifierNames.insert(ViewLayer::SchematicView, new NameTriple("schematicView", QObject::tr("schematic view"), "schematic"));
		ViewIdentifierNames.insert(ViewLayer::PCBView, new NameTriple("pcbView", QObject::tr("pcb view"), "pcb"));
	}

	if (bb.count() == 0) {
		ii << ViewLayer::Icon;
		bb << ViewLayer::BreadboardBreadboard << ViewLayer::Breadboard 
			<< ViewLayer::BreadboardWire << ViewLayer::BreadboardLabel 
			<< ViewLayer::BreadboardRatsnest 
			<< ViewLayer::BreadboardNote << ViewLayer::BreadboardRuler;
		ss << ViewLayer::SchematicFrame << ViewLayer::Schematic 
			<< ViewLayer::SchematicWire 
			<< ViewLayer::SchematicTrace << ViewLayer::SchematicLabel 
			<< ViewLayer::SchematicNote <<  ViewLayer::SchematicRuler;
		pp << ViewLayer::Board << ViewLayer::GroundPlane0 
			<< ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label
			<< ViewLayer::Copper0 
			<< ViewLayer::Copper0Trace << ViewLayer::GroundPlane1 
			<< ViewLayer::Copper1 << ViewLayer::Copper1Trace 
			<< ViewLayer::PcbRatsnest 
			<< ViewLayer::Silkscreen1 << ViewLayer::Silkscreen1Label 
			<< ViewLayer::PartImage 
			<< ViewLayer::PcbNote << ViewLayer::PcbRuler;
	}
}

QString & ViewLayer::displayName() {
	return names[m_viewLayerID]->displayName;
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

double ViewLayer::nextZ() {
	double temp = m_nextZ;
	m_nextZ += zIncrement;
	return temp;
}

ViewLayer::ViewLayerID ViewLayer::viewLayerIDFromXmlString(const QString & viewLayerName) {
	return xmlHash.value(viewLayerName, ViewLayer::UnknownLayer);
}

ViewLayer::ViewLayerID ViewLayer::viewLayerID() {
	return m_viewLayerID;
}

double ViewLayer::incrementZ(double z) {
	return (z + zIncrement);
}

double ViewLayer::getZIncrement() {
	return zIncrement;
}

const QString & ViewLayer::viewLayerNameFromID(ViewLayerID viewLayerID) {
	NamePair * sp = names.value(viewLayerID);
	if (sp == NULL) return ___emptyString___;

	return sp->displayName;
}

const QString & ViewLayer::viewLayerXmlNameFromID(ViewLayerID viewLayerID) {
	NamePair * sp = names.value(viewLayerID);
	if (sp == NULL) return ___emptyString___;

	return sp->xmlName;
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

bool ViewLayer::alreadyInLayer(double z) {
	return (z >= m_initialZ && z <= m_nextZ);
}

void ViewLayer::cleanup() {
	foreach (NamePair * sp, names.values()) {
		delete sp;
	}
	names.clear();

	foreach (NameTriple * nameTriple, ViewIdentifierNames) {
		delete nameTriple;
	}
	ViewIdentifierNames.clear();
}

void ViewLayer::resetNextZ(double z) {
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

bool ViewLayer::isActive() {
	return m_active;
}

void ViewLayer::setActive(bool a) {
	m_active = a;
}

ViewLayer::ViewLayerSpec ViewLayer::specFromID(ViewLayer::ViewLayerID viewLayerID) 
{
	switch (viewLayerID) {
		case Copper1:
		case Copper1Trace:
		case GroundPlane1:
			return ViewLayer::Top;
		default:
			return ViewLayer::Bottom;
	}
}

bool ViewLayer::isCopperLayer(ViewLayer::ViewLayerID viewLayerID) {
	if (CopperTopLayers.contains(viewLayerID)) return true;
	if (CopperBottomLayers.contains(viewLayerID)) return true;
	
	return false;
}


const LayerList & ViewLayer::copperLayers(ViewLayer::ViewLayerSpec viewLayerSpec) 
{
	if (viewLayerSpec == ViewLayer::Top) return CopperTopLayers;
	
	return CopperBottomLayers;
}

const LayerList & ViewLayer::maskLayers(ViewLayer::ViewLayerSpec viewLayerSpec) {
	static LayerList bottom;
	static LayerList top;
	if (bottom.isEmpty()) {
		bottom << ViewLayer::Copper0;
	}
	if (top.isEmpty()) {
		top << ViewLayer::Copper1;
	}
	if (viewLayerSpec == ViewLayer::Top) return top;
	
	return bottom;
}

const LayerList & ViewLayer::silkLayers(ViewLayer::ViewLayerSpec viewLayerSpec) {
	static LayerList bottom;
	static LayerList top;
	if (bottom.isEmpty()) {
		bottom << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label;
	}
	if (top.isEmpty()) {
		top << ViewLayer::Silkscreen1 << ViewLayer::Silkscreen1Label;
	}
	if (viewLayerSpec == ViewLayer::Top) return top;
	
	return bottom;
}

const LayerList & ViewLayer::outlineLayers() {
	static LayerList layerList;
	if (layerList.isEmpty()) {
		layerList << ViewLayer::Board;
	}
	
	return layerList;
}

const LayerList & ViewLayer::drillLayers() {
	static LayerList layerList;
	if (layerList.isEmpty()) {
		layerList << ViewLayer::Copper0 << ViewLayer::Copper1;
	}
	
	return layerList;
}

bool ViewLayer::isNonCopperLayer(ViewLayer::ViewLayerID viewLayerID) {
	// for pcb view layers only
	return NonCopperLayers.contains(viewLayerID);
}


bool ViewLayer::includeChildLayers() {
	return m_includeChildLayers;
} 

void ViewLayer::setIncludeChildLayers(bool incl) {
	m_includeChildLayers = incl;
}

/////////////////////////////////

QHash <ViewLayer::ViewIdentifier, NameTriple * > ViewLayer::ViewIdentifierNames;

QString & ViewLayer::viewIdentifierName(ViewLayer::ViewIdentifier viewIdentifier) {
	if (viewIdentifier < 0 || viewIdentifier >= ViewLayer::ViewCount) {
		throw "ViewLayer::viewIdentifierName bad identifier";
	}

	return ViewIdentifierNames[viewIdentifier]->viewName();
}

QString & ViewLayer::viewIdentifierXmlName(ViewLayer::ViewIdentifier viewIdentifier) {
	if (viewIdentifier < 0 || viewIdentifier >= ViewLayer::ViewCount) {
		throw "ViewLayer::viewIdentifierXmlName bad identifier";
	}

	return ViewIdentifierNames[viewIdentifier]->xmlName();
}

QString & ViewLayer::viewIdentifierNaturalName(ViewLayer::ViewIdentifier viewIdentifier) {
	if (viewIdentifier < 0 || viewIdentifier >= ViewLayer::ViewCount) {
		throw "ViewLayer::viewIdentifierNaturalName bad identifier";
	}

	return ViewIdentifierNames[viewIdentifier]->naturalName();
}

ViewLayer::ViewIdentifier ViewLayer::idFromXmlName(const QString & name) {
	foreach (ViewIdentifier id, ViewIdentifierNames.keys()) {
		NameTriple * nameTriple = ViewIdentifierNames.value(id);
		if (name.compare(nameTriple->xmlName()) == 0) return id;
	}

	return UnknownView;
}

const LayerList & ViewLayer::layersForView(ViewLayer::ViewIdentifier viewIdentifier) {
	switch(viewIdentifier) {
		case IconView:
			return ii;
		case BreadboardView:
			return bb;
		case SchematicView:
			return ss;
		case PCBView:
			return pp;
		default:
			return ee;
	}
}

bool ViewLayer::viewHasLayer(ViewLayer::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID) {
	return layersForView(viewIdentifier).contains(viewLayerID);
}
