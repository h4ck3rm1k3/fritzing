/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include "../viewlayer.h"
#include "partlabel.h"
#include "../connectors/nonconnectoritem.h"
#include "../connectors/svgidlayer.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QGroupBox>
#include <QSettings>

const double Hole::OffsetPixels = 2;
QHash<QString, QString> Hole::HoleSizes;
const QString Hole::AutorouteViaHoleSize = "autorouteViaHoleSize";
const QString Hole::AutorouteViaRingThickness = "autorouteViaRingThickness";
QString Hole::DefaultAutorouteViaHoleSize;
QString Hole::DefaultAutorouteViaRingThickness;

//////////////////////////////////////////////////

Hole::Hole( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	initHoleSettings(m_holeSettings);

	if (HoleSizes.count() == 0) {
		setUpHoleSizes();
	}

	QString holeSize = modelPart->localProp("hole size").toString();
	QStringList sizes = holeSize.split(",");
	if (sizes.count() != 2) {
		holeSize = HoleSizes.value(holeSize, "");
		if (holeSize.isEmpty()) {
			holeSize = modelPart->properties().value("hole size", ".035in,0.2in");
			modelPart->setLocalProp("hole size", holeSize);
		}
		sizes = holeSize.split(",");
	}

	m_holeSettings.holeDiameter = sizes.at(0);
	m_holeSettings.ringThickness = sizes.at(1);
}

Hole::~Hole() {
}

void Hole::initHoleSettings(HoleSettings & holeSettings) 
{
    PaletteItem::initHoleSettings(holeSettings, &HoleSizes, Hole::holeDiameterRange,  Hole::ringThicknessRange);
}


void Hole::setUpHoleSizes() {
	QSettings settings;
	QString ringThickness = settings.value(AutorouteViaRingThickness, "").toString();
	QString holeSize = settings.value(AutorouteViaHoleSize, "").toString();

    bool holeSizeWasEmpty = holeSize.isEmpty();
    bool ringThicknessWasEmpty = holeSize.isEmpty();
    PaletteItem::setUpHoleSizes(holeSize, ringThickness, "hole", HoleSizes);

	if (ringThicknessWasEmpty) {
		settings.setValue(AutorouteViaRingThickness, ringThickness);
		DefaultAutorouteViaRingThickness = ringThickness;
	}

	if (holeSizeWasEmpty) {
		settings.setValue(AutorouteViaHoleSize, holeSize);
		DefaultAutorouteViaHoleSize = holeSize;
	}
}


void Hole::setProp(const QString & prop, const QString & value) {
	if (prop.compare("hole size", Qt::CaseInsensitive) == 0) {
		setHoleSize(value, false);
		return;
	}

	PaletteItem::setProp(prop, value);
}

QString Hole::holeSize() {
	return m_holeSettings.holeSize();
}


void Hole::setHoleSize(QString newSize, bool force) {
	//DebugDialog::debug(QString("old holesize %1").arg(viewIdentifier()) + holeSize(), sceneBoundingRect());
	//foreach (QGraphicsItem * childItem, childItems()) {
	//	DebugDialog::debug(QString("   child"), childItem->sceneBoundingRect());
	//}

	if (PaletteItem::setHoleSize(newSize, force, m_holeSettings)) {
		setBoth(m_holeSettings.holeDiameter, m_holeSettings.ringThickness);
		modelPart()->setLocalProp("hole size", newSize);

		if (m_partLabel) m_partLabel->displayTextsIf();	
	}
	//DebugDialog::debug(QString("new holesize %1 ").arg(viewIdentifier()) + holeSize(), sceneBoundingRect());
	//foreach (QGraphicsItem * childItem, childItems()) {
	//	DebugDialog::debug(QString("   child"), childItem->sceneBoundingRect());
	//}
}

QRectF Hole::getRect(const QString & newSize) {
	QString s = newSize;
	QStringList sizes = getSizes(s, m_holeSettings);
	if (sizes.count() < 2) return boundingRect();

	double diameter = TextUtils::convertToInches(sizes.at(0));
	double ringThickness = TextUtils::convertToInches(sizes.at(1));
	double dim = (diameter + ringThickness + ringThickness) * FSvgRenderer::printerScale();
	//DebugDialog::debug(QString("get rect %1 %2").arg(newSize).arg(dim));
	return QRectF(0, 0, dim, dim);
}

void Hole::setBoth(const QString & holeDiameter, const QString & ringThickness) {
	if (this->m_viewIdentifier != ViewIdentifierClass::PCBView) return;

	ItemBase * otherLayer = setBothSvg(holeDiameter, ringThickness);

	// there's only one NonConnectorItem
	foreach (SvgIdLayer * svgIdLayer, fsvgRenderer()->setUpNonConnectors()) {
		if (svgIdLayer == NULL) continue;

		setBothNonConnectors(this, svgIdLayer);
		if (otherLayer != NULL) {
			setBothNonConnectors(otherLayer, svgIdLayer);
		}

		delete svgIdLayer;
	}
}

ItemBase * Hole::setBothSvg(const QString & holeDiameter, const QString & ringThickness) 
{
	QString svg = makeSvg(holeDiameter, ringThickness, m_viewLayerID);
	resetRenderer(svg);
	//DebugDialog::debug("both");
	//DebugDialog::debug(svg);

	QString setColor;
	QStringList noIDs;

	QString osvg;
	ItemBase * otherLayer = NULL;
	foreach (ItemBase * layerKin, m_layerKin) {
		if (layerKin->hasNonConnectors()) {
			otherLayer = layerKin;
			break;
		}
	}

	if (otherLayer) {
		osvg = makeSvg(holeDiameter, ringThickness, otherLayer->viewLayerID());
		//DebugDialog::debug(osvg);
		otherLayer->resetRenderer(osvg);
	}

	//DebugDialog::debug("other");
	
	return otherLayer;
}

void Hole::setBothNonConnectors(ItemBase * itemBase, SvgIdLayer * svgIdLayer) {
	foreach (QGraphicsItem * child, itemBase->childItems()) {
		NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(child);
		if (nonConnectorItem == NULL) continue;

		//DebugDialog::debug(QString("hole set rect %1").arg(m_viewIdentifier), svgIdLayer->m_rect);
		nonConnectorItem->setRect(svgIdLayer->m_rect);
		nonConnectorItem->setRadius(svgIdLayer->m_radius, svgIdLayer->m_strokeWidth);
		break;
	}
}


QString Hole::makeSvg(const QString & holeDiameter, const QString & ringThickness, ViewLayer::ViewLayerID viewLayerID) 
{
	double offsetDPI = OffsetPixels / FSvgRenderer::printerScale();
	double hd = TextUtils::convertToInches(holeDiameter);
	double rt = TextUtils::convertToInches(ringThickness);

	double wInches = hd + rt + rt + offsetDPI + offsetDPI;
	QString svg = TextUtils::makeSVGHeader(1, GraphicsUtils::StandardFritzingDPI, wInches, wInches);

	hd *= GraphicsUtils::StandardFritzingDPI;
	rt *= GraphicsUtils::StandardFritzingDPI;
	offsetDPI *= GraphicsUtils::StandardFritzingDPI;

	QString setColor;
	if (viewLayerID == ViewLayer::Copper0) {
		setColor = ViewLayer::Copper0Color;
	}
	else if (viewLayerID == ViewLayer::Copper1) {
		setColor = ViewLayer::Copper1Color;
	}

	svg += QString("<g id='%1'>").arg(ViewLayer::viewLayerXmlNameFromID(viewLayerID));
	
	QString id = makeID();
	if (hd == 0) {
		svg += QString("<circle cx='%1' cy='%1' r='%2' fill='%3' id='%4' />")
					.arg(rt + offsetDPI)
					.arg(rt)
					.arg(setColor)
					.arg(id);
	}
	else {
		svg += QString("<circle fill='none' cx='%1' cy='%1' r='%2' stroke-width='%3' stroke='%4' id='%5' />")
			.arg((hd / 2) + rt + offsetDPI)
			.arg((hd / 2) + (rt / 2))
			.arg(rt)
			.arg(setColor)
			.arg(id);
		svg += QString("<circle drill='0' fill='black' cx='%1' cy='%1' r='%2' stroke-width='0'  />")   // set the drill attribute to 0 for gerber translation
			.arg((hd / 2) + rt + offsetDPI)
			.arg(hd / 2);
	}
  		
	svg += "</g></svg>";
	//DebugDialog::debug("hole svg " + svg);
	return svg;
}

QString Hole::makeID() {
	return FSvgRenderer::NonConnectorName + "0";
}

QString Hole::getProperty(const QString & key) {
	if (key.compare("hole size", Qt::CaseInsensitive) == 0) {
		return m_holeSettings.holeDiameter;
	}

	return PaletteItem::getProperty(key);
}

void Hole::addedToScene(bool temporary)
{
	if (this->scene()) {
		setHoleSize(m_holeSettings.holeSize(), true);
	}

    return PaletteItem::addedToScene(temporary);
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

QString Hole::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, double dpi) 
{
	if (m_viewIdentifier != ViewIdentifierClass::PCBView || 
		(viewLayerID != ViewLayer::Copper0 && viewLayerID != ViewLayer::Copper1)) 
	{
		return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	}

	QStringList holeSize = m_modelPart->localProp("hole size").toString().split(",");
	if (holeSize.length() == 2) {
		QString svg = makeSvg(holeSize.at(0), holeSize.at(1), viewLayerID);
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
	}

	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

bool Hole::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
	if (prop.compare("hole size", Qt::CaseInsensitive) == 0) {
		returnProp = tr("hole size");

		returnValue = m_modelPart->localProp("hole size").toString();
		QWidget * frame = createHoleSettings(parent, m_holeSettings, swappingEnabled, returnValue, true);

		connect(m_holeSettings.sizesComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeHoleSize(const QString &)));	
		connect(m_holeSettings.mmRadioButton, SIGNAL(toggled(bool)), this, SLOT(changeUnits(bool)));
		connect(m_holeSettings.inRadioButton, SIGNAL(toggled(bool)), this, SLOT(changeUnits(bool)));
		connect(m_holeSettings.diameterEdit, SIGNAL(editingFinished()), this, SLOT(changeDiameter()));
		connect(m_holeSettings.thicknessEdit, SIGNAL(editingFinished()), this, SLOT(changeThickness()));

		returnWidget = frame;
		return true;
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void Hole::changeThickness() 
{
	if (changeThickness(m_holeSettings, sender())) {
		QLineEdit * edit = qobject_cast<QLineEdit *>(sender());
		changeHoleSize(m_holeSettings.holeDiameter + "," + edit->text() + currentUnits());
	}	
}

bool Hole::changeThickness(HoleSettings & holeSettings, QObject * sender) 
{
	QLineEdit * edit = qobject_cast<QLineEdit *>(sender);
	if (edit == NULL) return false;

	double newValue = edit->text().toDouble();
	QString temp = holeSettings.ringThickness;
	temp.chop(2);
	double oldValue = temp.toDouble();
	return (newValue != oldValue);
}

void Hole::changeDiameter() 
{
	if (changeDiameter(m_holeSettings, sender())) {
		QLineEdit * edit = qobject_cast<QLineEdit *>(sender());
		changeHoleSize(edit->text() + currentUnits() + "," + m_holeSettings.ringThickness);
	}
}

bool Hole::changeDiameter(HoleSettings & holeSettings, QObject * sender) 
{
	QLineEdit * edit = qobject_cast<QLineEdit *>(sender);
	if (edit == NULL) return false;

	double newValue = edit->text().toDouble();
	QString temp = holeSettings.holeDiameter;
	temp.chop(2);
	double oldValue = temp.toDouble();
	return (newValue != oldValue);
}

void Hole::changeUnits(bool) 
{
	QString newVal = changeUnits(currentUnits(), m_holeSettings);
	modelPart()->setLocalProp("hole size", newVal);
}

QString Hole::changeUnits(const QString & units, HoleSettings & holeSettings) 
{
	double hd = TextUtils::convertToInches(holeSettings.holeDiameter);
	double rt = TextUtils::convertToInches(holeSettings.ringThickness);
	QString newVal;
	if (units == "in") {
		newVal = QString("%1in,%2in").arg(hd).arg(rt);
	}
	else {
		newVal = QString("%1mm,%2mm").arg(hd * 25.4).arg(rt * 25.4);
	}

	QStringList sizes = newVal.split(",");
	holeSettings.ringThickness = sizes.at(1);
	holeSettings.holeDiameter = sizes.at(0);

	updateValidators(holeSettings);
	updateSizes(holeSettings);
	updateEditTexts(holeSettings);
	
	return newVal;
}


QPointF Hole::ringThicknessRange(const QString & holeDiameter) {
	double hd = TextUtils::convertToInches(holeDiameter);
	QPointF p(hd > 0 ? 0 : .001, 10.0);
	return p;
}

QPointF Hole::holeDiameterRange(const QString & ringThickness) {
	double rt = TextUtils::convertToInches(ringThickness);
	QPointF p(rt > 0 ? 0 : .001, 10.0);
	return p;
}


void Hole::changeHoleSize(const QString & newSize) {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
        QRectF holeRect = getRect(holeSize());
        QRectF newHoleRect = getRect(newSize);
        infoGraphicsView->setHoleSize(this, "hole size", tr("hole size"), holeSize(), newSize, holeRect, newHoleRect, true);
	}
}

bool Hole::canEditPart() {
	return false;
}

bool Hole::hasPartNumberProperty()
{
	return false;
}

bool Hole::rotationAllowed() {
	return false;
}

bool Hole::rotation45Allowed() {
	return false;
}

bool Hole::canFindConnectorsUnder() {
	return false;
}

QString Hole::currentUnits() {
	return m_holeSettings.currentUnits();
}

ViewIdentifierClass::ViewIdentifier Hole::useViewIdentifierForPixmap(ViewIdentifierClass::ViewIdentifier vid, bool) 
{
    if (vid == ViewIdentifierClass::PCBView) {
        return ViewIdentifierClass::IconView;
    }

    return ViewIdentifierClass::UnknownView;
}
