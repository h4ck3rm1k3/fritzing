/*
 * (c) Fachhochschule Potsdam
 */

#include <QHBoxLayout>
#include <QLabel>

#include "sketchareawidget.h"

SketchAreaWidget::SketchAreaWidget(SketchWidget *graphicsView, QWidget *parent)
	: QFrame(parent)
{
	m_graphicsView = graphicsView;
	graphicsView->setParent(this);
	createLayout();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(m_graphicsView);
	layout->addWidget(m_toolbar);
}

SketchAreaWidget::~SketchAreaWidget() {
	// TODO Auto-generated destructor stub
}

ItemBase::ViewIdentifier SketchAreaWidget::viewIdentifier() {
	return m_graphicsView->viewIdentifier();
}

SketchWidget *SketchAreaWidget::graphicsView() {
	return m_graphicsView;
}

void SketchAreaWidget::createLayout() {
	m_toolbar = new QFrame(this);
	m_toolbar->setObjectName("sketchAreaToolbar");

	QFrame *leftButtons = new QFrame(m_toolbar);
	m_buttonsContainer = new QHBoxLayout(leftButtons);
	m_buttonsContainer->setMargin(0);
	m_buttonsContainer->setSpacing(3);

	QFrame *rightButtons = new QFrame(m_toolbar);
	m_zoomContainer = new QHBoxLayout(rightButtons);
	m_zoomContainer->setMargin(0);
	m_zoomContainer->setSpacing(3);
	m_zoomContainer->addWidget(new QLabel(tr("Zoom"),this));

	QHBoxLayout *toolbarLayout = new QHBoxLayout(m_toolbar);
	toolbarLayout->setMargin(2);
	toolbarLayout->setSpacing(0);
	toolbarLayout->addWidget(leftButtons);
	toolbarLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	toolbarLayout->addWidget(rightButtons);
}

void SketchAreaWidget::setContent(QList<QWidget*> buttons, ZoomComboBox *zoomComboBox) {
	foreach(QWidget* button, buttons) {
		m_buttonsContainer->addWidget(button);
	}
	m_zoomContainer->addWidget(zoomComboBox);
}
