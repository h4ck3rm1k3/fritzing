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


#include <QGridLayout>

#include "connectorsviewswidget.h"

ConnectorsViewsWidget::ConnectorsViewsWidget(PartSymbolsWidget *symbols, SketchModel *sketchModel, WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info, QWidget *parent) : QFrame(parent) {
	createViewImageWidget(m_breadView, symbols->m_breadView, sketchModel, undoStack, info, ItemBase::BreadboardView, ViewLayer::Breadboard);
	createViewImageWidget(m_schemView, symbols->m_schemView, sketchModel, undoStack, info, ItemBase::SchematicView, ViewLayer::Schematic);
	createViewImageWidget(m_pcbView, symbols->m_pcbView, sketchModel, undoStack, info, ItemBase::PCBView, ViewLayer::Copper0);

	m_breadView->setViewLayerIDs(ViewLayer::Breadboard, ViewLayer::BreadboardWire, ViewLayer::Breadboard, ViewLayer::BreadboardRuler);
	m_schemView->setViewLayerIDs(ViewLayer::Schematic, ViewLayer::SchematicWire, ViewLayer::Schematic, ViewLayer::SchematicRuler);
	m_pcbView->setViewLayerIDs(ViewLayer::Schematic, ViewLayer::SchematicWire, ViewLayer::Schematic, ViewLayer::SchematicRuler);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(m_breadView,1,0);
	layout->addWidget(m_schemView,1,1);
	layout->addWidget(m_pcbView,1,2);

	this->setLayout(layout);

	this->setFixedHeight(170);
	this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}

void ConnectorsViewsWidget::createViewImageWidget(
		PartsEditorConnectorViewImageWidget *&viw, PartsEditorViewImageWidget* sister,
		SketchModel* sketchModel, WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info,
		ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerId) {
	viw = new PartsEditorConnectorViewImageWidget(viewId,this);
	connect(sister,SIGNAL(loadedFromModel(PaletteModel*, ModelPart*)),viw,SLOT(loadFromModel(PaletteModel*, ModelPart*)));
	connect(
		sister, SIGNAL(itemAddedToSymbols(ModelPart*, StringPair*)),
		viw, SLOT(addItemInPartsEditor(ModelPart *, StringPair*))
	);
	connect(
		viw, SIGNAL(svgFileLoadNeeded(const QString&)),
		sister, SLOT(loadSvgFile(const QString&))
	);
	connect(
		info, SIGNAL(connectorSelected(const QString&)),
		viw, SLOT(informConnectorSelection(const QString&))
	);
	connect(
		viw, SIGNAL(connectorsFound(ItemBase::ViewIdentifier, const QList<Connector*> &)),
		info, SLOT(syncNewConnectors(ItemBase::ViewIdentifier, const QList<Connector*> &))
	);
	/*connect(
		info, SIGNAL(existingConnector(ItemBase::ViewIdentifier, const QString &, Connector*)),
		viw, SLOT(setConnector(ItemBase::ViewIdentifier, const QString &, Connector*))
	);*/
	connect(
		info, SIGNAL(setMismatching(ItemBase::ViewIdentifier, const QString &, bool)),
		viw, SLOT(setMismatching(ItemBase::ViewIdentifier, const QString &, bool))
	);

	viw->setSketchModel(sketchModel);
	viw->setUndoStack(undoStack);
	viw->addViewLayer(new ViewLayer(viewLayerId, true, 2.5));
}

void ConnectorsViewsWidget::repaint() {
	m_breadView->scene()->update();
	m_schemView->scene()->update();
	m_pcbView->scene()->update();
}

void ConnectorsViewsWidget::drawConnector(Connector* conn) {
	m_breadView->drawConector(conn);
	m_schemView->drawConector(conn);
	m_pcbView->drawConector(conn);
}

void ConnectorsViewsWidget::aboutToSave() {
	m_breadView->updateDomIfNeeded();
	m_schemView->updateDomIfNeeded();
	m_pcbView->updateDomIfNeeded();
}


void ConnectorsViewsWidget::removeConnectorFrom(const QString &connId, ItemBase::ViewIdentifier viewId) {
	switch(viewId) {
		case ItemBase::AllViews:
			m_breadView->removeConnector(connId);
			m_schemView->removeConnector(connId);
			m_pcbView->removeConnector(connId);
			break;
		case ItemBase::BreadboardView:
			m_breadView->removeConnector(connId);
			break;
		case ItemBase::SchematicView:
			m_schemView->removeConnector(connId);
			break;
		case ItemBase::PCBView:
			m_pcbView->removeConnector(connId);
			break;
		default: Q_ASSERT(false);
	}
}
