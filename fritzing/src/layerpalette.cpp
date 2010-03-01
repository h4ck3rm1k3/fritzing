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

#include "layerpalette.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSpacerItem>
#include <QAction>
#include <QPushButton>


ViewLayerCheckBox::ViewLayerCheckBox(QWidget * parent) : QCheckBox(parent) {
}


ViewLayerCheckBox::~ViewLayerCheckBox()
{
}

void ViewLayerCheckBox::setViewLayer(ViewLayer * viewLayer)
{
	m_viewLayer = viewLayer;
}

ViewLayer * ViewLayerCheckBox::viewLayer() {
	return m_viewLayer;
}

//////////////////////////////////////

LayerPalette::LayerPalette(QWidget * parent) : QFrame(parent) 
{
	m_hideAllLayersAct = m_showAllLayersAct = NULL;

	QVBoxLayout *layout = new QVBoxLayout();

	for (int i = 0; i < ViewLayer::ViewLayerCount; i++) {
		ViewLayerCheckBox * cb = new ViewLayerCheckBox();
		connect(cb, SIGNAL(clicked(bool)), this, SLOT(setLayerVisibility(bool)));
		m_checkBoxes.append(cb);
		cb->setVisible(true);
		layout->addWidget(cb);
	}

	QGroupBox * groupBox = new QGroupBox("");
	QVBoxLayout * groupLayout = new QVBoxLayout();

	m_showAllWidget = new QPushButton(tr("show all layers"));
	connect(m_showAllWidget, SIGNAL(clicked()), this, SLOT(setAllLayersVisible()));

	m_hideAllWidget = new QPushButton(tr("hide all layers"));
	connect(m_hideAllWidget, SIGNAL(clicked()), this, SLOT(setAllLayersNotVisible()));

	groupLayout->addWidget(m_showAllWidget);
	groupLayout->addWidget(m_hideAllWidget);

	groupBox->setLayout(groupLayout);

	layout->addWidget(groupBox);
	layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

	this->setLayout(layout);

	foreach (QCheckBox * cb, m_checkBoxes) {
		cb->setVisible(false);
	}
}

LayerPalette::~LayerPalette() 
{
}

void LayerPalette::updateLayerPalette(LayerHash & viewLayers, QList<ViewLayer::ViewLayerID> & keys)
{
	m_showAllWidget->setEnabled(m_showAllLayersAct->isEnabled());
	m_showAllWidget->setChecked(!m_showAllLayersAct->isEnabled());
	m_hideAllWidget->setEnabled(m_hideAllLayersAct->isEnabled());
	m_hideAllWidget->setChecked(!m_hideAllLayersAct->isEnabled());

	int ix = 0;
	foreach (ViewLayer::ViewLayerID key, keys) {
		ViewLayer * viewLayer = viewLayers.value(key);
		ViewLayerCheckBox * cb = m_checkBoxes[ix++];
		cb->setText(viewLayer->action()->text());
		cb->setChecked(viewLayer->action()->isChecked());
		cb->setVisible(true);
		cb->setViewLayer(viewLayer);
	}

	for (int i = keys.count(); i < ViewLayer::ViewLayerCount; i++) {
		m_checkBoxes[i]->setVisible(false);
	}
}

void LayerPalette::setLayerVisibility(bool) {
	ViewLayerCheckBox * cb = dynamic_cast<ViewLayerCheckBox *>(sender());
	if (cb == NULL) return;

	cb->viewLayer()->action()->trigger();
}

void LayerPalette::setShowAllLayersAction(QAction * action) 
{
	m_showAllLayersAct = action;
}

void LayerPalette::setHideAllLayersAction(QAction * action) 
{
	m_hideAllLayersAct = action;
}

void LayerPalette::setAllLayersVisible() {
	if (m_showAllLayersAct) {
		m_showAllLayersAct->trigger();
	}
}


void LayerPalette::setAllLayersNotVisible() {
	if (m_hideAllLayersAct) {
		m_hideAllLayersAct->trigger();
	}
}
