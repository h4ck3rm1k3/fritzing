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

$Revision: 2829 $:
$Author: cohen@irascible.com $:
$Date: 2009-04-17 00:22:27 +0200 (Fri, 17 Apr 2009) $

********************************************************************/

#include "resistor.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"

#include <qmath.h>

static QString BreadboardLayerTemplate = "";
static QList<QString> Resistances;
static QList<QString> Footprints;
static QHash<int, QColor> ColorBands;

Resistor::Resistor( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (Resistances.count() == 0) {
		Resistances 
		 << "10" << "15" << "22" << "33" << "47" << "68"
		 << "100" << "150" << "220" << "330" << "470" << "680"
		 << "1k" << "1.5k" << "2.2k" << "3.3k" << "4.7k" << "6.8k"
		 << "10k" << "15k" << "22k" << "33k" << "47k" << "68k"
		 << "100k" << "150k" << "220k" << "330k" << "470k" << "680k"
		 << "1M";
	}

	if (Footprints.count() == 0) {
		Footprints << "300 mil" << "400 mil";
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
		m_ohms = modelPart->properties().value("Resistance", "220");
		modelPart->setProp("resistance", m_ohms);
	}
	m_footprint = modelPart->prop("footprint").toString();
	if (m_footprint.isEmpty()) {
		m_footprint = modelPart->properties().value("footprint", "400 mil");
		modelPart->setProp("footprint", m_footprint);
	}
	m_renderer = NULL;
}

Resistor::~Resistor() {
}

void Resistor::setResistance(QString resistance, QString footprint) {
	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
			{
				if (m_renderer == NULL) {
					m_renderer = new FSvgRenderer(this);
				}
				QString svg = makeBreadboardSvg(resistance);
				DebugDialog::debug(svg);
				bool result = m_renderer->fastLoad(svg.toUtf8());
				if (result) {
					setSharedRenderer(m_renderer);
				}
			}
			break;
		case ViewIdentifierClass::PCBView:
			// hack the dom element and call setUpImage?
			foreach (ItemBase * itemBase, m_layerKin) {
			}
		default:
			return;
	}

	m_ohms = resistance;
	m_footprint = footprint;
	modelPart()->setProp("resistance", resistance);
	modelPart()->setProp("footprint", footprint);
}


QString Resistor::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{

	/*
	qreal w = m_modelPart->prop("width").toDouble();
	if (w != 0) {
		qreal h = m_modelPart->prop("height").toDouble();
		QString xml;
		switch (viewLayerID) {
			case ViewLayer::Board:
				xml = makeBoardSvg(w, h, GraphicsUtils::mm2mils(w), GraphicsUtils::mm2mils(h));
				break;
			case ViewLayer::Silkscreen:
				xml = makeSilkscreenSvg(w, h, GraphicsUtils::mm2mils(w), GraphicsUtils::mm2mils(h));
				break;
			default:
				break;
		}

		if (!xml.isEmpty()) {
			QString xmlName = ViewLayer::viewLayerXmlNameFromID(viewLayerID);
			SvgFileSplitter splitter;
			bool result = splitter.splitString(xml, xmlName);
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

	*/

	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

QString Resistor::makeBreadboardSvg(const QString & resistance) {
	qreal ohms = toOhms(resistance);
	QString sohms = QString::number(ohms, 'e', 3);
	int firstband = sohms.at(0).toAscii() - '0';
	int secondband = sohms.at(2).toAscii() - '0';
	int temp = (firstband * 10) + secondband;
	int thirdband = log10(ohms / temp);
	return BreadboardLayerTemplate
		.arg(ColorBands.value(firstband, Qt::black).name())
		.arg(ColorBands.value(secondband, Qt::black).name())
		.arg(ColorBands.value(thirdband, Qt::black).name());
}


void Resistor::collectExtraInfoValues(const QString & prop, QString & value, QStringList & extraValues, bool & ignoreValues) {
	ignoreValues = false;

	if (prop.compare("resistance", Qt::CaseInsensitive) == 0) {
		ignoreValues = true;
		value = m_ohms;
		foreach (QString r, Resistances) {
			extraValues.append(r);
		}
		return;
	}
	if (prop.compare("footprint", Qt::CaseInsensitive) == 0) {
		ignoreValues = true;
		value = m_footprint;
		foreach (QString f, Footprints) {
			extraValues.append(f);
		}
		return;
	}
}

QString Resistor::collectExtraInfoHtml(const QString & prop, const QString & value) {
	Q_UNUSED(value);

	if (prop.compare("resistance", Qt::CaseInsensitive) == 0) {
		return QString("&nbsp;<input type='text' name='sResistance' id='sResistance' maxlength='8' value='%1' style='width:55px' onblur='setResistance()' onkeypress='setResistanceEnter(event)' />"
					   "<script language='JavaScript'>lastGoodResistance=%1;</script>"
					   ).arg(m_ohms);
	}

	return ___emptyString___;
}

QString Resistor::resistance() {
	return m_ohms;
}

QString Resistor::footprint() {
	return m_footprint;
}

qreal Resistor::toOhms(const QString & ohms) 
{
	qreal multiplier = 1;
	QString temp = ohms;
	if (temp.endsWith("k", Qt::CaseInsensitive)) {
		multiplier = 1000;
		temp.chop(1);
	}
	else if (ohms.endsWith("M", Qt::CaseInsensitive)) {
		multiplier = 1000000;
		temp.chop(1);
	}
	else if (ohms.endsWith("G", Qt::CaseInsensitive)) {
		multiplier = 1000000000;
		temp.chop(1);
	}
	temp = temp.trimmed();
	return temp.toDouble() * multiplier;
}
