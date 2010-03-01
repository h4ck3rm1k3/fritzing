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

#include <QAction>


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

	m_mainLayout = new QVBoxLayout();

	for (int i = 0; i < ViewLayer::ViewLayerCount; i++) {
		ViewLayerCheckBox * cb = new ViewLayerCheckBox(this);
		connect(cb, SIGNAL(clicked(bool)), this, SLOT(setLayerVisibility(bool)));
		m_checkBoxes.append(cb);
		cb->setVisible(false);

		m_spacerItems.append(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
	}

	m_groupBox = new QGroupBox("");
	QVBoxLayout * groupLayout = new QVBoxLayout();

	m_showAllWidget = new QPushButton(tr("show all layers"));
	connect(m_showAllWidget, SIGNAL(clicked()), this, SLOT(setAllLayersVisible()));

	m_hideAllWidget = new QPushButton(tr("hide all layers"));
	connect(m_hideAllWidget, SIGNAL(clicked()), this, SLOT(setAllLayersNotVisible()));

	groupLayout->addWidget(m_showAllWidget);
	groupLayout->addWidget(m_hideAllWidget);

	m_groupBox->setLayout(groupLayout);

	m_mainLayout->addWidget(m_groupBox);

	this->setLayout(m_mainLayout);


}

LayerPalette::~LayerPalette() 
{
}

void LayerPalette::resetLayout(LayerHash & viewLayers, QList<ViewLayer::ViewLayerID> & keys) {
	m_mainLayout->removeWidget(m_groupBox);
	foreach (ViewLayerCheckBox * cb, m_checkBoxes) {
		m_mainLayout->removeWidget(cb);
		cb->setVisible(false);
	}
	foreach (QSpacerItem * si, m_spacerItems) {
		m_mainLayout->removeItem(si);
	}

	int ix = 0;
	foreach (ViewLayer::ViewLayerID key, keys) {
		ViewLayer * viewLayer = viewLayers.value(key);
		ViewLayerCheckBox * cb = m_checkBoxes[ix++];
		cb->setText(viewLayer->action()->text());
		cb->setViewLayer(viewLayer);
		cb->setVisible(true);
		m_mainLayout->addWidget(cb);
		m_mainLayout->addSpacerItem(m_spacerItems[ix - 1]);
	}

	m_mainLayout->addWidget(m_groupBox);
	m_mainLayout->invalidate();

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
		cb->setChecked(viewLayer->action()->isChecked());
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
