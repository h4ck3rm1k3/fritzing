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

$Revision: 2398 $:
$Author: cohen@irascible.com $:
$Date: 2009-02-17 00:53:10 +0100 (Tue, 17 Feb 2009) $

********************************************************************/





#include "iconwidgetpaletteitem.h"

IconWidgetPaletteItem::IconWidgetPaletteItem(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel) {
}

void IconWidgetPaletteItem::setDefaultTooltip() {
	if (m_modelPart) {
		QString base = ITEMBASE_FONT_PREFIX + "%1" + ITEMBASE_FONT_SUFFIX;
		if(m_modelPart->itemType() != ModelPart::Wire) {
			this->setToolTip(base.arg(m_modelPart->title()));
		} else {
			this->setToolTip(base.arg(m_modelPart->modelPartStuff()->title() + " (" + m_modelPart->modelPartStuff()->moduleID() + ")"));
		}
	}
}
