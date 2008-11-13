/*
 * (c) Fachhochschule Potsdam
 */

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
