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

#include "logoitem.h"
#include "../utils/graphicsutils.h"
#include "../utils/folderutils.h"
#include "../utils/textutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
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

static QStringList ImageNames;
static QStringList NewImageNames;
static QStringList Copper0ImageNames;
static QStringList NewCopper0ImageNames;
static QStringList Copper1ImageNames;
static QStringList NewCopper1ImageNames;

LogoItem::LogoItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (ImageNames.count() == 0) {
		ImageNames << "Made with Fritzing" << "Fritzing icon" << "OHANDA logo" << "OSHW logo";
	}

	m_fileNameComboBox = NULL;
	m_aspectRatioCheck = NULL;
	m_keepAspectRatio = true;
	m_hasLogo = (modelPart->moduleID() == ModuleIDNames::LogoTextModuleIDName);
	m_logo = modelPart->prop("logo").toString();
	if (m_hasLogo && m_logo.isEmpty()) {
		m_logo = modelPart->properties().value("logo", "logo");
		modelPart->setProp("logo", m_logo);
	}
}

LogoItem::~LogoItem() {
}


void LogoItem::addedToScene()
{
	if (this->scene()) {
		setInitialSize();
		m_aspectRatio.setWidth(this->boundingRect().width());
		m_aspectRatio.setHeight(this->boundingRect().height());
		m_originalFilename = filename();
		QString shape = modelPart()->prop("shape").toString();
		if (!shape.isEmpty()) {
					
			m_aspectRatio = modelPart()->prop("aspectratio").toSizeF();
			if (m_renderer == NULL) {
				m_renderer = new FSvgRenderer(this);
			}

			bool result = m_renderer->fastLoad(shape.toUtf8());
			if (result) {
				setSharedRendererEx(m_renderer);
				positionGrips();
			}
		}
		else {
			QFile f(m_originalFilename);
			if (f.open(QFile::ReadOnly)) {
				QString svg = f.readAll();
				f.close();
				modelPart()->setProp("shape", svg);
				modelPart()->setProp("lastfilename", m_originalFilename);
				initImage();
			}
		}
	}

    return ResizableBoard::addedToScene();
}


QString LogoItem::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi)
{
	if (viewLayerID == layer() ) {
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

bool LogoItem::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
	if (m_hasLogo) {
		if (prop.compare("logo", Qt::CaseInsensitive) == 0) {
			returnProp = tr("logo");
			returnValue = m_logo;

			QLineEdit * e1 = new QLineEdit(parent);
			e1->setObjectName("infoViewLineEdit");

			e1->setText(m_logo);
			e1->setEnabled(swappingEnabled);
			connect(e1, SIGNAL(editingFinished()), this, SLOT(logoEntry()));

			returnWidget = e1;
			return true;
		}
	}
	else {
		if (prop.compare("filename", Qt::CaseInsensitive) == 0) {
			returnProp = tr("image file");

			QFrame * frame = new QFrame();
			frame->setObjectName("infoViewPartFrame");
			QVBoxLayout * vboxLayout = new QVBoxLayout();
			vboxLayout->setContentsMargins(0, 0, 0, 0);
			vboxLayout->setSpacing(0);
			vboxLayout->setMargin(0);

			QComboBox * comboBox = new QComboBox();
			comboBox->setObjectName("infoViewComboBox");
			comboBox->setEditable(false);
			comboBox->setEnabled(swappingEnabled);
			m_fileNameComboBox = comboBox;

			setFileNameItems();

			connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(fileNameEntry(const QString &)));

			QPushButton * button = new QPushButton (tr("load image file"));
			button->setObjectName("infoViewButton");
			connect(button, SIGNAL(pressed()), this, SLOT(prepLoadImage()));
			button->setEnabled(swappingEnabled);

			vboxLayout->addWidget(comboBox);
			vboxLayout->addWidget(button);

			frame->setLayout(vboxLayout);
			returnWidget = frame;

			returnProp = "";
			return true;
		}
	}

	if (prop.compare("shape", Qt::CaseInsensitive) == 0) {
		returnProp = tr("shape");
		qreal w = qRound(m_modelPart->prop("width").toDouble() * 10) / 10.0;	// truncate to 1 decimal point
		qreal h = qRound(m_modelPart->prop("height").toDouble() * 10) / 10.0;  // truncate to 1 decimal point

		QVBoxLayout * vboxLayout = NULL;
		QFrame * frame = NULL;

		frame = new QFrame();
		frame->setObjectName("infoViewPartFrame");
		vboxLayout = new QVBoxLayout();
		vboxLayout->setAlignment(Qt::AlignLeft);
		vboxLayout->setSpacing(1);
		vboxLayout->setContentsMargins(0, 3, 0, 0);

		QFrame * subframe1 = new QFrame();
		QHBoxLayout * hboxLayout1 = new QHBoxLayout();
		hboxLayout1->setAlignment(Qt::AlignLeft);
		hboxLayout1->setContentsMargins(0, 0, 0, 0);
		hboxLayout1->setSpacing(2);

		QFrame * subframe2 = new QFrame();
		QHBoxLayout * hboxLayout2 = new QHBoxLayout();
		hboxLayout2->setAlignment(Qt::AlignLeft);
		hboxLayout2->setContentsMargins(0, 0, 0, 0);
		hboxLayout2->setSpacing(2);

		QFrame * subframe3 = new QFrame();
		QHBoxLayout * hboxLayout3 = new QHBoxLayout();
		hboxLayout3->setAlignment(Qt::AlignLeft);
		hboxLayout3->setContentsMargins(0, 0, 0, 0);
		hboxLayout3->setSpacing(0);

		QLabel * l1 = new QLabel(tr("width(mm)"));	
		l1->setMargin(0);
		l1->setObjectName("infoViewLabel");
		QLineEdit * e1 = new QLineEdit();
		QDoubleValidator * validator = new QDoubleValidator(e1);
		validator->setRange(0.1, 999.9, 1);
		validator->setNotation(QDoubleValidator::StandardNotation);
		e1->setObjectName("infoViewLineEdit");
		e1->setValidator(validator);
		e1->setMaxLength(5);
		e1->setText(QString::number(w));

		QLabel * l2 = new QLabel(tr("height(mm)"));
		l2->setMargin(0);
		l2->setObjectName("infoViewLabel");
		QLineEdit * e2 = new QLineEdit();
		validator = new QDoubleValidator(e1);
		validator->setRange(0.1, 999.9, 1);
		validator->setNotation(QDoubleValidator::StandardNotation);
		e2->setObjectName("infoViewLineEdit");
		e2->setValidator(validator);
		e2->setMaxLength(5);
		e2->setText(QString::number(h));

		QLabel * l3 = new QLabel(tr("keep in proportion"));
		l3->setMargin(0);
		l3->setObjectName("infoViewLabel");
		QCheckBox * checkBox = new QCheckBox();
		checkBox->setChecked(m_keepAspectRatio);
		checkBox->setObjectName("infoViewCheckBox");

		hboxLayout1->addWidget(l1);
		hboxLayout1->addWidget(e1);
		hboxLayout2->addWidget(l2);
		hboxLayout2->addWidget(e2);
		hboxLayout3->addWidget(l3);
		hboxLayout3->addWidget(checkBox);

		subframe1->setLayout(hboxLayout1);
		subframe2->setLayout(hboxLayout2);
		subframe3->setLayout(hboxLayout3);

		connect(e1, SIGNAL(editingFinished()), this, SLOT(widthEntry()));
		connect(e2, SIGNAL(editingFinished()), this, SLOT(heightEntry()));
		connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(keepAspectRatio(bool)));

		m_widthEditor = e1;
		m_heightEditor = e2;
		m_aspectRatioCheck = checkBox;

		vboxLayout->addWidget(subframe1);
		vboxLayout->addWidget(subframe2);
		vboxLayout->addWidget(subframe3);
		frame->setLayout(vboxLayout);

		returnWidget = frame;

		returnProp = tr("size");
		return true;
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);

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

	prepLoadImageAux(fileName, true);
}

void LogoItem::prepLoadImageAux(const QString & fileName, bool addName)
{
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->loadLogoImage(this->id(), modelPart()->prop("shape").toString(), m_aspectRatio, modelPart()->prop("lastfilename").toString(), fileName, addName);
	}
}

void LogoItem::reloadImage(const QString & svg, const QSizeF & aspectRatio, const QString & fileName, bool addName) 
{
	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}
	bool result = m_renderer->fastLoad(svg.toUtf8());
	if (result) {
		setSharedRendererEx(m_renderer);
		if (aspectRatio == QSizeF(0, 0)) {
			QRectF r = m_renderer->viewBoxF();
			m_aspectRatio.setWidth(r.width());
			m_aspectRatio.setHeight(r.height());
		}
		else {
			m_aspectRatio = aspectRatio;
		}
		modelPart()->setProp("aspectratio", m_aspectRatio);
		modelPart()->setProp("shape", svg);
		modelPart()->setProp("logo", "");
		modelPart()->setProp("lastfilename", fileName);
		if (addName) {
			if (!getNewImageNames().contains(fileName, Qt::CaseInsensitive)) {
				getNewImageNames().append(fileName);
				bool wasBlocked = m_fileNameComboBox->blockSignals(true);
				while (m_fileNameComboBox->count() > 0) {
					m_fileNameComboBox->removeItem(0);
				}
				setFileNameItems();
				m_fileNameComboBox->blockSignals(wasBlocked);
			}
		}
		positionGrips();
		m_logo = "";
	}
	else {
		// restore previous (not sure whether this is necessary)
		m_renderer->fastLoad(modelPart()->prop("shape").toString().toUtf8());
		setSharedRendererEx(m_renderer);
		unableToLoad(fileName);
	}
}

void LogoItem::loadImage(const QString & fileName, bool addName)
{
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

		TextUtils::cleanSodipodi(svg);
		TextUtils::fixPixelDimensionsIn(svg);
		TextUtils::fixViewboxOrigin(svg);
		TextUtils::tspanRemove(svg);

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
		exceptions << "none" << "";
		QString toColor(colorString());
		SvgFileSplitter::changeColors(root, toColor, exceptions);

		bool isIllustrator = TextUtils::isIllustratorDoc(domDocument);

		QString viewBox = root.attribute("viewBox");
		if (viewBox.isEmpty()) {
			bool ok1, ok2;
			qreal w = TextUtils::convertToInches(root.attribute("width"), &ok1, isIllustrator) * FSvgRenderer::printerScale();
			qreal h = TextUtils::convertToInches(root.attribute("height"), &ok2, isIllustrator) * FSvgRenderer::printerScale();
			if (!ok1 || !ok2) {
				unableToLoad(fileName);
				return;
			}

			root.setAttribute("viewBox", QString("0 0 %1 %2").arg(w).arg(h));
		}

		QList<QDomNode> rootChildren;
		QDomNode rootChild = root.firstChild();
		while (!rootChild.isNull()) {
			rootChildren.append(rootChild);
			rootChild = rootChild.nextSibling();
		}

		QDomElement topG = domDocument.createElement("g");
		topG.setAttribute("id", layerName());
		root.appendChild(topG);
		foreach (QDomNode node, rootChildren) {
			topG.appendChild(node);
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
		gpg.scanImage(image, image.width(), image.height(), 1, res, colorString(), layerName(), false, 1);
		QStringList newSvgs = gpg.newSVGs();
		if (newSvgs.count() < 1) {
			QMessageBox::information(
				NULL,
				tr("Unable to display"),
				tr("Unable to display image from %1").arg(fileName)
			);
			return;
		}

		QDomDocument doc;
		foreach (QString newSvg, newSvgs) {
			TextUtils::mergeSvg(doc, newSvg, layerName());
		}
		svg = TextUtils::mergeSvgFinish(doc);
	}

	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	reloadImage(svg, QSizeF(0, 0), fileName, addName);
}

void LogoItem::resizeMM(qreal mmW, qreal mmH, const LayerHash & viewLayers) {
	Q_UNUSED(viewLayers);

	if (mmW == 0 || mmH == 0) {
		return;
	}

	QRectF r = this->boundingRect();
	if (qAbs(GraphicsUtils::pixels2mm(r.width(), FSvgRenderer::printerScale()) - mmW) < .001 &&
		qAbs(GraphicsUtils::pixels2mm(r.height(), FSvgRenderer::printerScale()) - mmH) < .001) 
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
		setSharedRendererEx(m_renderer);
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
	QFile f(m_originalFilename);
	if (f.open(QFile::ReadOnly)) {
		svg = f.readAll();
	}

	if (svg.isEmpty()) return;

	svg = hackSvg(svg, logo);

	rerender(svg);

	m_logo = logo;
	modelPart()->setProp("logo", logo);
	modelPart()->setProp("shape", svg);
	positionGrips();

	updateTooltip();
}

void LogoItem::rerender(const QString & svg)
{
	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}
	//DebugDialog::debug(svg);

	bool result = m_renderer->fastLoad(svg.toUtf8());
	if (result) {
		setSharedRendererEx(m_renderer);
		QRectF r = m_renderer->viewBoxF();
		m_aspectRatio.setWidth(r.width());
		m_aspectRatio.setHeight(r.height());
	}
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

	if (edit->text().compare(this->logo()) == 0) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setProp(this, "logo", tr("logo"), this->logo(), edit->text(), true);
	}
}

void LogoItem::initImage() {
	if (m_hasLogo) {
		setLogo(m_logo, true);
		return;
	}

	loadImage(m_originalFilename, false);
}

QString LogoItem::hackSvg(const QString & svg, const QString & logo) 
{
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

void LogoItem::fileNameEntry(const QString & filename) {
	foreach (QString name, getImageNames()) {
		if (filename.compare(name) == 0) {
			QString f = FolderUtils::getApplicationSubFolderPath("parts") + "/svg/core/pcb/" + filename + ".svg";
			return prepLoadImageAux(f, false);
			break;
		}
	}

	prepLoadImageAux(filename, true);
}

void LogoItem::setFileNameItems() {
	if (m_fileNameComboBox == NULL) return;

	m_fileNameComboBox->addItems(getImageNames());
	m_fileNameComboBox->addItems(getNewImageNames());

	int ix = 0;
	foreach (QString name, getImageNames()) {
		if (modelPart()->prop("lastfilename").toString().contains(name)) {
			m_fileNameComboBox->setCurrentIndex(ix);
			return;
		}
		ix++;
	}

	foreach (QString name, getNewImageNames()) {
		if (modelPart()->prop("lastfilename").toString().contains(name)) {
			m_fileNameComboBox->setCurrentIndex(ix);
			return;
		}
		ix++;
	}
}

bool LogoItem::stickyEnabled() {
	return true;
}

ItemBase::PluralType LogoItem::isPlural() {
	return Singular;
}

ViewLayer::ViewLayerID LogoItem::layer() {
	return  ViewLayer::Silkscreen1;
}

QString LogoItem::colorString() {
	return ViewLayer::Silkscreen1Color;
}

QString LogoItem::layerName() 
{
	return ViewLayer::viewLayerXmlNameFromID(layer());
}

QStringList & LogoItem::getImageNames() {
	return ImageNames;
}

QStringList & LogoItem::getNewImageNames() {
	return NewImageNames;
}

///////////////////////////////////////////////////////////////////////

CopperLogoItem::CopperLogoItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: LogoItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (Copper1ImageNames.count() == 0) {
		Copper1ImageNames << "Fritzing icon copper1";
	}

	if (Copper0ImageNames.count() == 0) {
		Copper0ImageNames << "Fritzing icon copper1";
	}

	m_hasLogo = (modelPart->moduleID().endsWith(ModuleIDNames::LogoTextModuleIDName));
	m_logo = modelPart->prop("logo").toString();
	if (m_hasLogo && m_logo.isEmpty()) {
		m_logo = modelPart->properties().value("logo", "logo");
		modelPart->setProp("logo", m_logo);
	}
}

CopperLogoItem::~CopperLogoItem() {
}

ViewLayer::ViewLayerID CopperLogoItem::layer() {
	return modelPart()->properties().value("layer").contains("0") ? ViewLayer::Copper0 :  ViewLayer::Copper1;
}

QString CopperLogoItem::colorString() {
	return modelPart()->properties().value("layer").contains("0") ? ViewLayer::Copper0Color :  ViewLayer::Copper1Color;
}

QStringList & CopperLogoItem::getImageNames() {
	return modelPart()->properties().value("layer").contains("0") ? Copper0ImageNames :  Copper1ImageNames;
}

QStringList & CopperLogoItem::getNewImageNames() {
	return modelPart()->properties().value("layer").contains("0") ? NewCopper0ImageNames :  NewCopper1ImageNames;
}

QString CopperLogoItem::hackSvg(const QString & svg, const QString & logo) {
	QString newSvg = LogoItem::hackSvg(svg, logo);
	if (!modelPart()->properties().value("layer").contains("0")) return newSvg;

	return flipSvg(newSvg);
}

QString CopperLogoItem::flipSvg(const QString & svg)
{
	QString newSvg = svg;
	newSvg.replace("copper1", "copper0");
	newSvg.replace(ViewLayer::Copper1Color, ViewLayer::Copper0Color, Qt::CaseInsensitive);
	QMatrix m;
	QSvgRenderer renderer(newSvg.toUtf8());
	QRectF bounds = renderer.viewBoxF();
	m.translate(bounds.center().x(), bounds.center().y());
	QMatrix mMinus = m.inverted();
    QMatrix cm = mMinus * QMatrix().scale(-1, 1) * m;
	int gix = newSvg.indexOf("<g");
	newSvg.replace(gix, 2, "<g _flipped_='1' transform='" + TextUtils::svgMatrix(cm) + "'");
	return newSvg;
}

void CopperLogoItem::reloadImage(const QString & svg, const QSizeF & aspectRatio, const QString & fileName, bool addName) 
{
	if (modelPart()->properties().value("layer").contains("0")) {
		if (!svg.contains("_flipped_")) {
			LogoItem::reloadImage(flipSvg(svg), aspectRatio, fileName, addName);
			return;
		}
	}

	LogoItem::reloadImage(svg, aspectRatio, fileName, addName);
}
