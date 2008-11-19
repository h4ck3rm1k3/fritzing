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
	m_toolbar->setFixedHeight(60);

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
