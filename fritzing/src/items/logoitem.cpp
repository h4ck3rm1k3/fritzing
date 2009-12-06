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
#include <QRegExp>
#include <QPushButton>
#include <QImageReader>
#include <QMessageBox>
#include <QImage>
#include <QLineEdit>

static const int LineThickness = 4;
static const QRegExp WidthExpr("width=\\'\\d*px");
static const QRegExp HeightExpr("height=\\'\\d*px");

LogoItem::LogoItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_aspectRatioCheck = NULL;
	m_keepAspectRatio = true;
	m_hasLogo = (modelPart->moduleID() == ModuleIDNames::logoTextModuleIDName);
	m_logo = modelPart->prop("logo").toString();
	if (m_hasLogo && m_logo.isEmpty()) {
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
				m_originalFilename = filename();
				QString shape = modelPart()->prop("shape").toString();
				if (!shape.isEmpty()) {
					// TODO: aspect ratio
					if (m_renderer == NULL) {
						m_renderer = new FSvgRenderer(this);
					}

					bool result = m_renderer->fastLoad(shape.toUtf8());
					if (result) {
						setSharedRenderer(m_renderer);
						positionGrips();
					}
				}
				else {
					QFile f(m_originalFilename);
					if (f.open(QFile::ReadOnly)) {
						QString svg = f.readAll();
						modelPart()->setProp("shape", svg);
					}
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
	if (m_hasLogo) {
		if (prop.compare("logo", Qt::CaseInsensitive) == 0) {
			returnProp = tr("logo");
			returnValue = "<object type='application/x-qt-plugin' classid='logo' width='100%' height='22px'></object>";  
			return true;
		}
	}
	else {
		if (prop.compare("filename", Qt::CaseInsensitive) == 0) {
			returnValue = QString("<object type='application/x-qt-plugin' classid='filename' width='100%' height='22px'></object>");
			returnProp = "";
			return true;
		}
	}

	if (prop.compare("shape", Qt::CaseInsensitive) == 0) {
		returnValue = QString("<object type='application/x-qt-plugin' classid='shape' width='100%' height='50px'></object>");
		returnProp = tr("size");
		return true;
	}

	return PaletteItem::collectExtraInfoHtml(family, prop, value, collectValues, returnProp, returnValue);

}

QObject * LogoItem::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {

	if (classid.compare("shape", Qt::CaseInsensitive) == 0) {
		// implemented below
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

	QVBoxLayout * vboxLayout = NULL;
	QFrame * frame = NULL;
	if (!m_hasLogo) {
		frame = new QFrame();
		vboxLayout = new QVBoxLayout();
		vboxLayout->setAlignment(Qt::AlignLeft);
		vboxLayout->setSpacing(0);
		vboxLayout->setContentsMargins(0, 3, 0, 0);
	}

	QFrame * subframe = new QFrame();
	QHBoxLayout * hboxLayout = new QHBoxLayout();
	hboxLayout->setAlignment(Qt::AlignLeft);
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	hboxLayout->setSpacing(0);

	QLabel * l1 = new QLabel(tr("width(mm)"));	
	l1->setMargin(0);
	QLineEdit * e1 = new QLineEdit();
	QDoubleValidator * validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, 1);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e1->setValidator(validator);
	e1->setMaxLength(5);
	e1->setText(QString::number(w));

	QLabel * l2 = new QLabel(tr("height(mm)"));
	l2->setMargin(0);
	QLineEdit * e2 = new QLineEdit();
	validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, 1);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e2->setValidator(validator);
	e2->setMaxLength(5);
	e2->setText(QString::number(h));

	QLabel * l3 = new QLabel(tr("w:h"));	
	l1->setMargin(0);
	QCheckBox * checkBox = new QCheckBox();
	checkBox->setChecked(m_keepAspectRatio);

	hboxLayout->addWidget(l1);
	hboxLayout->addWidget(e1);
	hboxLayout->addSpacing(8);
	hboxLayout->addWidget(l2);
	hboxLayout->addWidget(e2);
	hboxLayout->addSpacing(8);
	hboxLayout->addWidget(l3);
	hboxLayout->addWidget(checkBox);

	subframe->setLayout(hboxLayout);

	connect(e1, SIGNAL(editingFinished()), this, SLOT(widthEntry()));
	connect(e2, SIGNAL(editingFinished()), this, SLOT(heightEntry()));
	connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(keepAspectRatio(bool)));

	m_widthEditor = e1;
	m_heightEditor = e2;
	m_aspectRatioCheck = checkBox;

	if (m_hasLogo) {
		return subframe;
	}

	vboxLayout->addWidget(subframe);
	frame->setLayout(vboxLayout);

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

	QString svg;
	if (fileName.endsWith(".svg")) {
		QFile f(fileName);
		if (f.open(QFile::ReadOnly)) {
			svg = f.readAll();
		}
		if (svg.isEmpty()) {
			unableToLoad(fileName);
			return;
		}


		TextUtils::fixPixelDimensionsIn(svg);
		TextUtils::cleanSodipodi(svg);
		TextUtils::fixViewboxOrigin(svg);

		QString errorStr;
		int errorLine;
		int errorColumn;

		QDomDocument domDocument;

		if (!domDocument.setContent(svg, true, &errorStr, &errorLine, &errorColumn)) {
			unableToLoad(fileName);
			return;
		}

		QDomElement root = domDocument.documentElement();
		if (root.isNull()) {
			unableToLoad(fileName);
			return;
		}

		if (root.tagName() != "svg") {
			unableToLoad(fileName);
			return;
		}

		QStringList exceptions;
		QString toColor("#ffffff");
		SvgFileSplitter::changeColors(root, toColor, exceptions);

		QString viewBox = root.attribute("viewBox");
		if (viewBox.isEmpty()) {
			bool ok1, ok2;
			qreal w = TextUtils::convertToInches(root.attribute("width"), &ok1) * FSvgRenderer::printerScale();
			qreal h = TextUtils::convertToInches(root.attribute("height"), &ok2) * FSvgRenderer::printerScale();
			if (!ok1 || !ok2) {
				unableToLoad(fileName);
				return;
			}

			root.setAttribute("viewBox", QString("0 0 %1 %2").arg(w).arg(h));
		}

		svg = TextUtils::removeXMLEntities(domDocument.toString());
	}
	else {
		QImage image(fileName);
		if (image.isNull()) {
			unableToLoad(fileName);
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

		svg = newSvgs[0];
		for (int i = 1; i < newSvgs.length(); i++) {
			svg = TextUtils::mergeSvg(svg, newSvgs[i]);
		}
	}

	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	bool result = m_renderer->fastLoad(svg.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setProp("shape", svg);
		modelPart()->setProp("logo", "");
		QRectF r = m_renderer->viewBoxF();
		m_aspectRatio.setWidth(r.width());
		m_aspectRatio.setHeight(r.height());
		positionGrips();
		m_logo = "";
	}
	else {
		// restore previous (not sure whether this is necessary)
		m_renderer->fastLoad(modelPart()->prop("shape").toString().toUtf8());
		setSharedRenderer(m_renderer);
		unableToLoad(fileName);
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

	svg = TextUtils::removeXMLEntities(domDocument.toString());			

	bool result = m_renderer->fastLoad(svg.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setProp("shape", svg);
		modelPart()->setProp("width", mmW);
		modelPart()->setProp("height", mmH);
		positionGrips();
	}

	if (m_widthEditor) {
		m_widthEditor->setText(QString::number(qRound(mmW * 10) / 10.0));
	}

	if (m_heightEditor) {
		m_heightEditor->setText(QString::number(qRound(mmH * 10) / 10.0));
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

	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::PCBView:
			break;
		default:
			return;
	}

	QString svg;
	if (false  /* m_hasLogo */) {
		svg = modelPart()->prop("shape").toString();
	}
	else {
		QFile f(m_originalFilename);
		if (f.open(QFile::ReadOnly)) {
			svg = f.readAll();
		}
	}

	if (svg.isEmpty()) return;

	svg = hackSvg(svg, logo);

	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}
	//DebugDialog::debug(svg);

	bool result = m_renderer->fastLoad(svg.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		QRectF r = m_renderer->viewBoxF();
		m_aspectRatio.setWidth(r.width());
		m_aspectRatio.setHeight(r.height());
	}

	m_logo = logo;
	modelPart()->setProp("logo", logo);
	modelPart()->setProp("shape", svg);
	positionGrips();

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

bool LogoItem::hasPartLabel() {
	return false;
}

void LogoItem::logoEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setChipLabel(edit->text());
	}
}

QString LogoItem::hackSvg(const QString & svg, const QString & logo) {
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	if (!doc.setContent(svg, &errorStr, &errorLine, &errorColumn)) return svg;

	QDomElement root = doc.documentElement();
	root.setAttribute("width", QString::number(logo.length() * 0.1) + "in");
	
	QString viewBox = root.attribute("viewBox");
	QStringList coords = viewBox.split(" ", QString::SkipEmptyParts);
	coords[2] = QString::number(logo.length() * 10);
	root.setAttribute("viewBox", coords.join(" "));

	QDomNodeList domNodeList = root.elementsByTagName("text");
	for (int i = 0; i < domNodeList.count(); i++) {
		QDomElement node = domNodeList.item(i).toElement();
		if (node.isNull()) continue;

		if (node.attribute("id").compare("label") != 0) continue;

		node.setAttribute("x", QString::number(logo.length() * 5));

		QDomNodeList childList = node.childNodes();
		for (int j = 0; j < childList.count(); j++) {
			QDomNode child = childList.item(i);
			if (child.isText()) {
				child.setNodeValue(logo);

				modelPart()->setProp("width", logo.length() * 0.1 * 25.4);
				QString h = root.attribute("height");
				bool ok;
				modelPart()->setProp("height", TextUtils::convertToInches(h, &ok, false) * 25.4);
				return doc.toString();
			}
		}
	}

	return svg;
}

void LogoItem::widthEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	qreal w = edit->text().toDouble();
	qreal h = m_modelPart->prop("height").toDouble();
	if (m_keepAspectRatio) {
		h = w * m_aspectRatio.height() / m_aspectRatio.width();
	}

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->resizeBoard(w, h, true);
	}
}

void LogoItem::heightEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	qreal w = m_modelPart->prop("width").toDouble();
	qreal h = edit->text().toDouble();
	if (m_keepAspectRatio) {
		w = h * m_aspectRatio.width() / m_aspectRatio.height();
	}

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->resizeBoard(w, h, true);
	}
}

void LogoItem::unableToLoad(const QString & fileName) {
	QMessageBox::information(
		NULL,
		tr("Unable to load"),
		tr("Unable to load image from %1").arg(fileName)
	);
}

void LogoItem::keepAspectRatio(bool checkState) {
	m_keepAspectRatio = checkState;
}
