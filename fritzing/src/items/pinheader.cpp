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

#include "pinheader.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"
#include "../utils/textutils.h"
#include "../layerattributes.h"
#include "partlabel.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QLineEdit>

static QStringList Forms;
QString PinHeader::FemaleFormString;
QString PinHeader::FemaleRoundedFormString;
QString PinHeader::MaleFormString;


// TODO
//	save into parts bin

PinHeader::PinHeader( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_changingForm = false;
	m_form = modelPart->prop("form").toString();
	if (m_form.isEmpty()) {
		m_form = modelPart->properties().value("form", FemaleFormString);
		modelPart->setProp("form", m_form);
	}

	m_renderer = NULL;
}

PinHeader::~PinHeader() {
}


void PinHeader::initNames() {
	if (FemaleFormString.isEmpty()) {
		FemaleFormString = FemaleSymbolString + " (female)";
		FemaleRoundedFormString = FemaleSymbolString + " (female rounded)";
		MaleFormString = MaleSymbolString + " (male)";
	}
}

void PinHeader::setProp(const QString & prop, const QString & value) {
	if (prop.compare("form", Qt::CaseInsensitive) == 0) {
		setForm(value, false);
		return;
	}

	PaletteItem::setProp(prop, value);
}

void PinHeader::setForm(QString form, bool force) {
	if (!force && m_form.compare(form) == 0) return;

	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
			{
				InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
				if (infoGraphicsView == NULL) break;

				// hack the dom element and call setUpImage
				FSvgRenderer::removeFromHash(this->modelPart()->moduleID(), "");
				QDomElement element = LayerAttributes::getSvgElementLayers(modelPart()->domDocument(), m_viewIdentifier);
				if (element.isNull()) break;

				QString filename = element.attribute("image");
				if (filename.isEmpty()) break;

				QString genericString("generic_");
				int gix = filename.indexOf(genericString);
				if (gix < 0) break;

				gix += genericString.length();
				int pix = filename.indexOf("_pin_header");
				if (pix < 0) break;

				QString prefix = filename.left(gix);
				QString suffix = filename.remove(0, pix);

				if (form.contains("(male)", Qt::CaseInsensitive)) {
					filename = prefix + "male" + suffix;
				}
				else if (form.contains("rounded", Qt::CaseInsensitive)) {
					filename = prefix + 
						((this->m_viewIdentifier == ViewIdentifierClass::SchematicView) ? "female" : "rounded_female") + 
						suffix;
				}
				else if (form.contains("(female)", Qt::CaseInsensitive)) {
					filename = prefix + "female" + suffix;
				}
				else {
					break;
				}

				element.setAttribute("image", filename);

				m_changingForm = true;
				resetImage(infoGraphicsView);
				m_changingForm = false;
				
				updateConnections();
			}
			break;
		default:
			break;
	}

	m_form = form;
	modelPart()->setProp("form", form);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTexts();

}

QStringList PinHeader::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("form", Qt::CaseInsensitive) == 0) {
		QStringList values;
		foreach (QString f, forms()) {
			values.append(f);
		}

		value = m_form;
		return values;
	}

	return PaletteItem::collectValues(family, prop, value);
}

QString PinHeader::getProperty(const QString & key) {
	if (key.compare("form", Qt::CaseInsensitive) == 0) {
		return m_form;
	}

	return PaletteItem::getProperty(key);
}

QVariant PinHeader::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSceneHasChanged:
			if (this->scene()) {
				setForm(m_form, true);
			}
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}

ConnectorItem* PinHeader::newConnectorItem(Connector *connector) {
	if (m_changingForm) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), viewLayerID());
	}

	return PaletteItem::newConnectorItem(connector);
}

ConnectorItem* PinHeader::newConnectorItem(ItemBase * layerKin, Connector *connector) {
	if (m_changingForm) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), layerKin->viewLayerID());
	}

	return PaletteItem::newConnectorItem(layerKin, connector);
}

const QString & PinHeader::form() {
	return m_form;
}


const QStringList & PinHeader::forms() {
	if (Forms.count() == 0) {
		Forms << FemaleFormString << FemaleRoundedFormString << MaleFormString;
	}
	return Forms;
}

bool PinHeader::onlyFormChanges(QMap<QString, QString> & propsMap) {
	if (propsMap.value("form", "").compare(m_form) == 0) return false;

	if (modelPart()->properties().value("pins", "").compare(propsMap.value("pins", "")) != 0) return false;

	return true;
}

bool PinHeader::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

ItemBase::PluralType PinHeader::isPlural() {
	return Plural;
}

void PinHeader::syncKinSceneChanged(PaletteItemBase * originator) {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView == NULL) return;

	m_changingForm = true;
	resetKinImage(originator, infoGraphicsView);
	m_changingForm = false;
}
