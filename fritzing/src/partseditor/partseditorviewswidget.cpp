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

$Revision: 2715 $:
$Author: merunga $:
$Date: 2009-03-24 18:18:30 +0100 (Tue, 24 Mar 2009) $

********************************************************************/

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtAlgorithms>

#include "partseditorviewswidget.h"
#include "partseditormainwindow.h"
#include "zoomcontrols.h"
#include "../utils/misc.h"
#include "../waitpushundostack.h"
#include "../debugdialog.h"

QString PartsEditorViewsWidget::EmptyBreadViewText = ___emptyString___;
QString PartsEditorViewsWidget::EmptySchemViewText = ___emptyString___;
QString PartsEditorViewsWidget::EmptyPcbViewText = ___emptyString___;

PartsEditorViewsWidget::PartsEditorViewsWidget(SketchModel *sketchModel, WaitPushUndoStack *undoStack, ConnectorsInfoWidget* info, QWidget *parent) : QFrame(parent) {
	init();

	m_showTerminalPointsCheckBox = new QCheckBox(this);
	m_showTerminalPointsCheckBox->setText(tr("Show Anchor Points"));
	connect(m_showTerminalPointsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showHideTerminalPoints(int)));

	m_breadView = createViewImageWidget(sketchModel, undoStack, ViewIdentifierClass::BreadboardView, "breadboard_icon.png", EmptyBreadViewText, info, ViewLayer::Breadboard);
	m_schemView = createViewImageWidget(sketchModel, undoStack, ViewIdentifierClass::SchematicView, "schematic_icon.png", EmptySchemViewText, info, ViewLayer::Schematic);
	m_pcbView = createViewImageWidget(sketchModel, undoStack, ViewIdentifierClass::PCBView, "pcb_icon.png", EmptyPcbViewText, info, ViewLayer::Copper0);

	m_guidelines = new QLabel(tr("Please refer to the <a style='color: #52182C' href='http://fritzing.org/learning/tutorials/creating-custom-parts/'>guidelines</a> before modifying or creating parts"), this);
	m_guidelines->setOpenExternalLinks(true);
	m_guidelines->setObjectName("guidelinesLabel");

	QHBoxLayout *labelLayout = new QHBoxLayout();
	labelLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::Fixed));
	labelLayout->addWidget(m_guidelines);
	labelLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::Fixed));

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
	layout1->addWidget(addZoomControlsAndBrowseButton(m_breadView));
	layout1->addWidget(addZoomControlsAndBrowseButton(m_schemView));
	layout1->addWidget(addZoomControlsAndBrowseButton(m_pcbView));

	QFrame *toolsContainer = new QFrame(this);
	QHBoxLayout *layout2 = new QHBoxLayout(toolsContainer);
	layout2->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	layout2->addWidget(m_showTerminalPointsCheckBox);
	layout2->setMargin(1);
	layout2->setSpacing(1);

	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->addLayout(labelLayout);
	lo->addWidget(viewsContainter);
	lo->addWidget(toolsContainer);
	lo->setMargin(1);
	lo->setSpacing(1);

	this->resize(width(),220);
	//this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}

PartsEditorViewsWidget::~PartsEditorViewsWidget() {
	if (m_breadView) delete m_breadView;
	if (m_schemView) delete m_schemView;
	if (m_pcbView) delete m_pcbView;
}

void PartsEditorViewsWidget::init() {
	m_connsPosChanged = false;

	if(EmptyBreadViewText == ___emptyString___) {
		EmptyBreadViewText = tr("What does this\npart look like on\nthe breadboard?");
	}
	if(EmptySchemViewText == ___emptyString___) {
		EmptySchemViewText = tr("What does this\npart look like in\na schematic view?");
	}
	if(EmptyPcbViewText == ___emptyString___) {
		EmptyPcbViewText = tr("What does this\npart look like in\nthe PCB view?");
	}
}

PartsEditorView * PartsEditorViewsWidget::createViewImageWidget(
		SketchModel* sketchModel, WaitPushUndoStack *undoStack,
		ViewIdentifierClass::ViewIdentifier viewId, QString iconFileName, QString startText,
		ConnectorsInfoWidget* info, ViewLayer::ViewLayerID viewLayerId
	) {

	PartsEditorView * viw = new PartsEditorView(viewId,tempDir(),showingTerminalPoints(),PartsEditorMainWindow::emptyViewItem(iconFileName,startText),this);
	viw->setSketchModel(sketchModel);
	viw->setUndoStack(undoStack);
	viw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	viw->addViewLayer(new ViewLayer(viewLayerId, true, 2.5));

	m_views[viewId] = viw;

	connect(
		info, SIGNAL(connectorSelected(const QString&)),
		viw, SLOT(informConnectorSelection(const QString&))
	);
	connect(
		viw, SIGNAL(connectorsFound(ViewIdentifierClass::ViewIdentifier, const QList<Connector*> &)),
		info, SLOT(syncNewConnectors(ViewIdentifierClass::ViewIdentifier, const QList<Connector*> &))
	);
	/*connect(
		info, SIGNAL(existingConnector(ViewIdentifierClass::ViewIdentifier, const QString &, Connector*)),
		viw, SLOT(setConnector(ViewIdentifierClass::ViewIdentifier, const QString &, Connector*))
	);*/

	connect(
		info, SIGNAL(setMismatching(ViewIdentifierClass::ViewIdentifier, const QString &, bool)),
		viw, SLOT(setMismatching(ViewIdentifierClass::ViewIdentifier, const QString &, bool))
	);

	return viw;
}

void PartsEditorViewsWidget::copySvgFilesToDestiny(const QString &partFileName) {
	m_breadView->copySvgFileToDestiny(partFileName);
	m_schemView->copySvgFileToDestiny(partFileName);
	m_pcbView->copySvgFileToDestiny(partFileName);
}

void PartsEditorViewsWidget::loadViewsImagesFromModel(PaletteModel *paletteModel, ModelPart *modelPart) {
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

const QDir& PartsEditorViewsWidget::tempDir() {
	return ((PartsEditorMainWindow*)parentWidget())->tempDir();
}

void PartsEditorViewsWidget::connectPair(PartsEditorView *v1, PartsEditorView *v2) {
	connect(
		v1, SIGNAL(connectorSelected(const QString &)),
		v2, SLOT(informConnectorSelection(const QString &))
	);
	connect(
		v2, SIGNAL(connectorSelected(const QString &)),
		v1, SLOT(informConnectorSelection(const QString &))
	);
}

void PartsEditorViewsWidget::connectToThis(PartsEditorView *v) {
	connect(
		v, SIGNAL(connectorSelected(const QString &)),
		this, SLOT(informConnectorSelection(const QString &))
	);
}

void PartsEditorViewsWidget::repaint() {
	m_breadView->scene()->update();
	m_schemView->scene()->update();
	m_pcbView->scene()->update();
}

void PartsEditorViewsWidget::drawConnector(Connector* conn) {
	bool showing = showingTerminalPoints();
	m_breadView->drawConector(conn,showing);
	m_schemView->drawConector(conn,showing);
	m_pcbView->drawConector(conn,showing);
}

void PartsEditorViewsWidget::drawConnector(ViewIdentifierClass::ViewIdentifier viewId, Connector* conn) {
	bool showing = showingTerminalPoints();
	m_views[viewId]->drawConector(conn,showing);
}

void PartsEditorViewsWidget::setMismatching(ViewIdentifierClass::ViewIdentifier viewId, const QString &connId, bool mismatching) {
	m_views[viewId]->setMismatching(viewId, connId, mismatching);
	m_views[viewId]->scene()->update();
}

void PartsEditorViewsWidget::aboutToSave() {
	m_breadView->aboutToSave();
	m_schemView->aboutToSave();
	m_pcbView->aboutToSave();
}


void PartsEditorViewsWidget::removeConnectorFrom(const QString &connId, ViewIdentifierClass::ViewIdentifier viewId) {
	if(viewId == ViewIdentifierClass::AllViews) {
		m_breadView->removeConnector(connId);
		m_schemView->removeConnector(connId);
		m_pcbView->removeConnector(connId);
	} else {
		m_views[viewId]->removeConnector(connId);
	}
}


void PartsEditorViewsWidget::showHideTerminalPoints(int checkState) {
	bool show = checkStateToBool(checkState);

	m_breadView->showTerminalPoints(show);
	m_schemView->showTerminalPoints(show);
	m_pcbView->showTerminalPoints(show);
}

bool PartsEditorViewsWidget::showingTerminalPoints() {
	return checkStateToBool(m_showTerminalPointsCheckBox->checkState());
}

bool PartsEditorViewsWidget::checkStateToBool(int checkState) {
	if(checkState == Qt::Checked) {
		return true;
	} else if(checkState == Qt::Unchecked) {
		return false;
	}
	return false;
}

QCheckBox *PartsEditorViewsWidget::showTerminalPointsCheckBox() {
	return m_showTerminalPointsCheckBox;
}

void PartsEditorViewsWidget::informConnectorSelection(const QString &connId) {
	emit connectorSelectedInView(connId);
}

QWidget *PartsEditorViewsWidget::addZoomControlsAndBrowseButton(PartsEditorView *view) {
	QFrame *container1 = new QFrame(this);
	QVBoxLayout *lo1 = new QVBoxLayout(container1);
	lo1->setSpacing(1);
	lo1->setMargin(0);

	QLabel *button = new QLabel(QString("<a href='#'>%1</a>").arg(tr("Load image..")), this);
	button->setObjectName("browseButton");
	button->setMinimumWidth(85);
	button->setMaximumWidth(85);
	button->setFixedHeight(20);
	button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

	connect(button, SIGNAL(linkActivated(const QString&)), view, SLOT(loadFile()));
	QHBoxLayout *lo2 = new QHBoxLayout();
	lo2->setSpacing(1);
	lo2->setMargin(0);
	lo2->addWidget(button);
	lo2->addWidget(new ZoomControls(view,container1));

	lo1->addWidget(view);
	lo1->addLayout(lo2);

	return container1;
}

PartsEditorView *PartsEditorViewsWidget::breadboardView() {
	return m_breadView;
}

PartsEditorView *PartsEditorViewsWidget::schematicView() {
	return m_schemView;
}

PartsEditorView *PartsEditorViewsWidget::pcbView() {
	return m_pcbView;
}

bool PartsEditorViewsWidget::imagesLoadedInAllViews() {
	return m_breadView->imageLoaded()
		&& m_schemView->imageLoaded()
		&& m_pcbView->imageLoaded();
}

void PartsEditorViewsWidget::connectTerminalRemoval(const ConnectorsInfoWidget* connsInfo) {
	connect(
		m_breadView, SIGNAL(removeTerminalPoint(const QString&, ViewIdentifierClass::ViewIdentifier)),
		connsInfo, SLOT(removeTerminalPoint(const QString&, ViewIdentifierClass::ViewIdentifier))
	);
	connect(
		m_schemView, SIGNAL(removeTerminalPoint(const QString&, ViewIdentifierClass::ViewIdentifier)),
		connsInfo, SLOT(removeTerminalPoint(const QString&, ViewIdentifierClass::ViewIdentifier))
	);
	connect(
		m_pcbView, SIGNAL(removeTerminalPoint(const QString&, ViewIdentifierClass::ViewIdentifier)),
		connsInfo, SLOT(removeTerminalPoint(const QString&, ViewIdentifierClass::ViewIdentifier))
	);
}

bool PartsEditorViewsWidget::connectorsPosOrSizeChanged() {
	return m_breadView->connsPosOrSizeChanged()
			|| m_schemView->connsPosOrSizeChanged()
			|| m_pcbView->connsPosOrSizeChanged();
}

void PartsEditorViewsWidget::setViewItems(ItemBase* bbItem, ItemBase* schemItem, ItemBase* pcbItem) 
{
	m_breadView->setViewItem(bbItem);
	m_schemView->setViewItem(schemItem);
	m_pcbView->setViewItem(pcbItem);
}

