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

#include "hole.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"
#include "../utils/textutils.h"
#include "../layerattributes.h"
#include "../viewlayer.h"
#include "partlabel.h"
#include "../utils/focusoutcombobox.h"
#include "../utils/boundedregexpvalidator.h"
#include "../connectors/nonconnectoritem.h"
#include "../connectors/svgidlayer.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QLineEdit>
#include <QHBoxLayout>

static QStringList HoleDiameters;
static QStringList RingThicknesses;

static const int IndexMm = 0;
static const int IndexIn = 1;

Hole::Hole( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_holeDiameter = modelPart->prop("hole diameter").toString();
	if (m_holeDiameter.isEmpty()) {
		m_holeDiameter = modelPart->properties().value("hole diameter", ".055in");
		modelPart->setProp("hole diameter", m_holeDiameter);
	}
	m_ringThickness = modelPart->prop("ring thickness").toString();
	if (m_ringThickness.isEmpty()) {
		m_ringThickness = modelPart->properties().value("ring thickness", ".01in");
		modelPart->setProp("ring thickness", m_ringThickness);
	}

	m_renderer = NULL;
}

Hole::~Hole() {
}

QStringList & Hole::holeDiameters() {
	if (HoleDiameters.count() == 0) {
		HoleDiameters << "1mm" << "2mm" << "3mm" << "4mm" << "5mm" << "6mm";
	}
	return HoleDiameters;
}

QStringList & Hole::ringThicknesses() {
	if (RingThicknesses.count() == 0) {
		RingThicknesses << "1mm" << "2mm" << "3mm";
	}
	return RingThicknesses;
}

void Hole::setProp(const QString & prop, const QString & value) {
	if (prop.compare("hole diameter", Qt::CaseInsensitive) == 0) {
		setHoleDiameter(value, false);
		return;
	}

	if (prop.compare("ring thickness", Qt::CaseInsensitive) == 0) {
		setRingThickness(value, false);
		return;
	}

	PaletteItem::setProp(prop, value);
}

void Hole::setHoleDiameter(QString holeDiameter, bool force) {
	if (!force && m_holeDiameter.compare(holeDiameter) == 0) return;

	setBoth(holeDiameter, m_ringThickness);

	m_holeDiameter = holeDiameter;
	modelPart()->setProp("hole diameter", holeDiameter);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTexts();
}

void Hole::setRingThickness(QString ringThickness, bool force) {
	if (!force && m_ringThickness.compare(ringThickness) == 0) return;

	setBoth(m_holeDiameter, ringThickness);

	m_ringThickness = ringThickness;
	modelPart()->setProp("ring thickness", ringThickness);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTexts();
}

void Hole::setBoth(const QString & holeDiameter, const QString & ringThickness) {
	if (this->m_viewIdentifier != ViewIdentifierClass::PCBView) return;

	QString svg = makeSvg(holeDiameter, ringThickness);
	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	QString setColor;
	QStringList noIDs;
	if (m_viewLayerID == ViewLayer::Copper0) {
		setColor = ViewLayer::Copper0Color;
	}
	else if (m_viewLayerID == ViewLayer::Copper1) {
		setColor = ViewLayer::Copper1Color;
	}
	bool result = m_renderer->loadSvg(svg.toLatin1(), m_filename, noIDs, noIDs, setColor, ViewLayer::viewLayerXmlNameFromID(m_viewLayerID), true);
	if (result) {
		setSharedRenderer(m_renderer);
	}

	// there's only one NonConnectorItem
	foreach (SvgIdLayer * svgIdLayer, m_renderer->setUpNonConnectors()) {
		if (svgIdLayer == NULL) continue;

		foreach (QGraphicsItem * child, childItems()) {
			NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(child);
			if (nonConnectorItem == NULL) continue;

			nonConnectorItem->setRect(svgIdLayer->m_rect);
			nonConnectorItem->setRadius(svgIdLayer->m_radius, svgIdLayer->m_strokeWidth);
			break;
		}

		delete svgIdLayer;
	}
}

QString Hole::makeSvg(const QString & holeDiameter, const QString & ringThickness) 
{
	qreal hd = TextUtils::convertToInches(holeDiameter) * GraphicsUtils::StandardFritzingDPI;
	qreal rt = TextUtils::convertToInches(ringThickness) * GraphicsUtils::StandardFritzingDPI;

	qreal wInches = (hd + rt + rt) / GraphicsUtils::StandardFritzingDPI;
	QString svg = TextUtils::makeSVGHeader(1, GraphicsUtils::StandardFritzingDPI, wInches, wInches);
	svg += "<g id='copper0' >";
	
	QString id = FSvgRenderer::NonConnectorName + "0";
	if (rt == 0) {
		svg += QString("<circle cx='%1' cy='%1' r='%1' fill='black' id='%2' />")
					.arg(hd / 2)
					.arg(id);
	}
	else if (hd == 0) {
		svg += QString("<circle cx='%1' cy='%1' r='%1' fill='%2' id='%3' />")
					.arg(rt)
					.arg(ViewLayer::Copper0Color)
					.arg(id);
	}
	else {
		svg += QString("<circle fill='none' cx='%1' cy='%1' r='%2' stroke-width='%3' stroke='%4' id='%5' />")
			.arg((hd / 2) + rt)
			.arg((hd / 2) + (rt / 2))
			.arg(rt)
			.arg(ViewLayer::Copper0Color)
			.arg(id);
	}
  		
	svg += "</g></svg>";
	return svg;
}

QStringList Hole::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("hole diameter", Qt::CaseInsensitive) == 0) {
		QStringList values;
		foreach (QString f, holeDiameters()) {
			values.append(f);
		}
		value = m_holeDiameter;
		return values;
	}

	if (prop.compare("ring thickness", Qt::CaseInsensitive) == 0) {
		QStringList values;
		foreach (QString f, ringThicknesses()) {
			values.append(f);
		}

		value = m_ringThickness;
		return values;
	}

	return PaletteItem::collectValues(family, prop, value);
}

QString Hole::getProperty(const QString & key) {
	if (key.compare("hole diameter", Qt::CaseInsensitive) == 0) {
		return m_holeDiameter;
	}

	if (key.compare("ring thickness", Qt::CaseInsensitive) == 0) {
		return m_ringThickness;
	}

	return PaletteItem::getProperty(key);
}

QVariant Hole::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSceneHasChanged:
			if (this->scene()) {
				setHoleDiameter(m_holeDiameter, true);
				setRingThickness(m_ringThickness, true);
			}
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}

bool Hole::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::PCBView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

ItemBase::PluralType Hole::isPlural() {
	return Plural;
}

QString Hole::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	if (viewLayerID != ViewLayer::Copper0) {
		return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	}

	QString svg = makeSvg(m_modelPart->prop("hole diameter").toString(), m_modelPart->prop("ring thickness").toString());
	if (!svg.isEmpty()) {
		QString xmlName = ViewLayer::viewLayerXmlNameFromID(viewLayerID);
		SvgFileSplitter splitter;
		bool result = splitter.splitString(svg, xmlName);
		if (!result) {
			return "";
		}
		result = splitter.normalize(dpi, xmlName, blackOnly);
		if (!result) {
			return "";
		}
		return splitter.elementString(xmlName);
	}

	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

bool Hole::collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue) 
{
	if (prop.compare("hole diameter", Qt::CaseInsensitive) == 0) {
		returnProp = tr("hole diameter");
		returnValue = QString("<object type='application/x-qt-plugin' classid='HoleDiameter' swappingenabled='%1' width='100%' height='22px'></object>")
			.arg(swappingEnabled);  
		return true;
	}

	if (prop.compare("ring thickness", Qt::CaseInsensitive) == 0) {
		returnProp = tr("ring thickness");
		returnValue = QString("<object type='application/x-qt-plugin' classid='RingThickness' swappingenabled='%1' width='100%' height='22px'></object>")
			.arg(swappingEnabled);  
		return true;
	}

	return PaletteItem::collectExtraInfoHtml(family, prop, value, swappingEnabled, returnProp, returnValue);
}

QObject * Hole::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) 
{
	if (classid.compare("HoleDiameter", Qt::CaseInsensitive) == 0) {

		return makePlugin("hole diameter", "ring thickness", holeDiameters(), parent, paramNames, paramValues);
	}

	if (classid.compare("RingThickness", Qt::CaseInsensitive) == 0) {

		return makePlugin("ring thickness", "holeDiameter", ringThicknesses(), parent, paramNames, paramValues);
	}

	return PaletteItem::createPlugin(parent, classid, url, paramNames, paramValues);
}

QFrame * Hole::makePlugin(const QString & propName, const QString & otherPropName, QStringList & values, QWidget * parent, const QStringList &paramNames, const QStringList &paramValues) 
{
	bool swappingEnabled = getSwappingEnabled(paramNames, paramValues);
	int units = m_modelPart->prop(propName).toString().contains("mm") ? IndexMm : IndexIn;
	FocusOutComboBox * e1 = new FocusOutComboBox();
	e1->addItems(values);
	e1->setEditable(true);

	BoundedRegExpValidator * validator = new BoundedRegExpValidator(e1);
	validator->setConverter(TextUtils::convertToInches);
	setValidatorBounds(validator, otherPropName, units);
	validator->setRegExp(QRegExp("((\\d{1,2})|(\\d{1,2}\\.)|(\\d{1,2}\\.\\d{1,3}))(in|mm){0,1}"));
	e1->setValidator(validator);
	e1->setEnabled(swappingEnabled);
	
	QComboBox * comboBox = new QComboBox();
	comboBox->setEditable(false);
	comboBox->setEnabled(swappingEnabled);
	comboBox->addItem("mm");
	comboBox->addItem("in");
	comboBox->setCurrentIndex(units);

	HoleWidgetSet * holeWidgetSet = new HoleWidgetSet;
	holeWidgetSet->valueEditor = e1;
	holeWidgetSet->validator = validator;
	holeWidgetSet->unitsEditor = comboBox;
	holeWidgetSet->getValues = propName.compare("hole diameter") == 0 ? &Hole::holeDiameters : &Hole::ringThicknesses;
	m_holeWidgetSets.insert(propName, holeWidgetSet);

	setCurrentValue(e1, propName);

	QHBoxLayout * hboxLayout = new QHBoxLayout();
	hboxLayout->setAlignment(Qt::AlignLeft);
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	hboxLayout->setSpacing(0);

	hboxLayout->addWidget(e1);
	hboxLayout->addWidget(comboBox);

	QFrame * frame = new QFrame(parent);
	frame->setLayout(hboxLayout);

	connect(e1, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(valueEntry(const QString &)));
	connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(unitsEntry(const QString &)));

	frame->setMaximumWidth(200);

	return frame;
}

void Hole::valueEntry(const QString & text) {
	QString propName;
	QString otherPropName;
	HoleWidgetSet * holeWidgetSet = NULL;
	HoleWidgetSet * otherHoleWidgetSet = NULL;

	foreach (QString key, m_holeWidgetSets.keys()) {
		HoleWidgetSet * hws = m_holeWidgetSets.value(key);
		if (sender() == hws->valueEditor) {
			propName = key;
			holeWidgetSet = hws;
		}
		else {
			otherPropName = key;
			otherHoleWidgetSet = hws;
		}
	}

	if (holeWidgetSet == NULL) return;
	if (otherHoleWidgetSet == NULL) return;

	QString newValue = text;
	if (!newValue.endsWith("in") && !newValue.endsWith("mm")) {
		newValue += holeWidgetSet->unitsEditor->currentText();
	}

	int units = newValue.endsWith("in") ? IndexIn : IndexMm;
	modelPart()->setProp(propName, newValue);
	setCurrentValue(holeWidgetSet->valueEditor, propName);
	setBoth(modelPart()->prop("hole diameter").toString(), modelPart()->prop("ring thickness").toString());
	setValidatorBounds(otherHoleWidgetSet->validator, propName, units);

	if (!newValue.endsWith(holeWidgetSet->unitsEditor->currentText())) {
		unitsEntry(units == IndexIn ? "in" : "mm");
	}
}

void Hole::unitsEntry(const QString & units) 
{
	QString propName;
	QString otherPropName;
	HoleWidgetSet * holeWidgetSet = NULL;

	foreach (QString key, m_holeWidgetSets.keys()) {
		HoleWidgetSet * hws = m_holeWidgetSets.value(key);
		if (sender() == hws->unitsEditor) {
			propName = key;
			holeWidgetSet = hws;
		}
		else {
			otherPropName = key;
		}
	}

	if (holeWidgetSet == NULL) return;
	if (otherPropName.isEmpty()) return;

	qreal inches = TextUtils::convertToInches(modelPart()->prop(propName).toString());
	if (units == "in") {
		modelPart()->setProp(propName, QString::number(inches) + "in");
		setCurrentValue(holeWidgetSet->valueEditor, propName);
		setValidatorBounds(holeWidgetSet->validator, otherPropName, IndexIn);
	}
	else {
		modelPart()->setProp(propName, QString::number(inches * 25.4) + "mm");
		setCurrentValue(holeWidgetSet->valueEditor, propName);
		setValidatorBounds(holeWidgetSet->validator, otherPropName, IndexMm);
	}
}

void Hole::setValidatorBounds(BoundedRegExpValidator * validator, const QString & otherPropName, int units)
{
	qreal otherValue = TextUtils::convertToInches(m_modelPart->prop(otherPropName).toString());
	validator->setBounds(((otherValue == 0) ? .001 : 0) * ((units == IndexMm) ? 25.4 : 1), 1 * ((units == IndexMm) ? 25.4 : 1));
}

void Hole::setCurrentValue(QComboBox * comboBox, const QString & propName) 
{
	QString value = m_modelPart->prop(propName).toString();
	HoleWidgetSet * holeWidgetSet = m_holeWidgetSets.value(propName);
	if (holeWidgetSet == NULL) return;

	int count = holeWidgetSet->getValues().count();

	for (int ix = 0; ix < count; ix++) {
		if (holeWidgetSet->getValues().at(ix).compare(value) == 0) {
			comboBox->setCurrentIndex(ix);
			return;
		}
	}

	holeWidgetSet->getValues().append(value);
	comboBox->addItem(value);
	comboBox->setCurrentIndex(count);
}
