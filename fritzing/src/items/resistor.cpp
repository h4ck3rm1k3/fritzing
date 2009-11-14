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

#include "resistor.h"
#include "../utils/graphicsutils.h"
#include "../utils/focusoutcombobox.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"
#include "../layerattributes.h"

#include <qmath.h>
#include <QRegExpValidator>
#include <limits>

static QString BreadboardLayerTemplate = "";
static QList<QString> Resistances;
static QList<QString> PinSpacings;
static QHash<int, QColor> ColorBands;
static QChar OhmSymbol(0x03A9);
static QRegExp Digits("(\\d)+");
static QRegExp DigitsMil("(\\d)+mil");

class BoundedRegExpValidator : public QRegExpValidator 
{
public:
	BoundedRegExpValidator(QObject * parent) : QRegExpValidator(parent) {
		m_max = std::numeric_limits<double>::max();
		m_min = std::numeric_limits<double>::min();
	}

	void setBounds(qreal min, qreal max) {
		m_min = min;
		m_max = max;
	}

	QValidator::State validate ( QString & input, int & pos ) const {
		QValidator::State state = QRegExpValidator::validate(input, pos);
		if (state == QValidator::Invalid) return state;
		if (state == QValidator::Intermediate) return state;

		QString r = input;
		qreal ohms = Resistor::toOhms(r);
		if (ohms < m_min) return QValidator::Invalid;
		if (ohms > m_max) return QValidator::Invalid;

		return QValidator::Acceptable;
	}

protected:
	qreal m_min;
	qreal m_max;

};


// TODO
//	save into parts bin
//	other manifestations of "220"?

Resistor::Resistor( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel, LayerHash & layers)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_changingPinSpacing = false;
	m_layers = layers;
	if (Resistances.count() == 0) {
		Resistances 
		 << QString("1") + OhmSymbol << QString("1.5") + OhmSymbol << QString("2.2") + OhmSymbol << QString("3.3") + OhmSymbol << QString("4.7") + OhmSymbol << QString("6.8") + OhmSymbol
		 << QString("10") + OhmSymbol << QString("15") + OhmSymbol << QString("22") + OhmSymbol << QString("33") + OhmSymbol << QString("47") + OhmSymbol << QString("68") + OhmSymbol
		 << QString("100") + OhmSymbol << QString("150") + OhmSymbol << QString("220") + OhmSymbol << QString("330") + OhmSymbol << QString("470") + OhmSymbol << QString("680") + OhmSymbol
		 << QString("1k") + OhmSymbol << QString("1.5k") + OhmSymbol << QString("2.2k") + OhmSymbol << QString("3.3k") + OhmSymbol << QString("4.7k") + OhmSymbol << QString("6.8k") + OhmSymbol
		 << QString("10k") + OhmSymbol << QString("15k") + OhmSymbol << QString("22k") + OhmSymbol << QString("33k") + OhmSymbol << QString("47k") + OhmSymbol << QString("68k") + OhmSymbol
		 << QString("100k") + OhmSymbol << QString("150k") + OhmSymbol << QString("220k") + OhmSymbol << QString("330k") + OhmSymbol << QString("470k") + OhmSymbol << QString("680k") + OhmSymbol
		 << QString("1M") + OhmSymbol;
	}

	if (PinSpacings.count() == 0) {
		PinSpacings << "300 mil" << "400 mil" << "500 mil" << "600 mil" << "800 mil";
	}

	if (ColorBands.count() == 0) {
		ColorBands.insert(0, QColor(0, 0, 0));
		ColorBands.insert(1, QColor(138, 61, 6));
		ColorBands.insert(2, QColor(196, 8, 8));
		ColorBands.insert(3, QColor(255, 77, 0));
		ColorBands.insert(4, QColor(255, 213, 0));
		ColorBands.insert(5, QColor(0, 163, 61));
		ColorBands.insert(6, QColor(0, 96, 182));
		ColorBands.insert(7, QColor(130, 16, 210));
		ColorBands.insert(8, QColor(140, 140, 140));
		ColorBands.insert(9, QColor(255, 255, 255));
		ColorBands.insert(-1, QColor(173, 159, 78));
		ColorBands.insert(-2, QColor(192, 192, 192));
	}

	if (BreadboardLayerTemplate.isEmpty()) {
		QFile file(":/resources/templates/resistor_breadboardLayerTemplate.txt");
		file.open(QFile::ReadOnly);
		BreadboardLayerTemplate = file.readAll();
		file.close();
	}

	m_ohms = modelPart->prop("resistance").toString();
	if (m_ohms.isEmpty()) {
		m_ohms = modelPart->properties().value("resistance", "220");
		modelPart->setProp("resistance", m_ohms);
	}

	m_pinSpacing = modelPart->prop("pin spacing").toString();
	if (m_pinSpacing.isEmpty()) {
		m_pinSpacing = modelPart->properties().value("pin spacing", "400 mil");
		modelPart->setProp("pin spacing", m_pinSpacing);
	}

	m_renderer = NULL;

	updateResistances(m_ohms);
}

Resistor::~Resistor() {
}

void Resistor::setResistance(QString resistance, QString pinSpacing, LayerHash & layers, bool force) {
	if (resistance.endsWith(OhmSymbol)) {
		resistance.chop(1);
	}

	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
			if (force || resistance.compare(m_ohms) != 0) {
				if (m_renderer == NULL) {
					m_renderer = new FSvgRenderer(this);
				}
				QString svg = makeBreadboardSvg(resistance);
				//DebugDialog::debug(svg);
				bool result = m_renderer->fastLoad(svg.toUtf8());
				if (result) {
					setSharedRenderer(m_renderer);
				}
			}
			break;
		case ViewIdentifierClass::PCBView:
			if (force || pinSpacing.compare(m_pinSpacing) != 0) {
				// hack the dom element and call setUpImage
				FSvgRenderer::removeFromHash(this->modelPart()->moduleID(), "");
				QDomElement element = LayerAttributes::getSvgElementLayers(modelPart()->modelPartShared()->domDocument(), m_viewIdentifier);
				if (element.isNull()) break;

				QString filename = element.attribute("image");
				if (filename.isEmpty()) break;

				if (pinSpacing.indexOf(Digits) < 0) break;

				QString newSpacing = Digits.cap(0);		
				filename.replace(DigitsMil, newSpacing + "mil");
				element.setAttribute("image", filename);

				foreach (Connector * connector, modelPart()->connectors()) {
					connector->unprocess(this->viewIdentifier(), this->viewLayerID());
				}

				m_changingPinSpacing = true;
				this->setUpImage(modelPart(), this->viewIdentifier(), layers, this->viewLayerID(), true);
				m_changingPinSpacing = false;
				
				foreach (ItemBase * itemBase, m_layerKin) {
					qobject_cast<PaletteItemBase *>(itemBase)->setUpImage(modelPart(), itemBase->viewIdentifier(), layers, itemBase->viewLayerID(), true);
				}

				updateConnections();
			}
			break;
		default:
			break;
	}

	m_ohms = resistance;
	m_pinSpacing = pinSpacing;
	modelPart()->setProp("resistance", resistance);
	modelPart()->setProp("pin spacing", pinSpacing);

	updateResistances(m_ohms);
	updateTooltip();
}

QString Resistor::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	switch (viewLayerID) {
		case ViewLayer::Breadboard:
		case ViewLayer::Icon:
			break;
		default:
			return PaletteItem::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	}

	QString svg = makeBreadboardSvg(m_ohms);

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

QString Resistor::makeBreadboardSvg(const QString & resistance) {
	qreal ohms = toOhms(resistance);
	QString sohms = QString::number(ohms, 'e', 3);
	int firstband = sohms.at(0).toAscii() - '0';
	int secondband = sohms.at(2).toAscii() - '0';
	int temp = (firstband * 10) + secondband;
	int thirdband = (temp == 0) ? 0 : log10(ohms / temp);
	return BreadboardLayerTemplate
		.arg(ColorBands.value(firstband, Qt::black).name())
		.arg(ColorBands.value(secondband, Qt::black).name())
		.arg(ColorBands.value(thirdband, Qt::black).name());
}


void Resistor::collectExtraInfoValues(const QString & prop, QString & value, QStringList & extraValues, bool & ignoreValues) {
	ignoreValues = false;

	if (prop.compare("pin spacing", Qt::CaseInsensitive) == 0) {
		ignoreValues = true;
		value = m_pinSpacing;
		foreach (QString f, PinSpacings) {
			extraValues.append(f);
		}
		return;
	}
}

QString Resistor::collectExtraInfoHtml(const QString & prop, const QString & value) {
	Q_UNUSED(value);

	if (prop.compare("resistance", Qt::CaseInsensitive) == 0) {
		return "<object type='application/x-qt-plugin' classid='ResistanceInput' width='65px' height='22px'></object>";  
	}

	return ___emptyString___;
}

QString Resistor::getProperty(const QString & key) {
	if (key.compare("resistance", Qt::CaseInsensitive) == 0) {
		return m_ohms + OhmSymbol;
	}

	if (key.compare("pin spacing", Qt::CaseInsensitive) == 0) {
		return m_pinSpacing;
	}

	return PaletteItem::getProperty(key);
}

QString Resistor::resistance() {
	return m_ohms;
}

QString Resistor::pinSpacing() {
	return m_pinSpacing;
}

qreal Resistor::toOhms(const QString & ohms) 
{
	qreal multiplier = 1;
	QString temp = ohms;
	if (temp.endsWith(OhmSymbol)) {
		temp.chop(1);
	}

	if (temp.endsWith("k", Qt::CaseInsensitive)) {
		multiplier = 1000;
		temp.chop(1);
	}
	else if (temp.endsWith("M", Qt::CaseInsensitive)) {
		multiplier = 1000000;
		temp.chop(1);
	}
	else if (temp.endsWith("G", Qt::CaseInsensitive)) {
		multiplier = 1000000000;
		temp.chop(1);
	}
	temp = temp.trimmed();
	return temp.toDouble() * multiplier;
}

QVariant Resistor::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSceneHasChanged:
			if (this->scene()) {
				setResistance(m_ohms, m_pinSpacing, m_layers, true);
			}
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}

const QString & Resistor::title() {
	m_title = QString("%1%2 Resistor").arg(m_ohms).arg(OhmSymbol);
	return m_title;
}

void Resistor::updateResistances(QString r) {
	if (!Resistances.contains(r + OhmSymbol)) {
		Resistances.append(r + OhmSymbol);
	}
}

ConnectorItem* Resistor::newConnectorItem(Connector *connector) {
	if (m_changingPinSpacing) {
		return connector->connectorItem(this->scene());
	}

	return PaletteItem::newConnectorItem(connector);
}

bool Resistor::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::IconView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

bool Resistor::canEditPart() {
	return false;
}

QObject * Resistor::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {
	Q_UNUSED(url);
	Q_UNUSED(paramNames);
	Q_UNUSED(paramValues);

	if (classid.compare("ResistanceInput") != 0) return NULL;
	
	FocusOutComboBox * edit = new FocusOutComboBox(parent);
	edit->setEditable(true);
	int ix = 0;
	QString current = m_ohms + OhmSymbol;
	foreach (QString r, Resistances) {
		edit->addItem(r);
		if (r == current) {
			edit->setCurrentIndex(ix);
		}
		ix++;
	}
				
	BoundedRegExpValidator * validator = new BoundedRegExpValidator(edit);
        validator->setBounds(0, 9900000000.0);
	validator->setRegExp(QRegExp("((\\d{1,3})|(\\d{1,3}\\.)|(\\d{1,3}\\.\\d))[kMG]{0,1}[\\x03A9]{0,1}"));
	edit->setValidator(validator);

	connect(edit, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(resistanceEntry(const QString &)));

	return edit;	
}

void Resistor::resistanceEntry(const QString & text) {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setResistance(text, "");
	}
}
