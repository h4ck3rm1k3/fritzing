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

#ifndef LAYERPALETTE_H
#define LAYERPALETTE_H

#include <QFrame>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSpacerItem>

#include "viewlayer.h"

class ViewLayerCheckBox : public QCheckBox 
{
Q_OBJECT
public:
	ViewLayerCheckBox(QWidget * parent = NULL);
	~ViewLayerCheckBox();

	void setViewLayer(ViewLayer *);
	ViewLayer * viewLayer();

protected:
	ViewLayer * m_viewLayer;
};

class LayerPalette : public QFrame
{
Q_OBJECT
public:
	LayerPalette(QWidget * parent = NULL);
	~LayerPalette();

	void updateLayerPalette(LayerHash & viewLayers, QList<ViewLayer::ViewLayerID> & keys);
	void resetLayout(LayerHash & viewLayers, QList<ViewLayer::ViewLayerID> & keys);
	void setShowAllLayersAction(QAction *);
	void setHideAllLayersAction(QAction *);

protected slots:
	void setLayerVisibility(bool vis);
	void setAllLayersVisible();
	void setAllLayersNotVisible();

protected:
	QPushButton * m_showAllWidget;
	QPushButton * m_hideAllWidget;
	QList <ViewLayerCheckBox *> m_checkBoxes;
	QList <QSpacerItem *> m_spacerItems;
	QVBoxLayout * m_mainLayout;
	QGroupBox * m_groupBox;

    QAction *m_showAllLayersAct;
	QAction *m_hideAllLayersAct;
};

#endif
