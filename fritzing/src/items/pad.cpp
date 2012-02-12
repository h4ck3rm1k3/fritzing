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

$Revision: 5836 $:
$Author: cohen@irascible.com $:
$Date: 2012-02-04 11:42:07 +0100 (Sat, 04 Feb 2012) $

********************************************************************/

// TODO:
//		choice of terminalpoint
//		choice of layer
//		border around outside

#include "pad.h"

#include "../utils/graphicsutils.h"
#include "../utils/folderutils.h"
#include "../utils/textutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "moduleidnames.h"


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QRegExp>
#include <QPushButton>
#include <QImageReader>
#include <QMessageBox>
#include <QImage>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QHash>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>

static QString PadTemplate = "";
static double OriginalWidth = 0.1;
static double OriginalHeight = 0.1;

Pad::Pad( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
}

Pad::~Pad() {

}

QString Pad::makeLayerSvg(ViewLayer::ViewLayerID viewLayerID, double mmW, double mmH, double milsW, double milsH) 
{
	switch (viewLayerID) {
		case ViewLayer::Copper0:
		case ViewLayer::Copper1:
			break;
		default:
			return "";
	}

	if (milsW < OriginalWidth) milsW = OriginalWidth;
	if (milsH < OriginalHeight) milsH = OriginalHeight;
	//QString svg = SchematicTemplate.arg(milsW / 1000).arg(milsH / 1000).arg(milsW).arg(milsH).arg(milsW - 8).arg(milsH - 8);
	//svg = TextUtils::incrementTemplateString(svg, 1, milsW - OriginalWidth, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction, NULL);
	//svg.replace("{", "[");
	//svg.replace("}", "]");
	//svg = TextUtils::incrementTemplateString(svg, 1, milsH - OriginalHeight, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction, NULL);



	//return TextUtils::convertExtendedChars(TextUtils::replaceTextElements(svg, hash));

	return "";
}

QString Pad::makeNextLayerSvg(ViewLayer::ViewLayerID viewLayerID, double mmW, double mmH, double milsW, double milsH) 
{
	Q_UNUSED(mmW);
	Q_UNUSED(mmH);
	Q_UNUSED(milsW);
	Q_UNUSED(milsH);
	Q_UNUSED(viewLayerID);

	return "";
}

QString Pad::makeFirstLayerSvg(double mmW, double mmH, double milsW, double milsH) {
	return makeLayerSvg(ViewLayer::Copper0, mmW, mmH, milsW, milsH);
}

ViewIdentifierClass::ViewIdentifier Pad::theViewIdentifier() {
	return ViewIdentifierClass::PCBView;
}

double Pad::minWidth() {
	return OriginalWidth * FSvgRenderer::printerScale() / GraphicsUtils::StandardFritzingDPI;
}

double Pad::minHeight() {
	return OriginalHeight * FSvgRenderer::printerScale() / GraphicsUtils::StandardFritzingDPI;
}

void Pad::addedToScene(bool temporary)
{
	if (prop("filename").isEmpty()) {
		InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
		if (infoGraphicsView != NULL) {
			modelPart()->setProp("filename", infoGraphicsView->filenameIf());
		}
	}
    ResizableBoard::addedToScene(temporary);
	resizeMMAux(m_modelPart->prop("width").toDouble(), m_modelPart->prop("height").toDouble());
}

QString Pad::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, double dpi)
{
	return ResizableBoard::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

bool Pad::collectExtraInfo(QWidget * parent, const QString & family, const QString & propp, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{

	return PaletteItem::collectExtraInfo(parent, family, propp, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void Pad::setProp(const QString & prop, const QString & value) 
{	
	ResizableBoard::setProp(prop, value);

}

bool Pad::canEditPart() {
	return false;
}

bool Pad::hasPartLabel() {
	return true;
}

bool Pad::stickyEnabled() {
	return false;
}

ItemBase::PluralType Pad::isPlural() {
	return Plural;
}

bool Pad::rotationAllowed() {
	return true;
}

bool Pad::rotation45Allowed() {
	return true;
}

bool Pad::hasPartNumberProperty()
{
	return false;
}

void Pad::setInitialSize() {
	double w = m_modelPart->prop("width").toDouble();
	if (w == 0) {
		// set the size so the infoGraphicsView will display the size as you drag
		modelPart()->setProp("width", 25.4 * OriginalWidth / GraphicsUtils::StandardFritzingDPI); 
		modelPart()->setProp("height", 25.4 * OriginalHeight / GraphicsUtils::StandardFritzingDPI); 
	}
}


