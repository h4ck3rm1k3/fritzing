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

#include "mysterypart.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../commands.h"
#include "../utils/textutils.h"
#include "../layerattributes.h"
#include "partlabel.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QLineEdit>

static QStringList Spacings;
static QRegExp Digits("(\\d)+");
static QRegExp DigitsMil("(\\d)+mil");


// TODO
//	save into parts bin

MysteryPart::MysteryPart( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_changingSpacing = false;
	m_chipLabel = modelPart->prop("chip label").toString();
	if (m_chipLabel.isEmpty()) {
		m_chipLabel = modelPart->properties().value("chip label", "?");
		modelPart->setProp("chip label", m_chipLabel);
	}
	m_spacing = modelPart->prop("spacing").toString();
	if (m_spacing.isEmpty()) {
		m_spacing = modelPart->properties().value("spacing", "300mil");
		modelPart->setProp("spacing", m_spacing);
	}

	m_renderer = NULL;
}

MysteryPart::~MysteryPart() {
}

void MysteryPart::setProp(const QString & prop, const QString & value) {
	if (prop.compare("chip label", Qt::CaseInsensitive) == 0) {
		setChipLabel(value, false);
		return;
	}

	if (prop.compare("spacing", Qt::CaseInsensitive) == 0) {
		setSpacing(value, false);
		return;
	}

	PaletteItem::setProp(prop, value);
}

void MysteryPart::setSpacing(QString spacing, bool force) {
	if (!force && m_spacing.compare(spacing) == 0) return;
	if (!isDIP()) return;

	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::PCBView:
			{
				InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
				if (infoGraphicsView == NULL) break;

				// hack the dom element and call setUpImage
				FSvgRenderer::removeFromHash(moduleID(), "");
				QDomElement element = LayerAttributes::getSvgElementLayers(modelPart()->domDocument(), m_viewIdentifier);
				if (element.isNull()) break;

				QString filename = element.attribute("image");
				if (filename.isEmpty()) break;

				if (spacing.indexOf(Digits) < 0) break;

				QString newSpacing = Digits.cap(0);		
				filename.replace(DigitsMil, newSpacing + "mil");
				element.setAttribute("image", filename);

				m_changingSpacing = true;
				resetImage(infoGraphicsView);
				m_changingSpacing = false;

				if (m_viewIdentifier == ViewIdentifierClass::BreadboardView) {
					if (modelPart()->properties().value("chip label", "").compare(m_chipLabel) != 0) {
						setChipLabel(m_chipLabel, true);
					}
				}

				updateConnections();
			}
			break;
		default:
			break;
	}

	m_spacing = spacing;
	modelPart()->setProp("spacing", spacing);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTexts();

}

void MysteryPart::setChipLabel(QString chipLabel, bool force) {

	if (!force && m_chipLabel.compare(chipLabel) == 0) return;

	QString svg;
	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
			svg = makeSvg(chipLabel);
			break;
		default:
			break;
	}

	if (!svg.isEmpty()) {
		if (m_renderer == NULL) {
			m_renderer = new FSvgRenderer(this);
		}
		//DebugDialog::debug(svg);

		bool result = m_renderer->fastLoad(svg.toUtf8());
		if (result) {
			setSharedRendererEx(m_renderer);
		}
	}

	m_chipLabel = chipLabel;
	modelPart()->setProp("chip label", chipLabel);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTexts();

}

QString MysteryPart::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi) 
{
	QString svg = PaletteItem::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	switch (viewLayerID) {
		case ViewLayer::Breadboard:
		case ViewLayer::Schematic:
		case ViewLayer::Icon:
			return TextUtils::replaceTextElement(svg, m_chipLabel);
		default:
			break;
	}

	return svg; 
}

QString MysteryPart::makeSvg(const QString & chipLabel) {
	QString path = filename();
	QFile file(filename());
	QString svg;
	if (file.open(QFile::ReadOnly)) {
		svg = file.readAll();
		file.close();
		return TextUtils::replaceTextElement(svg, chipLabel);
	}

	return "";
}

QStringList MysteryPart::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("spacing", Qt::CaseInsensitive) == 0) {
		QStringList values;
		if (isDIP()) {
			foreach (QString f, spacings()) {
				values.append(f);
			}
		}
		else {
			values.append(m_spacing);
		}

		value = m_spacing;
		return values;
	}

	return PaletteItem::collectValues(family, prop, value);
}

bool MysteryPart::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	if (prop.compare("chip label", Qt::CaseInsensitive) == 0) {
		returnProp = tr("label");

		QLineEdit * e1 = new QLineEdit(parent);
		e1->setEnabled(swappingEnabled);
		e1->setText(m_chipLabel);
		connect(e1, SIGNAL(editingFinished()), this, SLOT(chipLabelEntry()));
		e1->setMaximumWidth(200);

		returnWidget = e1;

		return true;
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

QString MysteryPart::getProperty(const QString & key) {
	if (key.compare("chip label", Qt::CaseInsensitive) == 0) {
		return m_chipLabel;
	}

	if (key.compare("spacing", Qt::CaseInsensitive) == 0) {
		return m_spacing;
	}

	return PaletteItem::getProperty(key);
}

QString MysteryPart::chipLabel() {
	return m_chipLabel;
}

QVariant MysteryPart::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSceneHasChanged:
			if (this->scene()) {
				setChipLabel(m_chipLabel, true);
				setSpacing(m_spacing, true);
			}
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}

const QString & MysteryPart::title() {
	return m_chipLabel;
}

bool MysteryPart::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
		case ViewIdentifierClass::IconView:
		case ViewIdentifierClass::PCBView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

void MysteryPart::chipLabelEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	if (edit->text().compare(this->chipLabel()) == 0) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setProp(this, "chip label", tr("chip label"), this->chipLabel(), edit->text(), true);
	}
}

ConnectorItem* MysteryPart::newConnectorItem(Connector *connector) {
	if (m_changingSpacing) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), viewLayerID());
	}

	return PaletteItem::newConnectorItem(connector);
}

ConnectorItem* MysteryPart::newConnectorItem(ItemBase * layerKin, Connector *connector) {
	if (m_changingSpacing) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), layerKin->viewLayerID());
	}

	return PaletteItem::newConnectorItem(layerKin, connector);
}

const QString & MysteryPart::spacing() {
	return m_spacing;
}

bool MysteryPart::onlySpacingChanges(QMap<QString, QString> & propsMap) {
	if (propsMap.value("spacing", "").compare(m_spacing) == 0) return false;

	if (modelPart()->properties().value("pins", "").compare(propsMap.value("pins", "")) != 0) return false;

	if (otherPropsChange(propsMap)) return false;

	return true;
}

bool MysteryPart::isDIP() {
	QString layout = modelPart()->properties().value("layout", "");
	return (layout.indexOf("double", 0, Qt::CaseInsensitive) >= 0);
}

bool MysteryPart::otherPropsChange(const QMap<QString, QString> & propsMap) {
	QString layout = modelPart()->properties().value("layout", "");
	return (layout.compare(propsMap.value("layout", "")) != 0);
}

const QStringList & MysteryPart::spacings() {
	if (Spacings.count() == 0) {
		Spacings << "100mil" << "200mil" << "300mil" << "400mil" << "500mil" << "600mil" << "700mil" << "800mil" << "900mil";
	}
	return Spacings;
}

ItemBase::PluralType MysteryPart::isPlural() {
	return Plural;
}

void MysteryPart::syncKinSceneChanged(PaletteItemBase * originator) {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView == NULL) return;

	m_changingSpacing = true;
	resetKinImage(originator, infoGraphicsView);
	m_changingSpacing = false;
}
