/*
 * (c) Fachhochschule Potsdam
 */

#include <QFrame>
#include <QGridLayout>

#include "partconnectorswidget.h"

PartConnectorsWidget::PartConnectorsWidget(QList<QWidget*> widgets, QWidget *parent) : QFrame(parent) {
	QGridLayout *layout = new QGridLayout();
	for(int i=0; i < widgets.size(); i++) {
		layout->addWidget(widgets[i],i,0);
	}
	layout->setMargin(4);
	layout->setSpacing(10);
	setLayout(layout);
}
