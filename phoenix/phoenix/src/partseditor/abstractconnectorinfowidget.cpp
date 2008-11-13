/*
 * (c) Fachhochschule Potsdam
 */

#include "abstractconnectorinfowidget.h"

#include <QVariant>

AbstractConnectorInfoWidget::AbstractConnectorInfoWidget(QWidget *parent) : QFrame(parent) {
	resize(sizeHint());
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
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
