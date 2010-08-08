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

#include "tracewire.h"
#include "../sketch/infographicsview.h"
#include "../connectors/connectoritem.h"

#include <QComboBox>

/////////////////////////////////////////////////////////

TraceWire::TraceWire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu  ) 
	: ClipableWire(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu, true)
{
	m_canChainMultiple = true;
}


TraceWire::~TraceWire() 	
{
}

bool TraceWire::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	if (prop.compare("width", Qt::CaseInsensitive) == 0) {
		returnProp = tr("width");
		QComboBox * comboBox = new QComboBox(parent);
		comboBox->setEditable(false);
		comboBox->setEnabled(swappingEnabled);
		
		int ix = 0;
		qreal m = mils();
		foreach(long widthValue, Wire::widths) {
			QString widthName = Wire::widthTrans.value(widthValue);
			QVariant val((int)widthValue);
			comboBox->addItem(widthName, val);
			if (qAbs(m - widthValue) < .01) {
				comboBox->setCurrentIndex(ix);
			}
			ix++;
		}

		comboBox->setMaximumWidth(200);

		connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(widthEntry(const QString &)));
		returnWidget = comboBox;

		return true;
	}

	return ClipableWire::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void TraceWire::widthEntry(const QString & text) {
	Q_UNUSED(text);

	QComboBox * comboBox = dynamic_cast<QComboBox *>(sender());
	if (comboBox == NULL) return;

	long w = comboBox->itemData(comboBox->currentIndex()).toInt();

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->changeWireWidthMils(QString::number(w));
	}
}

void TraceWire::setColorFromElement(QDomElement & element) {
	switch (m_viewLayerID) {
		case ViewLayer::Copper0Trace:
			element.setAttribute("color", ViewLayer::Copper0Color);
			break;
		case ViewLayer::Copper1Trace:
			element.setAttribute("color", ViewLayer::Copper1Color);
			break;
		case ViewLayer::SchematicTrace:
			element.setAttribute("color", "#000000");
		default:
			break;
	}

	Wire::setColorFromElement(element);	
}

bool TraceWire::canSwitchLayers() {
	QList<Wire *> wires;
	QList<ConnectorItem *> uniqueEnds;
	QList<ConnectorItem *> ends;
	collectChained(wires, ends, uniqueEnds);
	if (ends.count() < 2) return false;   // should never happen, since traces have to be connected at both ends

	foreach (ConnectorItem * end, ends) {
		if (end->getCrossLayerConnectorItem() == NULL) return false;
	}

	return true;
}
