/*
 * (c) Fachhochschule Potsdam
 */

#include <QGridLayout>
#include <QLabel>
#include <QtAlgorithms>

#include "../waitpushundostack.h"
#include "../debugdialog.h"
#include "partseditormainwindow.h"
#include "partsymbolswidget.h"

#define EMPTY_BREAD_VIEW_TEXT "How does this part look like on the breadboard?"
#define EMPTY_SCHEM_VIEW_TEXT "How does this part look like in a schematic view?"
#define EMPTY_PCB_VIEW_TEXT "How does this part look like on a PCB?"

PartSymbolsWidget::PartSymbolsWidget(SketchModel *sketchModel, WaitPushUndoStack *undoStack, QWidget *parent) : QFrame(parent) {
	createViewImageWidget(sketchModel, undoStack, m_breadView, ItemBase::BreadboardView, "breadboard_icon.png", EMPTY_BREAD_VIEW_TEXT);
	createViewImageWidget(sketchModel, undoStack, m_schemView, ItemBase::SchematicView, "schematic_icon.png", EMPTY_SCHEM_VIEW_TEXT);
	createViewImageWidget(sketchModel, undoStack, m_pcbView, ItemBase::PCBView, "pcb_icon.png", EMPTY_PCB_VIEW_TEXT);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Symbols")),0,0);
	layout->addWidget(m_breadView,1,0);
	layout->addWidget(m_schemView,1,1);
	layout->addWidget(m_pcbView,1,2);
	this->setLayout(layout);

	this->setFixedHeight(200);
}

void PartSymbolsWidget::createViewImageWidget(
		SketchModel* sketchModel, WaitPushUndoStack *undoStack, PartsEditorViewImageWidget *&viw,
		ItemBase::ViewIdentifier viewId, QString iconFileName, QString startText
	) {

	Q_UNUSED(iconFileName);
	Q_UNUSED(startText);
	//viw = new PartsEditorViewImageWidget(viewId,tempDir(),PartsEditorMainWindow::emptyViewItem(iconFileName,startText),this);
	viw = new PartsEditorViewImageWidget(viewId,tempDir(),0,this);
	viw->setSketchModel(sketchModel);
	viw->setUndoStack(undoStack);

	switch( viewId ) {
		case ItemBase::BreadboardView: viw->addBreadboardViewLayers(); break;
		case ItemBase::SchematicView: viw->addSchematicViewLayers(); break;
		case ItemBase::PCBView: viw->addPcbViewLayers(); break;
		default: break;
	}
}

void PartSymbolsWidget::copySvgFilesToDestiny() {
	m_breadView->copySvgFileToDestiny();
	m_schemView->copySvgFileToDestiny();
	m_pcbView->copySvgFileToDestiny();
}

void PartSymbolsWidget::loadViewsImagesFromModel(PaletteModel *paletteModel, ModelPart *modelPart) {
	m_breadView->setPaletteModel(paletteModel);
	m_breadView->loadFromModel(paletteModel, modelPart);

	m_schemView->setPaletteModel(paletteModel);
	m_schemView->loadFromModel(paletteModel, modelPart);

	m_pcbView->setPaletteModel(paletteModel);
	m_pcbView->loadFromModel(paletteModel, modelPart);

	if(modelPart->connectors().size() > 0) {
		emit connectorsFound(modelPart->connectors().values());
	}
}

const QDir& PartSymbolsWidget::tempDir() {
	return ((PartsEditorMainWindow*)parentWidget())->tempDir();
}
