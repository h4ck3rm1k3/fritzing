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


#include <QHBoxLayout>

#include "connectorsviewswidget.h"

ConnectorsViewsWidget::ConnectorsViewsWidget(PartSymbolsWidget *symbols, SketchModel *sketchModel, WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info, QWidget *parent) : QFrame(parent) {
	m_showTerminalPointsCheckBox = new QCheckBox(this);
	m_showTerminalPointsCheckBox->setText(tr("Show Anchor Points"));
	connect(m_showTerminalPointsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showHideTerminalPoints(int)));

	// TODO: PARTS-EDITOR
	m_showTerminalPointsCheckBox->hide();

	createViewImageWidget(m_breadView, symbols->m_breadView, sketchModel, undoStack, info, ItemBase::BreadboardView, ViewLayer::Breadboard);
	createViewImageWidget(m_schemView, symbols->m_schemView, sketchModel, undoStack, info, ItemBase::SchematicView, ViewLayer::Schematic);
	createViewImageWidget(m_pcbView, symbols->m_pcbView, sketchModel, undoStack, info, ItemBase::PCBView, ViewLayer::Copper0);

	m_breadView->setViewLayerIDs(ViewLayer::Breadboard, ViewLayer::BreadboardWire, ViewLayer::Breadboard, ViewLayer::BreadboardRuler, ViewLayer::BreadboardLabel, ViewLayer::BreadboardNote);
	m_schemView->setViewLayerIDs(ViewLayer::Schematic, ViewLayer::SchematicWire, ViewLayer::Schematic, ViewLayer::SchematicRuler, ViewLayer::SchematicLabel, ViewLayer::SchematicNote);
	m_pcbView->setViewLayerIDs(ViewLayer::Schematic, ViewLayer::SchematicWire, ViewLayer::Schematic, ViewLayer::SchematicRuler, ViewLayer::SilkscreenLabel, ViewLayer::PcbNote);

	connectPair(m_breadView,m_schemView);
	connectPair(m_schemView,m_pcbView);
	connectPair(m_pcbView,m_breadView);

	connectToThis(m_breadView);
	connectToThis(m_schemView);
	connectToThis(m_pcbView);

	QFrame *viewsContainter = new QFrame(this);
	QHBoxLayout *layout1 = new QHBoxLayout(viewsContainter);
	layout1->addWidget(addZoomControls(m_breadView));
	layout1->addWidget(addZoomControls(m_schemView));
	layout1->addWidget(addZoomControls(m_pcbView));

	QFrame *toolsContainer = new QFrame(this);
	QHBoxLayout *layout2 = new QHBoxLayout(toolsContainer);
	layout2->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	layout2->addWidget(m_showTerminalPointsCheckBox);
	layout2->setMargin(1);
	layout2->setSpacing(1);

	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->addWidget(viewsContainter);
	lo->addWidget(toolsContainer);
	lo->setMargin(1);
	lo->setSpacing(1);

	this->resize(width(),220);
	this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}

void ConnectorsViewsWidget::createViewImageWidget(
		PartsEditorConnectorsView *&viw, PartsEditorSpecificationsView* sister,
		SketchModel* sketchModel, WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info,
		ItemBase::ViewIdentifier viewId, ViewLayer::ViewLayerID viewLayerId) {
	viw = new PartsEditorConnectorsView(viewId,sister->tempFolder(),showingTerminalPoints(),this);
	connect(sister,SIGNAL(loadedFromModel(PaletteModel*, ModelPart*)),viw,SLOT(loadFromModel(PaletteModel*, ModelPart*)));
	connect(
		sister, SIGNAL(itemAddedToSymbols(ModelPart*, SvgAndPartFilePath*)),
		viw, SLOT(addItemInPartsEditor(ModelPart *, SvgAndPartFilePath*))
	);
	connect(
		viw, SIGNAL(svgFileLoadNeeded(const QString&)),
		sister, SLOT(updateModelPart(const QString&))
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

void ConnectorsViewsWidget::connectPair(PartsEditorConnectorsView *v1, PartsEditorConnectorsView *v2) {
	connect(
		v1, SIGNAL(connectorSelected(const QString &)),
		v2, SLOT(informConnectorSelection(const QString &))
	);
	connect(
		v2, SIGNAL(connectorSelected(const QString &)),
		v1, SLOT(informConnectorSelection(const QString &))
	);
}

void ConnectorsViewsWidget::connectToThis(PartsEditorConnectorsView *v) {
	connect(
		v, SIGNAL(connectorSelected(const QString &)),
		this, SLOT(informConnectorSelection(const QString &))
	);
}

void ConnectorsViewsWidget::repaint() {
	m_breadView->scene()->update();
	m_schemView->scene()->update();
	m_pcbView->scene()->update();
}

void ConnectorsViewsWidget::drawConnector(Connector* conn) {
	bool showing = showingTerminalPoints();
	m_breadView->drawConector(conn,showing);
	m_schemView->drawConector(conn,showing);
	m_pcbView->drawConector(conn,showing);
}

void ConnectorsViewsWidget::aboutToSave() {
	m_breadView->aboutToSave();
	m_schemView->aboutToSave();
	m_pcbView->aboutToSave();
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


void ConnectorsViewsWidget::showHideTerminalPoints(int checkState) {
	bool show = checkStateToBool(checkState);

	m_breadView->showTerminalPoints(show);
	m_schemView->showTerminalPoints(show);
	m_pcbView->showTerminalPoints(show);
}

bool ConnectorsViewsWidget::showingTerminalPoints() {
	return checkStateToBool(m_showTerminalPointsCheckBox->checkState());
}

bool ConnectorsViewsWidget::checkStateToBool(int checkState) {
	if(checkState == Qt::Checked) {
		return true;
	} else if(checkState == Qt::Unchecked) {
		return false;
	}
	return false;
}

QCheckBox *ConnectorsViewsWidget::showTerminalPointsCheckBox() {
	return m_showTerminalPointsCheckBox;
}

void ConnectorsViewsWidget::informConnectorSelection(const QString &connId) {
	emit connectorSelectedInView(connId);
}

QWidget *ConnectorsViewsWidget::addZoomControls(PartsEditorConnectorsView *view) {
	QFrame *container = new QFrame(this);
	QVBoxLayout *lo = new QVBoxLayout(container);
	lo->setSpacing(1);
	lo->setMargin(0);
	lo->addWidget(view);
	lo->addWidget(new ZoomControls(view,container));

	return container;
}
