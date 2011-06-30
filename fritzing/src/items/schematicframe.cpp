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

// TODO:
//
//	direct editing (eventually)

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
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QHash>

static QString SchematicTemplate = "";
static int OriginalWidth = 0;
static int OriginalHeight = 0;
QHash<QString, QString> FrameProps;
static const QString DisplayFormat("dd MMM yyyy hh:mm:ss");

SchematicFrame::SchematicFrame( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (FrameProps.count() == 0) {
		FrameProps.insert("descr 1", "");
		FrameProps.insert("descr 2", "");
		FrameProps.insert("title", tr("TITLE: "));
		FrameProps.insert("doc#", tr("Document Number: "));
		FrameProps.insert("date", tr("Date: "));
		FrameProps.insert("sheets", tr("Sheet:"));
		FrameProps.insert("rev", tr(""));
	}

	foreach (QString prop, FrameProps.keys()) {
		if (modelPart->prop(prop).toString().isEmpty()) {
			modelPart->setProp(prop, modelPart->properties().value(prop));
		}
	}

	if (modelPart->prop("date").toString().isEmpty()) {
		QDateTime dt = QDateTime::currentDateTime();
		modelPart->setProp("date", QString::number(dt.toTime_t()));
	}
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
	svg = TextUtils::incrementTemplateString(svg, 1, milsH - OriginalHeight, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction);
	QHash<QString, QString> hash;
	foreach (QString prop, FrameProps.keys()) {
		hash.insert(prop, FrameProps.value(prop) + modelPart()->prop(prop).toString());
	}
	hash.insert("rev label", tr("REV:"));
	QDateTime dt;
	dt.setTime_t(modelPart()->prop("date").toUInt());
	hash.insert("date", FrameProps.value("date") + dt.toString(DisplayFormat));

	return TextUtils::replaceTextElements(svg, hash);
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
	return ResizableBoard::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

bool SchematicFrame::makeLineEdit(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
	Q_UNUSED(value);
	Q_UNUSED(family);


	returnProp = ItemBase::TranslatedPropertyNames.value(prop.toLower());
	returnValue = modelPart()->prop(prop).toString();

	QLineEdit * e1 = new QLineEdit(parent);
	e1->setObjectName("infoViewLineEdit");

	e1->setProperty("prop", QVariant(prop));

	e1->setText(returnValue);
	e1->setEnabled(swappingEnabled);
	connect(e1, SIGNAL(editingFinished()), this, SLOT(propEntry()));

	returnWidget = e1;
	return true;
}

bool SchematicFrame::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
	if (prop.compare("date", Qt::CaseInsensitive) == 0) {
		QDateTimeEdit * dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), parent);
		QString d = modelPart()->prop("date").toString();
		if (!d.isEmpty()) {
			QDateTime dateTime;
			dateTime.setTime_t(d.toUInt());
			dateTimeEdit->setDateTime(dateTime);
		}
		//dateTimeEdit->setCalendarPopup(true);
		dateTimeEdit->setDisplayFormat(DisplayFormat);
		connect(dateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(dateTimeEntry(QDateTime)));
		dateTimeEdit->setObjectName("infoViewDateEdit");
		dateTimeEdit->setEnabled(swappingEnabled);

		returnProp = ItemBase::TranslatedPropertyNames.value(prop.toLower());
		returnValue = modelPart()->prop(prop).toString();
		returnWidget = dateTimeEdit;

		return true;
	}

	if (prop.compare("sheets", Qt::CaseInsensitive) == 0) {
		QFrame * frame = new QFrame(parent);
		QSpinBox * spin1 = new QSpinBox(frame);
		spin1->setRange(1, 999);
		spin1->setSingleStep(1);
		connect(spin1, SIGNAL(valueChanged(int)), this, SLOT(sheetsEntry(int)));
		spin1->setObjectName("infoViewSpinner");
		spin1->setProperty("role", "numerator");
		spin1->setEnabled(swappingEnabled);

		QSpinBox * spin2 = new QSpinBox(frame);
		spin2->setRange(1, 999);
		spin2->setSingleStep(1);
		connect(spin2, SIGNAL(valueChanged(int)), this, SLOT(sheetsEntry(int)));
		spin2->setObjectName("infoViewSpinner");
		spin2->setProperty("role", "denominator");
		spin2->setEnabled(swappingEnabled);

		QLabel * label = new QLabel(parent);
		label->setText(tr("of"));
		label->setObjectName("infoViewLabel");
		label->setAlignment(Qt::AlignCenter);

		QHBoxLayout * hBoxLayout = new QHBoxLayout(frame);
		hBoxLayout->addWidget(spin1);
		hBoxLayout->addWidget(label);
		hBoxLayout->addWidget(spin2);

		frame->setLayout(hBoxLayout);

		returnProp = ItemBase::TranslatedPropertyNames.value(prop.toLower());
		returnValue = modelPart()->prop(prop).toString();
		returnWidget = frame;

		return true;
	}

	if (FrameProps.keys().contains(prop)) {
		return makeLineEdit(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

bool SchematicFrame::hasGrips() {
	return true;
}


void SchematicFrame::setProp(const QString & prop, const QString & value) 
{	
	ResizableBoard::setProp(prop, value);

	if (FrameProps.keys().contains(prop)) {
		modelPart()->setProp(prop, value);
		resizeMMAux(modelPart()->prop("width").toDouble(), modelPart()->prop("height").toDouble());
	}

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

void SchematicFrame::propEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	QString prop = edit->property("prop").toString();
	if (prop.isEmpty()) return;

	QString current = modelPart()->prop(prop).toString();

	if (edit->text().compare(current) == 0) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setProp(this, prop, ItemBase::TranslatedPropertyNames.value(prop), current, edit->text(), true);
	}
}


void SchematicFrame::dateTimeEntry(QDateTime dateTime) {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setProp(this, "date", tr("date"), modelPart()->prop("date").toString(), QString::number(dateTime.toTime_t()), true);
	}
}

void SchematicFrame::sheetsEntry(int value) {
	QString role = sender()->property("role").toString();
	QString sheets = modelPart()->prop("sheets").toString();
	QStringList strings = sheets.split("/");
	if (strings.count() != 2) {
		strings.clear();
		strings << "1" << "1";
	}
	if (role.compare("numerator") == 0) {
		strings[0] = QString::number(value);
	}
	else if (role.compare("denominator") == 0) {
		strings[1] = QString::number(value);
	}
	else return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setProp(this, "sheets", tr("sheets"), modelPart()->prop("sheets").toString(), strings.at(0) + "/" + strings[1], true);
	}
}
