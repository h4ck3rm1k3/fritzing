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

#include "partfactory.h"
#include "../viewgeometry.h"
#include "../model/modelpart.h"
#include "paletteitem.h"
#include "symbolpaletteitem.h"
#include "wire.h"
#include "virtualwire.h"
#include "tracewire.h"
#include "jumperitem.h"
#include "resizableboard.h"
#include "logoitem.h"
#include "resistor.h"
#include "moduleidnames.h"
#include "mysterypart.h"
#include "groundplane.h"
#include "note.h"
#include "ruler.h"
#include "dip.h"
#include "pinheader.h"
#include "hole.h"
#include "via.h"
#include "capacitor.h"


ItemBase * PartFactory::createPart( ModelPart * modelPart, ViewLayer::ViewLayerSpec viewLayerSpec, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, QMenu * wireMenu, bool doLabel)
{
	modelPart->setModelIndexFromMultiplied(id);			// make sure the model index is synched with the id; this is not always the case when parts are first created.
	ItemBase * itemBase = createPartAux(modelPart, viewIdentifier, viewGeometry, id, itemMenu, wireMenu, doLabel);
	if (itemBase) {
		itemBase->setViewLayerSpec(viewLayerSpec);
	}
	return itemBase;
}

ItemBase * PartFactory::createPartAux( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, QMenu * wireMenu, bool doLabel)
{
	switch (modelPart->itemType()) {
		case ModelPart::Wire:
		{
			bool ratsnest = viewGeometry.getRatsnest();
			if (ratsnest) {
				return new VirtualWire(modelPart, viewIdentifier, viewGeometry, id, wireMenu);		
			}
			if (viewGeometry.getTrace()) {
				TraceWire * traceWire = new TraceWire(modelPart, viewIdentifier, viewGeometry, id, wireMenu);
				traceWire->setSchematic(viewIdentifier == ViewIdentifierClass::SchematicView);
				return traceWire;
			}
			return new Wire(modelPart, viewIdentifier, viewGeometry, id, wireMenu, true);

		}
		case ModelPart::Note:
		{
			return new Note(modelPart, viewIdentifier, viewGeometry, id, NULL);
		}
		case ModelPart::CopperFill:
			return new GroundPlane(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::Jumper:
			return new JumperItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::ResizableBoard:
			return new ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::Board:
			return new Board(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::Logo:
			return new LogoItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::Ruler:
			return new Ruler(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::Symbol:
			return new SymbolPaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::Via:
			return new Via(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		case ModelPart::Hole:
			return new Hole(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
		default:
			{
				QString moduleID = modelPart->moduleID();
				if (moduleID.endsWith(ModuleIDNames::ModuleIDNameSuffix)) {
					if (moduleID.endsWith(ModuleIDNames::ResistorModuleIDName)) {
						return new Resistor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
					}
					if (moduleID.endsWith(ModuleIDNames::CapacitorModuleIDName)) {
						return new Capacitor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
					}
					if (moduleID.endsWith(ModuleIDNames::CrystalModuleIDName)) {
						return new Capacitor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
					}
					if (moduleID.endsWith(ModuleIDNames::ThermistorModuleIDName)) {
						return new Capacitor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
					}
					if (moduleID.endsWith(ModuleIDNames::ZenerDiodeModuleIDName)) {
						return new Capacitor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
					}
					if (moduleID.endsWith(ModuleIDNames::PotentiometerModuleIDName)) {
						return new Capacitor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
					}
					if (moduleID.endsWith(ModuleIDNames::InductorModuleIDName)) {
						return new Capacitor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
					}
				}
				QString family = modelPart->properties().value("family", "");
				if (family.compare("mystery part", Qt::CaseInsensitive) == 0) {
					return new MysteryPart(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
				}
				if (family.compare("pin header", Qt::CaseInsensitive) == 0) {
					return new PinHeader(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
				}
				if (family.compare("generic IC", Qt::CaseInsensitive) == 0) {
					return new Dip(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);
				}
				return new PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel);

			}
	}
}
