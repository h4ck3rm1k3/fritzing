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

#include "tracewire.h"
#include "../sketch/infographicsview.h"

#include <QComboBox>

/////////////////////////////////////////////////////////

TraceWire::TraceWire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu  ) 
	: ClipableWire(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu)
{
	m_canChainMultiple = true;
}

bool TraceWire::collectExtraInfoHtml(const QString & prop, const QString & value, QString & returnProp, QString & returnValue) {

	if (prop.compare("width", Qt::CaseInsensitive) == 0) {
		returnProp = tr("width");
		returnValue = "<object type='application/x-qt-plugin' classid='WireWidthInput' width='125px' height='22px'></object>";
		return true;
	}

	return ClipableWire::collectExtraInfoHtml(prop, value, returnProp, returnValue);
}

QObject * TraceWire::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {
	Q_UNUSED(url);
	Q_UNUSED(paramNames);
	Q_UNUSED(paramValues);

	if (classid.compare("WireWidthInput", Qt::CaseInsensitive) != 0) {
		return ClipableWire::createPlugin(parent, classid, url, paramNames, paramValues);
	}

	QComboBox * comboBox = new QComboBox(parent);
	comboBox->setEditable(false);
	
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

	connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(widthEntry(const QString &)));
	return comboBox;
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
