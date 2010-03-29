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


ItemBase * PartFactory::createPart( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, QMenu * wireMenu)
{
	switch (modelPart->itemType()) {
		case ModelPart::Wire:
		{
			bool virtualWire = viewGeometry.getVirtual();
			if (virtualWire) {
				return new VirtualWire(modelPart, viewIdentifier, viewGeometry, id, wireMenu);		
			}
			else {
				if (viewGeometry.getTrace()) {
					return new TraceWire(modelPart, viewIdentifier, viewGeometry, id, wireMenu);
				}
				else {
					return new Wire(modelPart, viewIdentifier, viewGeometry, id, wireMenu);
				}
			}

		}
		case ModelPart::Note:
		{
			return new Note(modelPart, viewIdentifier, viewGeometry, id, NULL);
		}
		case ModelPart::CopperFill:
			return new GroundPlane(modelPart, viewIdentifier, viewGeometry, id, itemMenu);
		case ModelPart::Jumper:
			return new JumperItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu);
		case ModelPart::ResizableBoard:
			return new ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu);
		case ModelPart::Logo:
			return new LogoItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu);
		case ModelPart::Ruler:
			return new Ruler(modelPart, viewIdentifier, viewGeometry, id, itemMenu);
		case ModelPart::Symbol:
			return new SymbolPaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu);
		default:
			{
				QString family = modelPart->properties().value("family", "");
				if (modelPart->moduleID().compare(ModuleIDNames::resistorModuleIDName) == 0) {
					return new Resistor(modelPart, viewIdentifier, viewGeometry, id, itemMenu, true);
				}
				if (family.compare("mystery part", Qt::CaseInsensitive) == 0) {
					return new MysteryPart(modelPart, viewIdentifier, viewGeometry, id, itemMenu, true);
				}
				if (family.compare("pin header", Qt::CaseInsensitive) == 0) {
					return new PinHeader(modelPart, viewIdentifier, viewGeometry, id, itemMenu, true);
				}
				if (family.compare("generic IC", Qt::CaseInsensitive) == 0) {
					return new Dip(modelPart, viewIdentifier, viewGeometry, id, itemMenu, true);
				}
				return new PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu);

			}
	}
}
