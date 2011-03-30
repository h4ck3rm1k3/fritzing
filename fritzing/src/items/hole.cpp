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

QHash<QString, QString> Hole::m_holeSizes;
QHash<QString, QString> Hole::m_holeSizeTranslations;

Hole::Hole( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_diameterEdit = m_thicknessEdit = NULL;
	m_diameterValidator = m_thicknessValidator = NULL;
	m_unitsComboBox = m_sizesComboBox = NULL;

	if (m_holeSizes.count() == 0) {
		QString name = "pin header";
		m_holeSizes.insert(name, ".038in,.02in");
		m_holeSizeTranslations.insert(tr("Pin Header"), name);

		name = "std through-hole";
		m_holeSizes.insert(name, ".035in,.02in");
		m_holeSizeTranslations.insert(tr("Standard Through-hole"), name);

		name = "big through-hole";
		m_holeSizes.insert(name, ".042in,.02in");
		m_holeSizeTranslations.insert(tr("Big Through-hole"), name);

		name = "mounting hole";
		m_holeSizes.insert(name, ".086in,.02in");
		m_holeSizeTranslations.insert(tr("Mounting Hole"), name);

		name = "fine lead";
		m_holeSizes.insert(name, ".028in,.02in");
		m_holeSizeTranslations.insert(tr("Fine Lead Parts"), name);

		name = "thick lead";
		m_holeSizes.insert(name, ".052in,.02in");
		m_holeSizeTranslations.insert(tr("Thick Lead Parts"), name);
	}

	QString holeSize = modelPart->prop("hole size").toString();
	QStringList sizes = holeSize.split(",");
	if (sizes.count() != 2) {
		holeSize = m_holeSizes.value(holeSize, "");
		if (holeSize.isEmpty()) {
			holeSize = modelPart->properties().value("hole size", ".035in,0.2in");
			modelPart->setProp("hole size", holeSize);
		}
		sizes = holeSize.split(",");
	}

	m_holeDiameter = sizes.at(0);
	m_ringThickness = sizes.at(1);

	m_otherLayerRenderer = m_renderer = NULL;
}

Hole::~Hole() {
}


void Hole::setProp(const QString & prop, const QString & value) {
	if (prop.compare("hole size", Qt::CaseInsensitive) == 0) {
		setHoleSize(value, false);
		return;
	}

	PaletteItem::setProp(prop, value);
}

QString Hole::holeSize() {
	return QString("%1,%2").arg(m_holeDiameter).arg(m_ringThickness);
}

void Hole::setHoleSize(QString holeSize, bool force) {
	QString hashedHoleSize = m_holeSizes.value(holeSize);
	QStringList sizes;
	if (hashedHoleSize.isEmpty()) {
		sizes = holeSize.split(",");
	}
	else {
		sizes = hashedHoleSize.split(",");
		holeSize = sizes[0] + "," + sizes[1];
	}
	if (sizes.count() < 2) return;

	if (!force && (m_holeDiameter.compare(sizes.at(0)) == 0) && (m_ringThickness.compare(sizes.at(1)) == 0)) 
	{
		return;
	}

	m_holeDiameter = sizes.at(0);
	m_ringThickness = sizes.at(1);
	setBoth(m_holeDiameter, m_ringThickness);
	updateEditTexts();
	updateValidators();
	updateSizes();

	modelPart()->setProp("hole size", holeSize);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTextsIf();
}

void Hole::setBoth(const QString & holeDiameter, const QString & ringThickness) {
	if (this->m_viewIdentifier != ViewIdentifierClass::PCBView) return;

	QStringList connectorIDs;
	ItemBase * otherLayer = setBothSvg(holeDiameter, ringThickness, connectorIDs);

	// there's only one NonConnectorItem
	foreach (SvgIdLayer * svgIdLayer, m_renderer->setUpNonConnectors()) {
		if (svgIdLayer == NULL) continue;

		setBothNonConnectors(this, svgIdLayer);
		if (otherLayer != NULL) {
			setBothNonConnectors(otherLayer, svgIdLayer);
		}

		delete svgIdLayer;
	}
}

ItemBase * Hole::setBothSvg(const QString & holeDiameter, const QString & ringThickness, const QStringList & connectorIDs) 
{
	QString svg = makeSvg(holeDiameter, ringThickness, m_viewLayerID);
	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	QString setColor;
	QStringList noIDs;
	bool result = m_renderer->loadSvg(svg.toLatin1(), m_filename, connectorIDs, noIDs, "", "", true);
	if (result) {
		setSharedRendererEx(m_renderer);
	}

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
		if (m_otherLayerRenderer == NULL) {
			m_otherLayerRenderer = new FSvgRenderer(this);
		}

		result = m_otherLayerRenderer->loadSvg(osvg.toLatin1(), m_filename, noIDs, noIDs, "", "", true);
		if (result) {
			qobject_cast<PaletteItemBase *>(otherLayer)->setSharedRendererEx(m_otherLayerRenderer);
		}
	}
	
	return otherLayer;
}

void Hole::setBothNonConnectors(ItemBase * itemBase, SvgIdLayer * svgIdLayer) {
	foreach (QGraphicsItem * child, itemBase->childItems()) {
		NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(child);
		if (nonConnectorItem == NULL) continue;

		nonConnectorItem->setRect(svgIdLayer->m_rect);
		nonConnectorItem->setRadius(svgIdLayer->m_radius, svgIdLayer->m_strokeWidth);
		break;
	}
}


QString Hole::makeSvg(const QString & holeDiameter, const QString & ringThickness, ViewLayer::ViewLayerID viewLayerID) 
{
	qreal hd = TextUtils::convertToInches(holeDiameter) * GraphicsUtils::StandardFritzingDPI;
	qreal rt = TextUtils::convertToInches(ringThickness) * GraphicsUtils::StandardFritzingDPI;

	qreal wInches = (hd + rt + rt) / GraphicsUtils::StandardFritzingDPI;
	QString svg = TextUtils::makeSVGHeader(1, GraphicsUtils::StandardFritzingDPI, wInches, wInches);

	QString setColor;
	if (viewLayerID == ViewLayer::Copper0) {
		setColor = ViewLayer::Copper0Color;
	}
	else if (viewLayerID == ViewLayer::Copper1) {
		setColor = ViewLayer::Copper1Color;
	}

	svg += QString("<g id='%1'>").arg(ViewLayer::viewLayerXmlNameFromID(viewLayerID));
	
	QString id = makeID();
	if (rt == 0) {
		svg += QString("<circle cx='%1' cy='%1' r='%1' fill='black' id='%2' />")
					.arg(hd / 2)
					.arg(id);
	}
	else if (hd == 0) {
		svg += QString("<circle cx='%1' cy='%1' r='%1' fill='%2' id='%3' />")
					.arg(rt)
					.arg(setColor)
					.arg(id);
	}
	else {
		svg += QString("<circle fill='none' cx='%1' cy='%1' r='%2' stroke-width='%3' stroke='%4' id='%5' />")
			.arg((hd / 2) + rt)
			.arg((hd / 2) + (rt / 2))
			.arg(rt)
			.arg(setColor)
			.arg(id);
		svg += QString("<circle drill='0' fill='black' cx='%1' cy='%1' r='%2' stroke-width='0'  />")   // set the drill attribute for gerber translation
			.arg((hd / 2) + rt)
			.arg(hd / 2);
	}
  		
	svg += "</g></svg>";
	return svg;
}

QString Hole::makeID() {
	return FSvgRenderer::NonConnectorName + "0";
}

QString Hole::getProperty(const QString & key) {
	if (key.compare("hole size", Qt::CaseInsensitive) == 0) {
		return m_holeDiameter;
	}

	return PaletteItem::getProperty(key);
}

void Hole::addedToScene()
{
	if (this->scene()) {
		setHoleSize(QString("%1,%2").arg(m_holeDiameter).arg(m_ringThickness), true);
	}

    return PaletteItem::addedToScene();
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

QString Hole::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi) 
{
	if (m_viewIdentifier != ViewIdentifierClass::PCBView || 
		(viewLayerID != ViewLayer::Copper0 && viewLayerID != ViewLayer::Copper1)) 
	{
		return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	}

	QStringList holeSize = m_modelPart->prop("hole size").toString().split(",");
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
	static const int rowHeight = 21;

	if (prop.compare("hole size", Qt::CaseInsensitive) == 0) {
		returnProp = tr("hole size");

		QFrame * frame = new QFrame(parent);
		frame->setObjectName("infoViewPartFrame");

		QVBoxLayout * vBoxLayout = new QVBoxLayout(frame);
		vBoxLayout->setMargin(0);
		vBoxLayout->setContentsMargins(0, 0, 0, 0);
		vBoxLayout->setSpacing(0);

		m_sizesComboBox = new QComboBox(frame);
		m_sizesComboBox->setMaximumWidth(200);
		m_sizesComboBox->setEditable(false);
		m_sizesComboBox->setObjectName("infoViewComboBox");

		vBoxLayout->addWidget(m_sizesComboBox);

        QFrame * hFrame = new QFrame(frame);
        QHBoxLayout * hLayout = new QHBoxLayout(hFrame);
		hLayout->setMargin(0);

		QGroupBox * subFrame = new QGroupBox(tr("custom settings"), frame);
		subFrame->setObjectName("infoViewGroupBox");

		QGridLayout * gridLayout = new QGridLayout(subFrame);
		gridLayout->setMargin(0);

		m_unitsComboBox = new QComboBox(subFrame);
        m_unitsComboBox->setMaximumWidth(60);
		m_unitsComboBox->setMinimumHeight(rowHeight);
		m_unitsComboBox->setMaximumHeight(rowHeight);
		m_unitsComboBox->setEditable(false);
		m_unitsComboBox->addItem("mm");
		m_unitsComboBox->addItem("in");
		gridLayout->addWidget(m_unitsComboBox, 0, 2, 2, 1);
		m_unitsComboBox->setObjectName("infoViewComboBox");

		m_diameterEdit = new QLineEdit(subFrame);
		m_diameterEdit->setMaximumWidth(50);
		m_diameterEdit->setMinimumHeight(rowHeight);
		m_diameterValidator = new QDoubleValidator(m_diameterEdit);
		m_diameterValidator->setNotation(QDoubleValidator::StandardNotation);
		m_diameterEdit->setValidator(m_diameterValidator);
		gridLayout->addWidget(m_diameterEdit, 0, 1);
		m_diameterEdit->setObjectName("infoViewLineEdit");

		QLabel * label = new QLabel(tr("Hole Diameter"));
		label->setMinimumHeight(rowHeight);
		label->setObjectName("infoViewLabel");
		gridLayout->addWidget(label, 0, 0);

		m_thicknessEdit = new QLineEdit(subFrame);
		m_thicknessEdit->setMaximumWidth(50);
		m_thicknessEdit->setMinimumHeight(rowHeight);
		m_thicknessValidator = new QDoubleValidator(m_thicknessEdit);
		m_thicknessValidator->setNotation(QDoubleValidator::StandardNotation);
		m_thicknessEdit->setValidator(m_thicknessValidator);
		gridLayout->addWidget(m_thicknessEdit, 1, 1);
		m_thicknessEdit->setObjectName("infoViewLineEdit");

		label = new QLabel(tr("Ring Thickness"));
		label->setMinimumHeight(rowHeight);
		gridLayout->addWidget(label, 1, 0);
		label->setObjectName("infoViewLabel");

		gridLayout->setContentsMargins(10, 2, 0, 2);
		gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 3);
		gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 3);

        hLayout->addWidget(subFrame);
        hLayout->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
        vBoxLayout->addWidget(hFrame);

		m_sizesComboBox->addItems(m_holeSizes.keys());
		m_sizesComboBox->setEnabled(swappingEnabled);
		m_unitsComboBox->setEnabled(swappingEnabled);
		m_unitsComboBox->setCurrentIndex(m_modelPart->prop("hole size").toString().contains("in") ? 1 : 0);
		m_diameterEdit->setEnabled(swappingEnabled);
		m_thicknessEdit->setEnabled(swappingEnabled);

		updateEditTexts();
		updateValidators();
		updateSizes();

		connect(m_sizesComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeHoleSize(const QString &)));
		connect(m_unitsComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeUnits(const QString &)));
		connect(m_diameterEdit, SIGNAL(editingFinished()), this, SLOT(changeDiameter()));
		connect(m_thicknessEdit, SIGNAL(editingFinished()), this, SLOT(changeThickness()));

		returnWidget = frame;
		return true;
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void Hole::changeThickness() 
{
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	double newValue = edit->text().toDouble();
	QString temp = m_ringThickness;
	temp.chop(2);
	double oldValue = temp.toDouble();
	if (newValue == oldValue) return;

	changeHoleSize(m_holeDiameter + "," + edit->text() + m_unitsComboBox->currentText());
}

void Hole::changeDiameter() 
{
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;


	double newValue = edit->text().toDouble();
	QString temp = m_holeDiameter;
	temp.chop(2);
	double oldValue = temp.toDouble();
	if (newValue == oldValue) return;

	changeHoleSize(edit->text() + m_unitsComboBox->currentText() + "," + m_ringThickness);
}

void Hole::changeUnits(const QString & units) 
{
	qreal hd = TextUtils::convertToInches(m_holeDiameter);
	qreal rt = TextUtils::convertToInches(m_ringThickness);
	QString newVal;
	if (units == "in") {
		newVal = QString("%1in,%2in").arg(hd).arg(rt);
	}
	else {
		newVal = QString("%1mm,%2mm").arg(hd * 25.4).arg(rt * 25.4);
	}

	QStringList sizes = newVal.split(",");
	m_ringThickness = sizes.at(1);
	m_holeDiameter = sizes.at(0);
	modelPart()->setProp("hole size", newVal);

	updateValidators();
	updateSizes();
	updateEditTexts();
}

void Hole::updateValidators()
{
	if (m_diameterValidator == NULL) return;
	if (m_thicknessValidator == NULL) return;
	if (m_unitsComboBox == NULL) return;

	QString units = m_unitsComboBox->currentText();
	QPointF hdRange = holeDiameterRange();
	QPointF rtRange = ringThicknessRange();

	qreal multiplier = (units == "mm") ? 25.4 : 1.0;
	m_diameterValidator->setRange(hdRange.x() * multiplier, hdRange.y() * multiplier, 3);
	m_thicknessValidator->setRange(rtRange.x() * multiplier, rtRange.y() * multiplier, 3);
}


QPointF Hole::ringThicknessRange() {
	qreal hd = TextUtils::convertToInches(m_holeDiameter);
	QPointF p(hd > 0 ? 0 : .001, 1.0);
	return p;
}

QPointF Hole::holeDiameterRange() {
	qreal rt = TextUtils::convertToInches(m_ringThickness);
	QPointF p(rt > 0 ? 0 : .001, 1.0);
	return p;
}


void Hole::changeHoleSize(const QString & newSize) {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setProp(this, "hole size", tr("hole size"), this->holeSize(), newSize, true);
	}
}

void Hole::updateEditTexts() {
	if (m_diameterEdit == NULL) return;
	if (m_thicknessEdit == NULL) return;
	if (m_unitsComboBox == NULL) return;

	qreal hd = TextUtils::convertToInches(m_holeDiameter);
	qreal rt = TextUtils::convertToInches(m_ringThickness);

	QString newVal;
	if (m_unitsComboBox->currentText() == "in") {
		newVal = QString("%1,%2").arg(hd).arg(rt);
	}
	else {
		newVal = QString("%1,%2").arg(hd * 25.4).arg(rt * 25.4);
	}

	QStringList sizes = newVal.split(",");
	m_diameterEdit->setText(sizes.at(0));
	m_thicknessEdit->setText(sizes.at(1));
}

void Hole::updateSizes() {
	if (m_sizesComboBox == NULL) return;

	int newIndex = -1;

	QPointF current(TextUtils::convertToInches(m_holeDiameter), TextUtils::convertToInches(m_ringThickness));
	for (int ix = 0; ix < m_sizesComboBox->count(); ix++) {
		QString key = m_sizesComboBox->itemText(ix);
		QString value = m_holeSizes.value(key, "");
		QStringList sizes;
		if (value.isEmpty()) {
			sizes = key.split(",");
		}
		else {
			sizes = value.split(",");
		}
		if (sizes.count() < 2) continue;

		QPointF p(TextUtils::convertToInches(sizes.at(0)), TextUtils::convertToInches(sizes.at(1)));
		if (p == current) {
			newIndex = ix;
			break;
		}
	}

	if (newIndex < 0) {
		QString newItem = m_holeDiameter + "," + m_ringThickness;
		m_sizesComboBox->addItem(newItem);
		newIndex = m_sizesComboBox->findText(newItem);
	}

	// don't want to trigger another undo command
	disconnect(m_sizesComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeHoleSize(const QString &)));
	m_sizesComboBox->setCurrentIndex(newIndex);
	connect(m_sizesComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeHoleSize(const QString &)));
}

bool Hole::canEditPart() {
	return false;
}

bool Hole::hasPartNumberProperty()
{
	return false;
}
