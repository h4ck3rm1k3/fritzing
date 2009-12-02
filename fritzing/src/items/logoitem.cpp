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

#include "logoitem.h"
#include "../utils/graphicsutils.h"
#include "../utils/folderutils.h"
#include "../utils/textutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"
#include "moduleidnames.h"
#include "../svg/groundplanegenerator.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>
#include <QCheckBox>
#include <QPushButton>
#include <QImageReader>
#include <QMessageBox>
#include <QImage>

static const int LineThickness = 4;
static const QRegExp WidthExpr("width=\\'\\d*px");
static const QRegExp HeightExpr("height=\\'\\d*px");

LogoItem::LogoItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_logo = modelPart->prop("logo").toString();
	if (m_logo.isEmpty()) {
		m_logo = modelPart->properties().value("logo", "logo");
		modelPart->setProp("logo", m_logo);
	}

}

LogoItem::~LogoItem() {
}


QVariant LogoItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSceneHasChanged:
			if (this->scene()) {
				setInitialSize();
				m_aspectRatio.setWidth(this->boundingRect().width());
				m_aspectRatio.setHeight(this->boundingRect().height());
				QString path = filename();
				QFile f(path);
				if (f.open(QFile::ReadOnly)) {
					QString svg = f.readAll();
					modelPart()->setProp("shape", svg);
				}
			}
			break;
		default:
			break;
   	}

    return ResizableBoard::itemChange(change, value);
}


QString LogoItem::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	if (viewLayerID == ViewLayer::Silkscreen) {
		QString svg = modelPart()->prop("shape").toString();
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

QStringList LogoItem::collectValues(const QString & family, const QString & prop, QString & value) {
	return PaletteItem::collectValues(family, prop, value);
}

bool LogoItem::collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue) 
{
	if (prop.compare("shape", Qt::CaseInsensitive) == 0) {
		returnValue = QString("<object type='application/x-qt-plugin' classid='shape' width='100%' height='50px'></object>");
		returnProp = tr("size");
		return true;
	}

	if (prop.compare("logo", Qt::CaseInsensitive) == 0) {
		returnProp = tr("logo");
		returnValue = "<object type='application/x-qt-plugin' classid='logo' width='100%' height='22px'></object>";  
		return true;
	}


	if (prop.compare("filename", Qt::CaseInsensitive) == 0) {
		returnValue = QString("<object type='application/x-qt-plugin' classid='filename' width='100%' height='22px'></object>");
		returnProp = "";
		return true;
	}

	return PaletteItem::collectExtraInfoHtml(family, prop, value, collectValues, returnProp, returnValue);

}

QObject * LogoItem::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {

	if (classid.compare("shape", Qt::CaseInsensitive) == 0) {
	}
	else if (classid.compare("filename", Qt::CaseInsensitive) == 0) { 
		QPushButton * button = new QPushButton (tr("load image file"), parent);
		connect(button, SIGNAL(pressed()), this, SLOT(prepLoadImage()));
		return button;
	}
	else if (classid.compare("logo", Qt::CaseInsensitive) == 0) {
		QLineEdit * e1 = new QLineEdit(parent);
		e1->setText(m_logo);
		connect(e1, SIGNAL(editingFinished()), this, SLOT(logoEntry()));

		return e1;
	}
	else {
		return PaletteItem::createPlugin(parent, classid, url, paramNames, paramValues);;
	}

	qreal w = qRound(m_modelPart->prop("width").toDouble() * 10) / 10.0;	// truncate to 1 decimal point
	qreal h = qRound(m_modelPart->prop("height").toDouble() * 10) / 10.0;  // truncate to 1 decimal point

	QFrame * frame = new QFrame();
	QVBoxLayout * vboxLayout = new QVBoxLayout();
	vboxLayout->setAlignment(Qt::AlignLeft);
	vboxLayout->setSpacing(0);
	vboxLayout->setContentsMargins(0, 3, 0, 0);

	QFrame * subframe = new QFrame();
	QHBoxLayout * hboxLayout = new QHBoxLayout();
	hboxLayout->setAlignment(Qt::AlignLeft);
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	hboxLayout->setSpacing(0);

	QLabel * l1 = new QLabel(tr("width(mm):"));	
	l1->setMargin(0);
	QLineEdit * e1 = new QLineEdit();
	QDoubleValidator * validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, 1);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e1->setValidator(validator);
	e1->setMaxLength(5);
	e1->setText(QString::number(w));
	m_widthEditor = e1;

	QLabel * l2 = new QLabel(tr("height(mm):"));
	l2->setMargin(0);
	QLineEdit * e2 = new QLineEdit();
	validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, 1);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e2->setValidator(validator);
	e2->setMaxLength(5);
	e2->setText(QString::number(h));
	m_heightEditor = e2;

	QCheckBox * checkBox = new QCheckBox();
	checkBox->setChecked(true);							// TODO: remember the state next time

	hboxLayout->addWidget(l1);
	hboxLayout->addWidget(e1);
	hboxLayout->addSpacing(8);
	hboxLayout->addWidget(l2);
	hboxLayout->addWidget(e2);
	hboxLayout->addSpacing(8);
	hboxLayout->addWidget(checkBox);

	subframe->setLayout(hboxLayout);

	vboxLayout->addWidget(subframe);

	frame->setLayout(vboxLayout);

	connect(e1, SIGNAL(editingFinished()), this, SLOT(widthEntry()));
	connect(e2, SIGNAL(editingFinished()), this, SLOT(heightEntry()));

	// TODO:  connect checkbox to something useful

	return frame;

}

bool LogoItem::hasGrips() {
	return true;
}

void LogoItem::prepLoadImage() {
	QList<QByteArray> supportedImageFormats = QImageReader::supportedImageFormats();
	QString imagesStr = tr("Images");
	imagesStr += " (";
	foreach (QByteArray ba, supportedImageFormats) {
		imagesStr += "*." + QString(ba) + " ";
	}
	if (!imagesStr.contains("svg")) {
		imagesStr += "*.svg";
	}
	imagesStr += ")";
	QString fileName = FolderUtils::getOpenFileName(
		NULL,
		tr("Select an image file to load"),
		"",
		imagesStr
	);

	if (fileName.isEmpty()) return;

	if (fileName.endsWith(".svg")) {
		return;
	}

	QImage image(fileName);
	if (image.isNull()) {
		QMessageBox::information(
			NULL,
			tr("Unable to load"),
			tr("Unable to load image from %1").arg(fileName)
		);
		return;
	}

	if (image.format() != QImage::Format_RGB32) {
		image = image.convertToFormat(QImage::Format_RGB32);
	}


	GroundPlaneGenerator gpg;
	qreal res = image.dotsPerMeterX() / GraphicsUtils::InchesPerMeter;
	gpg.scanImage(image, image.width(), image.height(), 1, res, "#ffffff");
	QStringList newSvgs = gpg.newSVGs();
	if (newSvgs.count() < 1) {
		QMessageBox::information(
			NULL,
			tr("Unable to display"),
			tr("Unable to display image from %1").arg(fileName)
		);
		return;
	}

	QString svg = newSvgs[0];
	for (int i = 1; i < newSvgs.length(); i++) {
		svg = TextUtils::mergeSvg(svg, newSvgs[i]);
	}

	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	bool result = m_renderer->fastLoad(svg.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setProp("shape", svg);
		QRectF r = m_renderer->viewBoxF();
		m_aspectRatio.setWidth(r.width());
		m_aspectRatio.setHeight(r.height());
		positionGrips();
	}
}

void LogoItem::resizeMM(qreal mmW, qreal mmH, const LayerHash & viewLayers) {
	Q_UNUSED(viewLayers);

	if (mmW == 0 || mmH == 0) {
		return;
	}

	QRectF r = this->boundingRect();
	if (qAbs(GraphicsUtils::pixels2mm(r.width()) - mmW) < .001 &&
		qAbs(GraphicsUtils::pixels2mm(r.height()) - mmH) < .001) 
	{
		positionGrips();
		return;
	}

	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	qreal inW = GraphicsUtils::mm2mils(mmW) / 1000;
	qreal inH = GraphicsUtils::mm2mils(mmH) / 1000;

	// TODO: deal with aspect ratio

	QString svg = modelPart()->prop("shape").toString();
	if (svg.isEmpty()) return;

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(svg, &errorStr, &errorLine, &errorColumn)) {
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		return;
	}

	if (root.tagName() != "svg") {
		return;
	}

	root.setAttribute("width", QString::number(inW) + "in");
	root.setAttribute("height", QString::number(inH) + "in");

	svg = domDocument.toString(-1);					// (-1) remove whitespace (does't really help)
	svg = svg.replace("&#xa;", "");					// there will probably be other shit....

	bool result = m_renderer->fastLoad(svg.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setProp("shape", svg);
		positionGrips();
	}
}

void LogoItem::setProp(const QString & prop, const QString & value) {
	if (prop.compare("logo", Qt::CaseInsensitive) == 0) {
		setLogo(value, false);
		return;
	}

	ResizableBoard::setProp(prop, value);
}

void LogoItem::setLogo(QString logo, bool force) {

	if (!force && m_logo.compare(logo) == 0) return;

	QString svg;
	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::PCBView:
			// see if there's already a <text> element 
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
			setSharedRenderer(m_renderer);
		}
	}

	m_logo = logo;
	modelPart()->setProp("logo", logo);

	updateTooltip();
}

QString LogoItem::getProperty(const QString & key) {
	if (key.compare("logo", Qt::CaseInsensitive) == 0) {
		return m_logo;
	}

	return PaletteItem::getProperty(key);
}

const QString & LogoItem::logo() {
	return m_logo;
}

bool LogoItem::canEditPart() {
	return false;
}

