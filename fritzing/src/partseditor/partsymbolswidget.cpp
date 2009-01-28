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



#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtAlgorithms>

#include "../waitpushundostack.h"
#include "../debugdialog.h"
#include "partseditormainwindow.h"
#include "partsymbolswidget.h"

#define EMPTY_BREAD_VIEW_TEXT "How does this\npart look like on\nthe breadboard?"
#define EMPTY_SCHEM_VIEW_TEXT "How does this\npart look like in\na schematic view?"
#define EMPTY_PCB_VIEW_TEXT "How does this\npart look like\non a PCB?"

PartSymbolsWidget::PartSymbolsWidget(SketchModel *sketchModel, WaitPushUndoStack *undoStack, QWidget *parent) : QFrame(parent) {
	createViewImageWidget(sketchModel, undoStack, m_breadView, ItemBase::BreadboardView, "breadboard_icon.png", EMPTY_BREAD_VIEW_TEXT);
	createViewImageWidget(sketchModel, undoStack, m_schemView, ItemBase::SchematicView, "schematic_icon.png", EMPTY_SCHEM_VIEW_TEXT);
	createViewImageWidget(sketchModel, undoStack, m_pcbView, ItemBase::PCBView, "pcb_icon.png", EMPTY_PCB_VIEW_TEXT);

	m_guidelines = new QLabel(tr("Please refer to the <a style='color: #52182C' href='http://new.fritzing.org/learning/tutorials/making-parts/'>guidelines</a> before modifying or creating parts"), this);
	m_guidelines->setOpenExternalLinks(true);

	QHBoxLayout *labelLayout = new QHBoxLayout();
	labelLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::Fixed));
	labelLayout->addWidget(m_guidelines);
	labelLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::Fixed));

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(new QLabel(tr("Symbols")),0,0);
	layout->addWidget(m_breadView,1,0);
	layout->addWidget(m_schemView,1,1);
	layout->addWidget(m_pcbView,1,2);
	layout->addLayout(labelLayout,3,0,1,3);
	this->setLayout(layout);

	this->setFixedHeight(225);
}

void PartSymbolsWidget::createViewImageWidget(
		SketchModel* sketchModel, WaitPushUndoStack *undoStack, PartsEditorViewImageWidget *&viw,
		ItemBase::ViewIdentifier viewId, QString iconFileName, QString startText
	) {

	viw = new PartsEditorViewImageWidget(viewId,tempDir(),PartsEditorMainWindow::emptyViewItem(iconFileName,startText),this);
	viw->setSketchModel(sketchModel);
	viw->setUndoStack(undoStack);
	viw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

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
	m_breadView->scene()->clear();
	m_breadView->setPaletteModel(paletteModel);
	m_breadView->loadFromModel(paletteModel, modelPart);

	m_schemView->scene()->clear();
	m_schemView->setPaletteModel(paletteModel);
	m_schemView->loadFromModel(paletteModel, modelPart);

	m_pcbView->scene()->clear();
	m_pcbView->setPaletteModel(paletteModel);
	m_pcbView->loadFromModel(paletteModel, modelPart);

	if(modelPart->connectors().size() > 0) {
		emit connectorsFound(modelPart->connectors().values());
	}
}

const QDir& PartSymbolsWidget::tempDir() {
	return ((PartsEditorMainWindow*)parentWidget())->tempDir();
}
