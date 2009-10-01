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

#include "mysterypart.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"

static QRegExp Text("<text.*>.*</text>");
static QRegExp Subtext(">.*</text>");

// TODO
//	save into parts bin

MysteryPart::MysteryPart( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_chipLabel = modelPart->prop("chiplabel").toString();
	if (m_chipLabel.isEmpty()) {
		m_chipLabel = modelPart->properties().value("chip label", "?");
		modelPart->setProp("chiplabel", m_chipLabel);
	}

	m_renderer = NULL;
}

MysteryPart::~MysteryPart() {
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
		DebugDialog::debug(svg);

		bool result = m_renderer->fastLoad(svg.toUtf8());
		if (result) {
			setSharedRenderer(m_renderer);
		}
	}

	m_chipLabel = chipLabel;
	modelPart()->setProp("chiplabel", chipLabel);

	updateTooltip();
}

QString MysteryPart::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	QString svg = PaletteItem::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	switch (viewLayerID) {
		case ViewLayer::Breadboard:
		case ViewLayer::Schematic:
			return replaceText(svg, m_chipLabel);
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
	}
	return replaceText(svg, chipLabel);
}

QString MysteryPart::replaceText(QString svg, const QString & chipLabel) {
	if (svg.indexOf(Text) < 0) return svg;

	QString text = Text.cap(0);
	text.replace(Subtext, ">" + chipLabel + "</text>");
	svg.replace(Text, text);
	return svg;
}

QString MysteryPart::collectExtraInfoHtml(const QString & prop, const QString & value) {
	Q_UNUSED(value);

	if (prop.compare("chip label", Qt::CaseInsensitive) == 0) {
		return QString("&nbsp;<input type='text' name='sChipLabel' id='sChipLabel' maxlength='15' value='%1' style='width:95px' onblur='setChipLabel()' onkeypress='setChipLabelEnter(event)' />"
					   ).arg(m_chipLabel);
	}

	return ___emptyString___;
}

QString MysteryPart::getProperty(const QString & key) {
	if (key.compare("chip label", Qt::CaseInsensitive) == 0) {
		return m_chipLabel;
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


