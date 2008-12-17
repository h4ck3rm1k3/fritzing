/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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



#include "abstractconnectorinfowidget.h"

#include <QVariant>

AbstractConnectorInfoWidget::AbstractConnectorInfoWidget(QWidget *parent) : QFrame(parent) {
	resize(sizeHint());
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
	setMinimumHeight(30);
}

QSize AbstractConnectorInfoWidget::sizeHint() {
	return QSize(480,30);
}

void AbstractConnectorInfoWidget::setSelected(bool selected, bool doEmitChange) {
	m_isSelected = selected;
	setProperty("selected",m_isSelected);

	reapplyStyle();

	if(selected) {
		setFocus();
		if(doEmitChange) {
			emit tellSistersImNewSelected(this);
		}
	}
}

void AbstractConnectorInfoWidget::reapplyStyle() {
	QString path = ":/resources/styles/partseditor.qss";
	QFile styleSheet(path);

	if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/partseditor.qss");
	} else {
		setStyleSheet(styleSheet.readAll());
	}
}

bool AbstractConnectorInfoWidget::isSelected() {
	return m_isSelected;
}
