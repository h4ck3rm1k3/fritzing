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
#include <qmath.h>

static const int IndexCm = 0;
static const int IndexIn = 1;

Ruler::Ruler( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_widthEditor = NULL;
	m_unitsEditor = NULL;
	m_widthValidator = NULL;
	m_renderer = NULL;
	QString w = modelPart->prop("width").toString();
	if (w.isEmpty()) {
		m_modelPart->setProp("width", modelPart->properties().value("width", "10cm"));
	}
}

Ruler::~Ruler() {
}

void Ruler::resizeMM(qreal magnitude, qreal unitsFlag, const LayerHash & viewLayers) {
	Q_UNUSED(viewLayers);

	qreal w = convertToInches(modelPart()->prop("width").toString());
	QString units((unitsFlag == IndexCm) ? "cm" : "in");
	qreal newW = convertToInches(QString::number(magnitude) + units);
	if (w == newW) return;

	QString s = makeSvg(newW);

	if (m_renderer == NULL) {
		m_renderer = new FSvgRenderer(this);
	}

	bool result = m_renderer->fastLoad(s.toUtf8());
	if (result) {
		setSharedRenderer(m_renderer);
		modelPart()->setProp("width", QString::number(magnitude) + units);
	}
	//	DebugDialog::debug(QString("fast load result %1 %2").arg(result).arg(s));

}

QString Ruler::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	qreal w = convertToInches(m_modelPart->prop("width").toString());
	if (w != 0) {
		QString xml;
		switch (viewLayerID) {
			case ViewLayer::BreadboardRuler:
			case ViewLayer::SchematicRuler:
			case ViewLayer::PcbRuler:
				xml = makeSvg(w);
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

QString Ruler::makeSvg(qreal inches) {
	qreal cm = 1 / 2.54;
	qreal offset = 0.125;
	qreal mmW = inches * 25.4;
	QString svg = TextUtils::makeSVGHeader(FSvgRenderer::printerScale(), GraphicsUtils::StandardFritzingDPI, (inches + offset + offset) * FSvgRenderer::printerScale(), FSvgRenderer::printerScale());
	svg += "<g font-family='DroidSans' text-anchor='middle' font-size='100' stroke-width='1px' stroke='black'>";
	int counter = 0;
	for (int i = 0; i <= qCeil(mmW); i++) {
		qreal h = cm / 4;
		qreal x = (offset + (i / 25.4)) * 1000;
		if (i % 10 == 0) {
			h = cm / 2;
			qreal y = (h + .1) * 1000;
			svg += QString("<text x='%1' y='%2'>%3</text>")
					.arg(x)
					.arg(y)
					.arg(QString::number(counter++));
			if (counter == 1) {
				svg += QString("<text x='%1' y='%2'>cm</text>").arg(x + 103).arg(y);
			}
		}
		else if (i % 5 == 0) {
			h = 3 * cm / 8;
		}
		svg += QString("<line x1='%1' y1='0' x2='%1' y2='%2' />\n")
			.arg(x)
			.arg(h * 1000);
	}
	counter = 0;
	for (int i = 0; i <= inches * 16; i++) {
		qreal h = 0.125;
		qreal x = (offset + (i / 16.0)) * 1000;
		if (i % 16 == 0) {
			h = .125 +  (3.0 / 16);
			qreal y = 1000 - ((h + .015) * 1000);
			svg += QString("<text x='%1' y='%2'>%3</text>")
					.arg(x)
					.arg(y)
					.arg(QString::number(counter++));
			if (counter == 1) {
				svg += QString("<text x='%1' y='%2'>in</text>").arg(x + 81).arg(y);
			}
		}
		else if (i % 8 == 0) {
			h = .125 +  (2.0 / 16);
		}
		else if (i % 4 == 0) {
			h = .125 +  (1.0 / 16);
		}
		svg += QString("<line x1='%1' y1='%2' x2='%1' y2='1000' />\n")
			.arg(x)
			.arg(1000 - (h * 1000));
	}

	svg += "</g></svg>";

	return svg;
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
		int units = m_modelPart->prop("width").toString().contains("cm") ? IndexCm : IndexIn;
		QLineEdit * e1 = new QLineEdit();
		QDoubleValidator * validator = new QDoubleValidator(e1);
		validator->setRange(1.0, 20 * ((units == IndexCm) ? 2.54 : 1), 2);
		validator->setNotation(QDoubleValidator::StandardNotation);
		e1->setValidator(validator);
		QString temp = m_modelPart->prop("width").toString();
		temp.chop(2);
		e1->setText(temp);
		m_widthEditor = e1;
		m_widthValidator = validator;

		QComboBox * comboBox = new QComboBox(parent);
		comboBox->setEditable(false);
		comboBox->addItem("cm");
		comboBox->addItem("in");
		comboBox->setCurrentIndex(units);
		m_unitsEditor = comboBox;

		QHBoxLayout * hboxLayout = new QHBoxLayout();
		hboxLayout->setAlignment(Qt::AlignLeft);
		hboxLayout->setContentsMargins(0, 0, 0, 0);
		hboxLayout->setSpacing(0);

		hboxLayout->addWidget(e1);
		hboxLayout->addWidget(comboBox);

		QFrame * frame = new QFrame();
		frame->setLayout(hboxLayout);

		connect(e1, SIGNAL(editingFinished()), this, SLOT(widthEntry()));
		connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(unitsEntry(const QString &)));

		return frame;
	}

	return PaletteItem::createPlugin(parent, classid, url, paramNames, paramValues);

}

void Ruler::widthEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	QString t = edit->text();
	QString w = modelPart()->prop("width").toString();
	w.chop(2);
	if (t.compare(w) == 0) {
		return;
	}

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		int units = (m_unitsEditor->currentText() == "cm") ? IndexCm : IndexIn;
		infoGraphicsView->resizeBoard(edit->text().toDouble(), units, false);
	}
}

void Ruler::unitsEntry(const QString & units) {
	qreal inches = convertToInches(modelPart()->prop("width").toString());
	if (units == "in") {
		modelPart()->setProp("width", QString::number(inches) + "in");
		m_widthEditor->setText(QString::number(inches));
		m_widthValidator->setTop(20);
	}
	else {
		modelPart()->setProp("width", QString::number(inches * 2.54) + "cm");
		m_widthEditor->setText(QString::number(inches * 2.54));
		m_widthValidator->setTop(20 * 2.54);
	}
}

qreal Ruler::convertToInches(const QString & string) {
	bool ok;
	qreal retval = TextUtils::convertToInches(string, &ok, false);
	if (!ok) return 0;

	return retval;
}

bool Ruler::stickyEnabled() {
	return false;
}

