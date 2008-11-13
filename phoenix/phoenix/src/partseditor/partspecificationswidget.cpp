/*
 * (c) Fachhochschule Potsdam
 */

#include <QFrame>
#include <QGridLayout>
#include <QScrollBar>

#include "partspecificationswidget.h"
#include "../debugdialog.h"

PartSpecificationsWidget::PartSpecificationsWidget(QList<QWidget*> widgets, QWidget *parent) : QScrollArea(parent) {
	QGridLayout *layout = new QGridLayout();
	for(int i=0; i < widgets.size(); i++) {
		//widgets[i]->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
		//widgets[i]->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
		layout->addWidget(widgets[i],i,0);
		if(widgets[i]->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("editionStarted()")) > -1) {
			connect(widgets[i],SIGNAL(editionStarted()),this,SLOT(updateLayout()));
		}
		if(widgets[i]->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("editionFinished()")) > -1) {
			connect(widgets[i],SIGNAL(editionFinished()),this,SLOT(updateLayout()));
		}
	}
	layout->setMargin(0);
	layout->setSpacing(10);

	m_scrollContent = new QFrame(this);
	m_scrollContent->setObjectName("scroll");
	m_scrollContent->setLayout(layout);


	//m_scrollContent->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed));

	setWidget(m_scrollContent);
	setMinimumWidth(m_scrollContent->width()+15);

	QGridLayout *mylayout = new QGridLayout();
	mylayout->setMargin(0);
	mylayout->setSpacing(0);
	setLayout(mylayout);

	resize(sizeHint());
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void PartSpecificationsWidget::updateLayout() {
	m_scrollContent->adjustSize();
}

QSize PartSpecificationsWidget::sizeHint() {
	return QSize(width(),600);
}

