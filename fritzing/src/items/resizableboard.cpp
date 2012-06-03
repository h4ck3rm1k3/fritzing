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

#include "resizableboard.h"
#include "../utils/resizehandle.h"
#include "../utils/graphicsutils.h"
#include "../utils/folderutils.h"
#include "../utils/textutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "../commands.h"
#include "moduleidnames.h"
#include "../layerattributes.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QImageReader>
#include <QMessageBox>
#include <QRegExp>
#include <qmath.h>
#include <qnumeric.h>

static QString BoardLayerTemplate = "";
static QString SilkscreenLayerTemplate = "";
static const int LineThickness = 8;
static const QRegExp HeightExpr("height=\\'\\d*px");
static QString StandardCustomBoardExplanation;

QStringList Board::BoardImageNames;
QStringList Board::NewBoardImageNames;

const double ResizableBoard::CornerHandleSize = 7.0;

QString Board::OneLayerTranslated;
QString Board::TwoLayersTranslated;

Board::Board( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
    m_svgOnly = true;
	m_fileNameComboBox = NULL;
    if (isBoard(modelPart)) {
        if (modelPart->localProp("layers").isNull()) {
            modelPart->setLocalProp("layers", modelPart->properties().value("layers"));
        }
    }

    if (StandardCustomBoardExplanation.isEmpty()) {
        StandardCustomBoardExplanation = tr("\n\nA custom board svg typically has one silkscreen layer and one board layer.\n") +
                                            tr("Have a look at the circle_pcb.svg file in your Fritzing installation folder at parts/svg/core/pcb/.\n\n");
    }

}

Board::~Board() {
}

void Board::paintHover(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);
	Q_UNUSED(option);
	painter->save();
	painter->setOpacity(0);
	painter->fillPath(this->hoverShape(), QBrush(hoverColor));
	painter->restore();
}

QStringList Board::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("layers", Qt::CaseInsensitive) == 0) {
        QString realValue = modelPart()->localProp("layers").toString();
        if (!realValue.isEmpty()) {
            value = realValue;
        }
		QStringList result;
		if (OneLayerTranslated.isEmpty()) {
			OneLayerTranslated = tr("one layer (single-sided)");
		}
		if (TwoLayersTranslated.isEmpty()) {
			TwoLayersTranslated = tr("two layers (double-sided)");
		}

		result.append(OneLayerTranslated);
		result.append(TwoLayersTranslated);

		if (value == "1") {
			value = OneLayerTranslated;
		}
		else if (value == "2") {
			value = TwoLayersTranslated;
		}

		return result;
	}


	QStringList result = PaletteItem::collectValues(family, prop, value);
    if (prop.compare("shape", Qt::CaseInsensitive) == 0 && isBoard(this) && !this->moduleID().contains(ModuleIDNames::BoardLogoImageModuleIDName)) {
        result.removeAll("Custom Shape");
    }

    return result;

}

bool Board::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
	if (prop.compare("filename", Qt::CaseInsensitive) == 0 && isBoard(this)) {
        setupLoadImage(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
		return true;
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

bool Board::rotation45Allowed() {
	return false;
}

bool Board::stickyEnabled() {
	return false;
}

ItemBase::PluralType Board::isPlural() {
	return Plural;
}

bool Board::canFindConnectorsUnder() {
	return false;
}


bool Board::isBoard(ItemBase * itemBase) {
    if (qobject_cast<Board *>(itemBase) == NULL) return false;

    return isBoard(itemBase->modelPart());
}

bool Board::isBoard(ModelPart * modelPart) {
    if (modelPart == NULL) return false;

    switch (modelPart->itemType()) {
        case ModelPart::Board:
            return true;
        case ModelPart::ResizableBoard:
            return true;
        case ModelPart::Logo:
            return modelPart->family().contains("pcb", Qt::CaseInsensitive);
        default:
            return false;
    }
}

void Board::setupLoadImage(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
    Q_UNUSED(returnValue);
    Q_UNUSED(value);
    Q_UNUSED(prop);
    Q_UNUSED(family);
    Q_UNUSED(parent);

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
}

void Board::setFileNameItems() {
	if (m_fileNameComboBox == NULL) return;

	m_fileNameComboBox->addItems(getImageNames());
	m_fileNameComboBox->addItems(getNewImageNames());

	int ix = 0;
	foreach (QString name, getImageNames()) {
		if (prop("lastfilename").contains(name)) {
			m_fileNameComboBox->setCurrentIndex(ix);
			return;
		}
		ix++;
	}

	foreach (QString name, getNewImageNames()) {
		if (prop("lastfilename").contains(name)) {
			m_fileNameComboBox->setCurrentIndex(ix);
			return;
		}
		ix++;
	}
}

QStringList & Board::getImageNames() {
	return BoardImageNames;
}

QStringList & Board::getNewImageNames() {
	return NewBoardImageNames;
}

void Board::fileNameEntry(const QString & filename) {
	foreach (QString name, getImageNames()) {
		if (filename.compare(name) == 0) {
			QString f = FolderUtils::getApplicationSubFolderPath("parts") + "/svg/core/pcb/" + filename + ".svg";
			return prepLoadImageAux(f, false);
			break;
		}
	}

	prepLoadImageAux(filename, true);
}

void Board::prepLoadImage() {
	QString imagesStr = tr("Images");
	imagesStr += " (";
    if (!m_svgOnly) {
	    QList<QByteArray> supportedImageFormats = QImageReader::supportedImageFormats();
	    foreach (QByteArray ba, supportedImageFormats) {
		    imagesStr += "*." + QString(ba) + " ";
	    }
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

    if (!checkImage(fileName)) return;

	prepLoadImageAux(fileName, true);
}

bool Board::checkImage(const QString & filename) {
    QFile file(filename);
    
	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;

	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		unableToLoad(filename, tr("due to an xml problem: %1 line:%2 column:%3").arg(errorStr).arg(errorLine).arg(errorColumn));
		return false;
	}

    QDomElement root = domDocument.documentElement();
	if (root.tagName() != "svg") {
		unableToLoad(filename, tr("because the xml is not correctly formatted"));
		return false;
	}

    QList<QDomElement> elements;
    TextUtils::findElementsWithAttribute(root, "id", elements);
    int layers = 0;
    int boardLayers = 0;
    int silk1Layers = 0;
    bool boardHasChildren = false;
    foreach (QDomElement element, elements) {
        QString id = element.attribute("id");
        ViewLayer::ViewLayerID viewLayerID = ViewLayer::viewLayerIDFromXmlString(id);
        if (viewLayerID != ViewLayer::UnknownLayer) {
            layers++;
            if (viewLayerID == ViewLayer::Board) {
                boardLayers++;
                if (element.childNodes().count() > 0) {
                    boardHasChildren = true;
                }
            }
            if (viewLayerID == ViewLayer::Silkscreen1) {
                silk1Layers++;
            }
        }
    }

    if (boardLayers == 1 && !boardHasChildren) {
        unableToLoad(filename, tr("the <board> element contains no shape elements") + StandardCustomBoardExplanation);
        return false;
    }

    if ((boardLayers == 1) && (silk1Layers == 1)) return true;

    if (boardLayers > 1) {
        unableToLoad(filename, tr("because there are multiple <board> layers") + StandardCustomBoardExplanation);
        return false;
    }

    if (silk1Layers > 1) {
        unableToLoad(filename, tr("because there are multiple <silkscreen> layers") + StandardCustomBoardExplanation);
        return false;
    }

    if (layers > 0 && boardLayers == 0) {
        unableToLoad(filename, tr("because there is no <board> layer") + StandardCustomBoardExplanation);
        return false;
    }

    if (layers == 0 && root.childNodes().count() == 0) {
        unableToLoad(filename, tr("the svg contains no shape elements") + StandardCustomBoardExplanation);
        return false;
    }

    if (layers == 0 || (boardLayers == 1 && silk1Layers == 0)) {
        return canLoad(filename, tr("but the pcb itself will have no silkscreen layer") + StandardCustomBoardExplanation);
    }

    unableToLoad(filename, tr("the svg doesn't fit the custom board format") + StandardCustomBoardExplanation);
    return false;
}

void Board::unableToLoad(const QString & fileName, const QString & reason) {
	QMessageBox::information(
		NULL,
		tr("Unable to load"),
		tr("Unable to load image from %1 %2").arg(fileName).arg(reason)
	);
}

bool Board::canLoad(const QString & fileName, const QString & reason) {
    QMessageBox::StandardButton answer = QMessageBox::question(
		NULL,
		tr("Can load, but"),
		tr("The image from %1 can be loaded, but %2\nUse the file?").arg(fileName).arg(reason),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No
	);
	return answer == QMessageBox::Yes;
}

void Board::prepLoadImageAux(const QString & fileName, bool addName)
{
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->loadLogoImage(this, "", QSizeF(0,0), "", fileName, addName);
	}
}

///////////////////////////////////////////////////////////

ResizableBoard::ResizableBoard( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: Board(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	fixWH();

	m_keepAspectRatio = false;
	m_widthEditor = m_heightEditor = NULL;
    m_aspectRatioCheck = NULL;
    m_aspectRatioLabel = NULL;

	m_silkscreenRenderer = NULL;
	m_corner = ResizableBoard::NO_CORNER;
	m_currentScale = 1.0;
	m_decimalsAfter = 1;
}

ResizableBoard::~ResizableBoard() {
}

void ResizableBoard::addedToScene(bool temporary) {

	loadTemplates();
	if (this->scene()) {
		setInitialSize();
	}

	Board::addedToScene(temporary);
}

void ResizableBoard::loadTemplates() {
	if (BoardLayerTemplate.isEmpty()) {
		QFile file(":/resources/templates/resizableBoard_boardLayerTemplate.txt");
		file.open(QFile::ReadOnly);
		BoardLayerTemplate = file.readAll();
		file.close();
	}
	if (SilkscreenLayerTemplate.isEmpty()) {
		QFile file(":/resources/templates/resizableBoard_silkscreenLayerTemplate.txt");
		file.open(QFile::ReadOnly);
		SilkscreenLayerTemplate = file.readAll();
		file.close();
	}
}

double ResizableBoard::minWidth() {
	return 0.5 * FSvgRenderer::printerScale();
}

double ResizableBoard::minHeight() {
	return 0.5 * FSvgRenderer::printerScale();
}


void ResizableBoard::mousePressEvent(QGraphicsSceneMouseEvent * event) 
{
	m_corner = ResizableBoard::NO_CORNER;

	if (m_spaceBarWasPressed) {
		Board::mousePressEvent(event);
		return;
	}

	double right = m_size.width();
	double bottom = m_size.height();

	m_resizeMousePos = event->scenePos();
	m_resizeStartPos = pos();
	m_resizeStartSize = m_size;
	m_resizeStartTopLeft = mapToScene(0, 0);
	m_resizeStartTopRight = mapToScene(right, 0);
	m_resizeStartBottomLeft = mapToScene(0, bottom);
	m_resizeStartBottomRight = mapToScene(right, bottom);

	m_corner = findCorner(event->scenePos(), event->modifiers());
	switch (m_corner) {
		case ResizableBoard::NO_CORNER:
			Board::mousePressEvent(event);
			return;
        default:
                break;
	}

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView) {
		setInitialSize();
		infoGraphicsView->viewItemInfo(this);
	}
}

void ResizableBoard::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	if (m_corner == ResizableBoard::NO_CORNER) {
		Board::mouseMoveEvent(event);
		return;
	}

	QPointF zero = mapToScene(0, 0);
	QPointF ds = mapFromScene(zero + event->scenePos() - m_resizeMousePos);
	QPointF newPos;
	QSizeF size = m_resizeStartSize;

	switch (m_corner) {
		case ResizableBoard::BOTTOM_RIGHT:
			size.setWidth(size.width() + ds.x());
			size.setHeight(size.height() + ds.y());
			break;
		case ResizableBoard::TOP_RIGHT:
			size.setWidth(size.width() + ds.x());
			size.setHeight(size.height() - ds.y());
			break;
		case ResizableBoard::BOTTOM_LEFT:
			size.setWidth(size.width() - ds.x());
			size.setHeight(size.height() + ds.y());
			break;
		case ResizableBoard::TOP_LEFT:
			size.setWidth(size.width() - ds.x());
			size.setHeight(size.height() - ds.y());
			break;
                default:
                        break;
	}

	if (size.width() < minWidth()) size.setWidth(minWidth());
	if (size.height() < minHeight()) size.setHeight(minHeight());

	if (m_keepAspectRatio) {
		double cw = size.height() * m_aspectRatio.width() / m_aspectRatio.height();
		double ch = size.width() * m_aspectRatio.height() / m_aspectRatio.width();
		if (ch < minHeight()) {
			size.setWidth(cw);
		}
		else if (cw < minWidth()) {
			size.setHeight(ch);
		}
		else {
			// figure out which one changes the area least
			double a1 = cw * size.height();
			double a2 = ch * size.width();
			double ac = m_size.width() * m_size.height();
			if (qAbs(ac - a1) <= qAbs(ac - a2)) {
				size.setWidth(cw);
			}
			else {
				size.setHeight(ch);
			}
		}
	}

	bool changePos = (m_corner != ResizableBoard::BOTTOM_RIGHT);
	bool changeTransform = !this->transform().isIdentity();

	LayerHash lh;
	QSizeF oldSize = m_size;
	resizePixels(size.width(), size.height(), lh);

	if (changePos) {
		if (changeTransform) {
			QTransform oldT = transform();

			DebugDialog::debug(QString("t old m:%1,%2,%3,%4 d:%5,%6 p:%7,%8 sz:%9,%10")
				.arg(oldT.m11()).arg(oldT.m12()).arg(oldT.m21()).arg(oldT.m22())
				.arg(oldT.toAffine().dx()).arg(oldT.toAffine().dy())
				.arg(pos().x()).arg(pos().y())
				.arg(oldSize.width()).arg(oldSize.height()));

			double sw = size.width() / 2;
			double sh = size.height() / 2;	
			QMatrix m(oldT.m11(), oldT.m12(), oldT.m21(), oldT.m22(), 0, 0);
			ds = m.inverted().map(ds);
			QTransform newT = QTransform().translate(-sw, -sh) * QTransform(m) * QTransform().translate(sw, sh);

			QList<ItemBase *> kin;
			kin << this->layerKinChief();
			foreach (ItemBase * lk, this->layerKinChief()->layerKin()) {
				kin << lk;
			}
			foreach (ItemBase * itemBase, kin) {
				itemBase->getViewGeometry().setTransform(newT);
				itemBase->setTransform(newT);
			}
			
			QTransform t = transform();
			DebugDialog::debug(QString("t new m:%1,%2,%3,%4 d:%5,%6 p:%7,%8 sz:%9,%10")
				.arg(t.m11()).arg(t.m12()).arg(t.m21()).arg(t.m22())
				.arg(t.toAffine().dx()).arg(t.toAffine().dy())
				.arg(pos().x()).arg(pos().y())
				.arg(size.width()).arg(size.height()));
		}
	
		QPointF actual;
		QPointF desired;
		switch (m_corner) {
			case ResizableBoard::TOP_RIGHT:
				actual = mapToScene(0, size.height());
				desired = m_resizeStartBottomLeft;
				break;
			case ResizableBoard::BOTTOM_LEFT:
				actual = mapToScene(size.width(), 0);
				desired = m_resizeStartTopRight;
				break;
			case ResizableBoard::TOP_LEFT:
				actual = mapToScene(size.width(), size.height());
				desired = m_resizeStartBottomRight;
				break;
                        default:
                                break;
		}	

		setPos(pos() + desired - actual);
	}
}

void ResizableBoard::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
	if (m_corner == ResizableBoard::NO_CORNER) {
		Board::mouseReleaseEvent(event);
		return;
	}

	event->accept();
	m_corner = ResizableBoard::NO_CORNER;
	setKinCursor(Qt::ArrowCursor);

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView) {
		infoGraphicsView->viewItemInfo(this);
	}
}

bool ResizableBoard::setUpImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, ViewLayer::ViewLayerSpec viewLayerSpec, bool doConnectors, LayerAttributes & layerAttributes, QString & error)
{
	bool result = Board::setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, viewLayerSpec, doConnectors, layerAttributes, error);

	return result;
}

ViewIdentifierClass::ViewIdentifier ResizableBoard::theViewIdentifier() {
	return ViewIdentifierClass::PCBView;
}

void ResizableBoard::resizePixels(double w, double h, const LayerHash & viewLayers) {
	resizeMM(GraphicsUtils::pixels2mm(w, FSvgRenderer::printerScale()), GraphicsUtils::pixels2mm(h, FSvgRenderer::printerScale()), viewLayers);
}

bool ResizableBoard::resizeMM(double mmW, double mmH, const LayerHash & viewLayers) {
	if (mmW == 0 || mmH == 0) {
		QString error;
		LayerAttributes layerAttributes;
		setUpImage(modelPart(), m_viewIdentifier, viewLayers, m_viewLayerID, m_viewLayerSpec, true, layerAttributes, error);
		modelPart()->setLocalProp("height", QVariant());
		modelPart()->setLocalProp("width", QVariant());
		// do the layerkin
		return false;
	}

	QRectF r = this->boundingRect();
	if (qAbs(GraphicsUtils::pixels2mm(r.width(), FSvgRenderer::printerScale()) - mmW) < .001 &&
		qAbs(GraphicsUtils::pixels2mm(r.height(), FSvgRenderer::printerScale()) - mmH) < .001) 
	{
		return false;
	}

	resizeMMAux(mmW, mmH);
    return true;
}


void ResizableBoard::resizeMMAux(double mmW, double mmH) {

	double milsW = GraphicsUtils::mm2mils(mmW);
	double milsH = GraphicsUtils::mm2mils(mmH);

	QString s = makeFirstLayerSvg(mmW, mmH, milsW, milsH);

	bool result = loadExtraRenderer(s.toUtf8(), false);
	if (result) {
		modelPart()->setLocalProp("width", mmW);
		modelPart()->setLocalProp("height", mmH);

		double tens = pow(10.0, m_decimalsAfter);
		setWidthAndHeight(qRound(mmW * tens) / tens, qRound(mmH * tens) / tens);
	}
	//	DebugDialog::debug(QString("fast load result %1 %2").arg(result).arg(s));

	foreach (ItemBase * itemBase, m_layerKin) {
		QString s = makeNextLayerSvg(itemBase->viewLayerID(), mmW, mmH, milsW, milsH);
		if (!s.isEmpty()) {
			if (m_silkscreenRenderer == NULL) {
				m_silkscreenRenderer = new FSvgRenderer(itemBase);
			}
			itemBase->prepareGeometryChange();
			bool result = m_silkscreenRenderer->loadSvgString(s);
			if (result) {
				qobject_cast<PaletteItemBase *>(itemBase)->setSharedRendererEx(m_silkscreenRenderer);
				itemBase->modelPart()->setLocalProp("width", mmW);
				itemBase->modelPart()->setLocalProp("height", mmH);
			}
			break;
		}
	}
}

void ResizableBoard::loadLayerKin( const LayerHash & viewLayers, ViewLayer::ViewLayerSpec viewLayerSpec) {

	loadTemplates();				
	Board::loadLayerKin(viewLayers, viewLayerSpec);
	double w =  m_modelPart->localProp("width").toDouble();
	if (w != 0) {
		resizeMM(w, m_modelPart->localProp("height").toDouble(), viewLayers);
	}
}

void ResizableBoard::setInitialSize() {
	double w =  m_modelPart->localProp("width").toDouble();
	if (w == 0) {
		// set the size so the infoGraphicsView will display the size as you drag
		QSizeF sz = this->boundingRect().size();
		modelPart()->setLocalProp("width", GraphicsUtils::pixels2mm(sz.width(), FSvgRenderer::printerScale())); 
		modelPart()->setLocalProp("height", GraphicsUtils::pixels2mm(sz.height(), FSvgRenderer::printerScale())); 
	}
}

QString ResizableBoard::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, double dpi)
{
	double w = m_modelPart->localProp("width").toDouble();
	if (w != 0) {
		double h = m_modelPart->localProp("height").toDouble();
		QString xml = makeLayerSvg(viewLayerID, w, h, GraphicsUtils::mm2mils(w), GraphicsUtils::mm2mils(h));
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

	return Board::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

QSizeF ResizableBoard::getSizeMM() {
	double w = m_modelPart->localProp("width").toDouble();
	double h = m_modelPart->localProp("height").toDouble();
	return QSizeF(w, h);
}

QString ResizableBoard::makeLayerSvg(ViewLayer::ViewLayerID viewLayerID, double mmW, double mmH, double milsW, double milsH) 
{
	switch (viewLayerID) {
		case ViewLayer::Board:
			return makeBoardSvg(mmW, mmH, milsW, milsH);
		case ViewLayer::Silkscreen1:
			return makeSilkscreenSvg(mmW, mmH, milsW, milsH);
			break;
		default:
			return "";
	}
}

QString ResizableBoard::makeNextLayerSvg(ViewLayer::ViewLayerID viewLayerID, double mmW, double mmH, double milsW, double milsH) {

	if (viewLayerID == ViewLayer::Silkscreen1) return makeSilkscreenSvg(mmW, mmH, milsW, milsH);

	return "";
}

QString ResizableBoard::makeFirstLayerSvg(double mmW, double mmH, double milsW, double milsH) {
	return makeBoardSvg(mmW, mmH, milsW, milsH);
}

QString ResizableBoard::makeBoardSvg(double mmW, double mmH, double milsW, double milsH) {
	return BoardLayerTemplate
		.arg(mmW).arg(mmH)			
		.arg(milsW).arg(milsH)
		.arg(milsW - LineThickness).arg(milsH - LineThickness);
}

QString ResizableBoard::makeSilkscreenSvg(double mmW, double mmH, double milsW, double milsH) {
	return SilkscreenLayerTemplate
		.arg(mmW).arg(mmH)
		.arg(milsW).arg(milsH)
		.arg(milsW - LineThickness).arg(milsH - LineThickness);
}

void ResizableBoard::saveParams() {
	double w = modelPart()->localProp("width").toDouble();
	double h = modelPart()->localProp("height").toDouble();
	m_boardSize = QSizeF(w, h);
	m_boardPos = pos();
}

void ResizableBoard::getParams(QPointF & p, QSizeF & s) {
	p = m_boardPos;
	s = m_boardSize;
}

bool ResizableBoard::hasCustomSVG() {
	return theViewIdentifier() == m_viewIdentifier;
}

bool ResizableBoard::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	bool result = Board::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);

	if (prop.compare("shape", Qt::CaseInsensitive) == 0) {

		returnProp = tr("shape");

		if (!m_modelPart->localProp("height").isValid()) { 
			// display uneditable width and height
			QFrame * frame = new QFrame();
			frame->setObjectName("infoViewPartFrame");		

			QVBoxLayout * vboxLayout = new QVBoxLayout();
			vboxLayout->setAlignment(Qt::AlignLeft);
			vboxLayout->setSpacing(0);
			vboxLayout->setMargin(0);
			vboxLayout->setContentsMargins(0, 3, 0, 0);

			double tens = pow(10.0, m_decimalsAfter);
			QRectF r = this->boundingRect();
			double w = qRound(GraphicsUtils::pixels2mm(r.width(), FSvgRenderer::printerScale()) * tens) / tens;
			QLabel * l1 = new QLabel(tr("width: %1mm").arg(w));	
			l1->setMargin(0);
			l1->setObjectName("infoViewLabel");		

			double h = qRound(GraphicsUtils::pixels2mm(r.height(), FSvgRenderer::printerScale()) * tens) / tens;
			QLabel * l2 = new QLabel(tr("height: %1mm").arg(h));
			l2->setMargin(0);
			l2->setObjectName("infoViewLabel");		

			if (returnWidget) vboxLayout->addWidget(qobject_cast<QWidget *>(returnWidget));
			vboxLayout->addWidget(l1);
			vboxLayout->addWidget(l2);

			frame->setLayout(vboxLayout);

			returnValue = l1->text() + "," + l2->text();
			returnWidget = frame;
			return true;
		}

		returnWidget = setUpDimEntry(false, returnWidget);
		return true;
	}

	return result;
}

QStringList ResizableBoard::collectValues(const QString & family, const QString & prop, QString & value) {
	return Board::collectValues(family, prop, value);
}

void ResizableBoard::widthEntry() {
	QLineEdit * edit = qobject_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	double w = edit->text().toDouble();
	double oldW = m_modelPart->localProp("width").toDouble();
	if (w == oldW) return;

	double h =  m_modelPart->localProp("height").toDouble();

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->resizeBoard(w, h, true);
	}
}

void ResizableBoard::heightEntry() {
	QLineEdit * edit = qobject_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	double h = edit->text().toDouble();
	double oldH =  m_modelPart->localProp("height").toDouble();
	if (h == oldH) return;

	double w =  m_modelPart->localProp("width").toDouble();

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->resizeBoard(w, h, true);
	}
}

bool ResizableBoard::hasPartNumberProperty()
{
	return false;
}

void ResizableBoard::paintSelected(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	if (m_hidden) return;

	Board::paintSelected(painter, option, widget);

	// http://www.gamedev.net/topic/441695-transform-matrix-decomposition/
	double m11 = painter->worldTransform().m11();
	double m12 = painter->worldTransform().m12();
	double scale = m_currentScale = qSqrt((m11 * m11) + (m12 * m12));   // assumes same scaling for both x and y

	double scalefull = CornerHandleSize / scale;
	double scalehalf = scalefull / 2;
	double bottom = m_size.height();
	double right = m_size.width();
	
	QPen pen;
	pen.setWidthF(1.0 / scale);
	pen.setColor(QColor(0, 0, 0));
	QBrush brush(QColor(255, 255, 255));
	painter->setPen(pen);
	painter->setBrush(brush);

	QPolygonF poly;

	// upper left
	poly.append(QPointF(0, 0));
	poly.append(QPointF(0, scalefull));
	poly.append(QPointF(scalehalf, scalefull));
	poly.append(QPointF(scalehalf, scalehalf));
	poly.append(QPointF(scalefull, scalehalf));
	poly.append(QPointF(scalefull, 0));
	painter->drawPolygon(poly);

	// upper right
	poly.clear();
	poly.append(QPointF(right, 0));
	poly.append(QPointF(right, scalefull));
	poly.append(QPointF(right - scalehalf, scalefull));
	poly.append(QPointF(right - scalehalf, scalehalf));
	poly.append(QPointF(right - scalefull, scalehalf));
	poly.append(QPointF(right - scalefull, 0));
	painter->drawPolygon(poly);

	// lower left
	poly.clear();
	poly.append(QPointF(0, bottom - scalefull));
	poly.append(QPointF(0, bottom));
	poly.append(QPointF(scalefull, bottom));
	poly.append(QPointF(scalefull, bottom - scalehalf));
	poly.append(QPointF(scalehalf, bottom - scalehalf));
	poly.append(QPointF(scalehalf, bottom - scalefull));
	painter->drawPolygon(poly);

	// lower right
	poly.clear();
	poly.append(QPointF(right, bottom - scalefull));
	poly.append(QPointF(right, bottom));
	poly.append(QPointF(right - scalefull, bottom));
	poly.append(QPointF(right - scalefull, bottom - scalehalf));
	poly.append(QPointF(right - scalehalf, bottom - scalehalf));
	poly.append(QPointF(right - scalehalf, bottom - scalefull));
	painter->drawPolygon(poly);
}

bool ResizableBoard::inResize() {
	return m_corner != ResizableBoard::NO_CORNER;
}

void ResizableBoard::hoverEnterEvent( QGraphicsSceneHoverEvent * event ) {
	Board::hoverEnterEvent(event);
}

void ResizableBoard::hoverMoveEvent( QGraphicsSceneHoverEvent * event ) {
	Board::hoverMoveEvent(event);

	m_corner = findCorner(event->scenePos(), event->modifiers());
	QCursor cursor;
	switch (m_corner) {
		case ResizableBoard::BOTTOM_RIGHT:
		case ResizableBoard::TOP_LEFT:
		case ResizableBoard::TOP_RIGHT:
		case ResizableBoard::BOTTOM_LEFT:
			//DebugDialog::debug("setting scale cursor");
			cursor = *CursorMaster::ScaleCursor;
			break;
		default:
			//DebugDialog::debug("setting other cursor");
			cursor = Qt::ArrowCursor;
			break;
	}
	setKinCursor(cursor);

}

void ResizableBoard::hoverLeaveEvent( QGraphicsSceneHoverEvent * event ) {
	setKinCursor(Qt::ArrowCursor);
	//DebugDialog::debug("setting arrow cursor");		
	Board::hoverLeaveEvent(event);
}

ResizableBoard::Corner ResizableBoard::findCorner(QPointF scenePos, Qt::KeyboardModifiers modifiers) {
	Q_UNUSED(modifiers);
		
	if (!this->isSelected()) return ResizableBoard::NO_CORNER;

	double d = CornerHandleSize / m_currentScale;
	double d2 = d * d;
	double right = m_size.width();
	double bottom = m_size.height();
	//DebugDialog::debug(QString("size %1 %2").arg(right).arg(bottom));
	QPointF q = mapToScene(right, bottom);
	if (GraphicsUtils::distanceSqd(scenePos, q) <= d2) {
		return ResizableBoard::BOTTOM_RIGHT;
	}
	q = mapToScene(0, 0);
	if (GraphicsUtils::distanceSqd(scenePos, q) <= d2) {
		return ResizableBoard::TOP_LEFT;
	}
	q = mapToScene(right, 0);
	if (GraphicsUtils::distanceSqd(scenePos, q) <= d2) {
		return ResizableBoard::TOP_RIGHT;
	}
	q = mapToScene(0, bottom);
	if (GraphicsUtils::distanceSqd(scenePos, q) <= d2) {
		return ResizableBoard::BOTTOM_LEFT;
	}

	return ResizableBoard::NO_CORNER;
}

void ResizableBoard::setKinCursor(QCursor & cursor) {
	ItemBase * chief = this->layerKinChief();
	chief->setCursor(cursor);
	foreach (ItemBase * itemBase, chief->layerKin()) {
		itemBase->setCursor(cursor);
	}
}

void ResizableBoard::setKinCursor(Qt::CursorShape cursor) {
	ItemBase * chief = this->layerKinChief();
	chief->setCursor(cursor);
	foreach (ItemBase * itemBase, chief->layerKin()) {
		itemBase->setCursor(cursor);
	}
}

QFrame * ResizableBoard::setUpDimEntry(bool includeProportion, QWidget * & returnWidget)
{
	double tens = pow(10.0, m_decimalsAfter);
	double w = qRound(m_modelPart->localProp("width").toDouble() * tens) / tens;	// truncate to 1 decimal point
	double h = qRound(m_modelPart->localProp("height").toDouble() * tens) / tens;  // truncate to 1 decimal point

	QFrame * frame = new QFrame();
	frame->setObjectName("infoViewPartFrame");
	QVBoxLayout * vboxLayout = new QVBoxLayout();
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

	QLabel * l1 = new QLabel(tr("width(mm)"));	
	l1->setMargin(0);
	l1->setObjectName("infoViewLabel");
	QLineEdit * e1 = new QLineEdit();
	QDoubleValidator * validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, m_decimalsAfter);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e1->setObjectName("infoViewLineEdit");
	e1->setValidator(validator);
	e1->setMaxLength(4 + m_decimalsAfter);
	e1->setText(QString::number(w));

	QLabel * l2 = new QLabel(tr("height(mm)"));
	l2->setMargin(0);
	l2->setObjectName("infoViewLabel");
	QLineEdit * e2 = new QLineEdit();
	validator = new QDoubleValidator(e1);
	validator->setRange(0.1, 999.9, m_decimalsAfter);
	validator->setNotation(QDoubleValidator::StandardNotation);
	e2->setObjectName("infoViewLineEdit");
	e2->setValidator(validator);
	e2->setMaxLength(4 + m_decimalsAfter);
	e2->setText(QString::number(h));

	hboxLayout1->addWidget(l1);
	hboxLayout1->addWidget(e1);
	hboxLayout2->addWidget(l2);
	hboxLayout2->addWidget(e2);

	subframe1->setLayout(hboxLayout1);
	subframe2->setLayout(hboxLayout2);
	if (returnWidget != NULL) vboxLayout->addWidget(returnWidget);

	connect(e1, SIGNAL(editingFinished()), this, SLOT(widthEntry()));
	connect(e2, SIGNAL(editingFinished()), this, SLOT(heightEntry()));

	m_widthEditor = e1;
	m_heightEditor = e2;

	vboxLayout->addWidget(subframe1);
	vboxLayout->addWidget(subframe2);

	if (includeProportion) {
		QFrame * subframe3 = new QFrame();
		QHBoxLayout * hboxLayout3 = new QHBoxLayout();
		hboxLayout3->setAlignment(Qt::AlignLeft);
		hboxLayout3->setContentsMargins(0, 0, 0, 0);
		hboxLayout3->setSpacing(0);
	
		QLabel * l3 = new QLabel(tr("keep in proportion"));
		l3->setMargin(0);
		l3->setObjectName("infoViewLabel");
		QCheckBox * checkBox = new QCheckBox();
		checkBox->setChecked(m_keepAspectRatio);
		checkBox->setObjectName("infoViewCheckBox");

		hboxLayout3->addWidget(l3);
		hboxLayout3->addWidget(checkBox);
		subframe3->setLayout(hboxLayout3);
		connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(keepAspectRatio(bool)));
		m_aspectRatioCheck = checkBox;
        m_aspectRatioLabel = l3;

		vboxLayout->addWidget(subframe3);
	}

	frame->setLayout(vboxLayout);

	return frame;
}

void ResizableBoard::fixWH() {
	bool okw, okh;
	QString wstr = m_modelPart->localProp("width").toString();
	QString hstr = m_modelPart->localProp("height").toString();
	double w = wstr.toDouble(&okw);
	double h = hstr.toDouble(&okh);

	//DebugDialog::debug(QString("w:%1 %2 ok:%3 h:%4 %5 ok:%6")
					//.arg(wstr).arg(w).arg(okw)
					//.arg(hstr).arg(h).arg(okh));

	if ((!okw && !wstr.isEmpty()) || qIsNaN(w) || qIsInf(w) || (!okh && !hstr.isEmpty()) || qIsNaN(h) || qIsInf(h)) {
		DebugDialog::debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		DebugDialog::debug("bad width or height in ResizableBoard or subclass " + wstr + " " + hstr);
		DebugDialog::debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		m_modelPart->setLocalProp("width", "");
		m_modelPart->setLocalProp("height", "");
	}
}

void ResizableBoard::setWidthAndHeight(double w, double h)
{
	if (m_widthEditor) {
		m_widthEditor->setText(QString::number(w));
	}
	if (m_heightEditor) {
		m_heightEditor->setText(QString::number(h));
	}
}
