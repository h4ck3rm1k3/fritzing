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

#include "ruler.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../svg/svgfilesplitter.h"
#include "moduleidnames.h"
#include "../utils/textutils.h"
#include "../utils/boundedregexpvalidator.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>

Ruler::Ruler( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_widthEditor = NULL;
	m_renderer = NULL;
}

Ruler::~Ruler() {
}


QVariant Ruler::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
		case ItemSceneHasChanged:
			break;
		default:
			break;
   	}

    return PaletteItem::itemChange(change, value);
}

void Ruler::resizeMM(qreal mmW, qreal mmH, const LayerHash & viewLayers) {
	if (mmW == 0 || mmH == 0) {
		setUpImage(modelPart(), m_viewIdentifier, viewLayers, m_viewLayerID, true);
		modelPart()->setProp("height", QVariant());
		modelPart()->setProp("width", QVariant());
		// do the layerkin
		return;
	}

	QRectF r = this->boundingRect();
	if (qAbs(GraphicsUtils::pixels2mm(r.width()) - mmW) < .001 &&
		qAbs(GraphicsUtils::pixels2mm(r.height()) - mmH) < .001) 
	{
		return;
	}


	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	qreal milsW = GraphicsUtils::mm2mils(mmW);

	QString s = makeSvg(mmW, milsW);

	bool result = m_renderer->fastLoad(s.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setProp("width", mmW);
		modelPart()->setProp("height", mmH);

		if (m_widthEditor) {
			m_widthEditor->setText(QString::number(qRound(mmW * 10) / 10.0));
		}
	}
	//	DebugDialog::debug(QString("fast load result %1 %2").arg(result).arg(s));


}

void Ruler::loadLayerKin( const LayerHash & viewLayers) {
	PaletteItem::loadLayerKin(viewLayers);
	qreal w = m_modelPart->prop("width").toDouble();
	if (w != 0) {
		resizeMM(w, m_modelPart->prop("height").toDouble(), viewLayers);
	}
}

void Ruler::setInitialSize() {
	qreal w = m_modelPart->prop("width").toDouble();
	if (w == 0) {
		// set the size so the infoGraphicsView will display the size as you drag
		QSizeF sz = this->boundingRect().size();
		modelPart()->setProp("width", GraphicsUtils::pixels2mm(sz.width())); 
		modelPart()->setProp("height", GraphicsUtils::pixels2mm(sz.height())); 
	}
}

QString Ruler::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	qreal w = m_modelPart->prop("width").toDouble();
	if (w != 0) {
		QString xml;
		switch (viewLayerID) {
			case ViewLayer::BreadboardRuler:
			case ViewLayer::SchematicRuler:
			case ViewLayer::PcbRuler:
				xml = makeSvg(w, GraphicsUtils::mm2mils(w));
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

	return PaletteItemBase::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
}

QString Ruler::makeSvg(qreal mmW, qreal milsW) {
	return ___emptyString___;
}

bool Ruler::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::PCBView:
		case ViewIdentifierClass::SchematicView:
		case ViewIdentifierClass::BreadboardView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

bool Ruler::collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue) 
{
	bool result = PaletteItem::collectExtraInfoHtml(family, prop, value, collectValues, returnProp, returnValue);

	if (prop.compare("width", Qt::CaseInsensitive) == 0) {
		returnValue = "<object type='application/x-qt-plugin' classid='width' width='100%' height='22px'></object>";  
		returnProp = tr("width");
		return true;
	}

	return result;
}

QObject * Ruler::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {

	if (classid.compare("width", Qt::CaseInsensitive) == 0) {

		QLabel * l1 = new QLabel(tr("in/cm:"));	
		l1->setMargin(0);

		qreal w = qRound(m_modelPart->prop("width").toDouble() * 10) / 10.0;	// truncate to 1 decimal point

		QLineEdit * e1 = new QLineEdit();
		BoundedRegExpValidator * validator = new BoundedRegExpValidator(e1);
		validator->setConverter(convertToInches);
		validator->setBounds(1.0 / 2.54, 20);
		validator->setRegExp(QRegExp("\\d{1,3}((in)|(cm))"));
		e1->setValidator(validator);
		e1->setMaxLength(5);
		e1->setText(QString::number(w));
		m_widthEditor = e1;

		QHBoxLayout * hboxLayout = new QHBoxLayout();
		hboxLayout->setAlignment(Qt::AlignLeft);
		hboxLayout->setContentsMargins(0, 0, 0, 0);
		hboxLayout->setSpacing(0);

		hboxLayout->addWidget(l1);
		hboxLayout->addWidget(e1);

		QFrame * frame = new QFrame();
		frame->setLayout(hboxLayout);

		connect(e1, SIGNAL(editingFinished()), this, SLOT(widthEntry()));

		return frame;
	}

	return PaletteItem::createPlugin(parent, classid, url, paramNames, paramValues);

}

void Ruler::widthEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->resizeBoard(edit->text().toDouble(), m_modelPart->prop("height").toDouble(), true);
	}
}

qreal Ruler::convertToInches(const QString & string) {
	bool ok;
	qreal retval = TextUtils::convertToInches(string, &ok, false);
	if (!ok) return 0;

	return retval;
}



