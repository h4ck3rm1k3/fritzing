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

#include "schematicframe.h"
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

static QString SchematicTemplate = "";
static int OriginalWidth = 0;
static int OriginalHeight = 0;

SchematicFrame::SchematicFrame( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{

}

SchematicFrame::~SchematicFrame() {
}

void SchematicFrame::loadTemplates() {
	if (SchematicTemplate.isEmpty()) {
		QFile file(":/resources/templates/schematic_frame_template.txt");
		file.open(QFile::ReadOnly);
		SchematicTemplate = file.readAll();
		file.close();

		QFile file2(":/resources/parts/svg/core/schematic/frame.svg");
		file2.open(QFile::ReadOnly);
		QString original = file2.readAll();
		file2.close();

		OriginalWidth = TextUtils::getViewBoxCoord(original, 2);
		OriginalHeight = TextUtils::getViewBoxCoord(original, 3);
	}
}

QString SchematicFrame::makeLayerSvg(ViewLayer::ViewLayerID viewLayerID, qreal mmW, qreal mmH, qreal milsW, qreal milsH) 
{
	Q_UNUSED(mmW);
	Q_UNUSED(mmH);

	switch (viewLayerID) {
		case ViewLayer::SchematicFrame:
			break;
		default:
			return "";
	}

	if (milsW < OriginalWidth) milsW = OriginalWidth;
	if (milsH < OriginalHeight) milsH = OriginalHeight;
	QString svg = SchematicTemplate.arg(milsW / 1000).arg(milsH / 1000).arg(milsW).arg(milsH).arg(milsW - 8).arg(milsH - 8);
	svg = TextUtils::incrementTemplateString(svg, 1, milsW - OriginalWidth, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction);
	svg.replace("{", "[");
	svg.replace("}", "]");
	return  TextUtils::incrementTemplateString(svg, 1, milsH - OriginalHeight, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction);
}

QString SchematicFrame::makeNextLayerSvg(ViewLayer::ViewLayerID viewLayerID, qreal mmW, qreal mmH, qreal milsW, qreal milsH) 
{
	Q_UNUSED(mmW);
	Q_UNUSED(mmH);
	Q_UNUSED(milsW);
	Q_UNUSED(milsH);
	Q_UNUSED(viewLayerID);

	return "";
}

QString SchematicFrame::makeFirstLayerSvg(qreal mmW, qreal mmH, qreal milsW, qreal milsH) {
	return makeLayerSvg(ViewLayer::SchematicFrame, mmW, mmH, milsW, milsH);
}

ViewIdentifierClass::ViewIdentifier SchematicFrame::theViewIdentifier() {
	return ViewIdentifierClass::SchematicView;
}

qreal SchematicFrame::minWidth() {
	return OriginalWidth * FSvgRenderer::printerScale() / 1000;
}

qreal SchematicFrame::minHeight() {
	return OriginalHeight * FSvgRenderer::printerScale() / 1000;
}

void SchematicFrame::addedToScene()
{
    ResizableBoard::addedToScene();
	resizeMMAux(m_modelPart->prop("width").toDouble(), m_modelPart->prop("height").toDouble());
}

QString SchematicFrame::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi)
{

	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

bool SchematicFrame::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

bool SchematicFrame::hasGrips() {
	return true;
}


void SchematicFrame::setProp(const QString & prop, const QString & value) {


	ResizableBoard::setProp(prop, value);
}


QString SchematicFrame::getProperty(const QString & key) {

	return PaletteItem::getProperty(key);
}

bool SchematicFrame::canEditPart() {
	return false;
}

bool SchematicFrame::hasPartLabel() {
	return false;
}


bool SchematicFrame::stickyEnabled() {
	return false;
}

ItemBase::PluralType SchematicFrame::isPlural() {
	return Plural;
}

bool SchematicFrame::rotationAllowed() {
	return false;
}

bool SchematicFrame::rotation45Allowed() {
	return false;
}

bool SchematicFrame::hasPartNumberProperty()
{
	return false;
}

void SchematicFrame::setInitialSize() {
	qreal w = m_modelPart->prop("width").toDouble();
	if (w == 0) {
		// set the size so the infoGraphicsView will display the size as you drag
		modelPart()->setProp("width", 25.4 * OriginalWidth / 1000); 
		modelPart()->setProp("height", 25.4 * OriginalHeight / 1000); 
	}
}
