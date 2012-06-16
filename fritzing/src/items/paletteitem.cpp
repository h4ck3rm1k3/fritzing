/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "paletteitem.h"
#include "../debugdialog.h"
#include "../viewgeometry.h"
#include "../sketch/infographicsview.h"
#include "layerkinpaletteitem.h"
#include "../fsvgrenderer.h"
#include "partlabel.h"
#include "../commands.h"
#include "../connectors/connectoritem.h"
#include "../connectors/svgidlayer.h"
#include "../layerattributes.h"
#include "../dialogs/pinlabeldialog.h"
#include "../utils/folderutils.h"
#include "../utils/textutils.h"

#include <QGraphicsSceneMouseEvent>
#include <QSvgRenderer>
#include <QtDebug>
#include <QPainter>
#include <QDomElement>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRegExp>
#include <QGroupBox>
#include <QLabel>
#include <limits>

/////////////////////////////////////////////////

static bool ByIDParseSuccessful = true;
static QRegExp IntegerFinder("\\d+");


int findNumber(const QString & string) {
	int ix = string.indexOf(IntegerFinder);
	if (ix < 0) {
		return -1;
	}

	int result = IntegerFinder.cap(0).toInt();
	int length = IntegerFinder.cap(0).length();

	int jx = string.lastIndexOf(IntegerFinder);
	if (jx >= ix + length) {
		return -1;
	}

	return result;
}

bool byID(ConnectorItem * c1, ConnectorItem * c2)
{
	int i1 = findNumber(c1->connectorSharedID());
	if (i1 < 0) {
		ByIDParseSuccessful = false;
		return true;
	}
	int i2 = findNumber(c2->connectorSharedID());
	if (i2 < 0) {
		ByIDParseSuccessful = false;
		return true;
	}

	if (i2 == i1 && c1 != c2) {
		// should not be two connectors with the same number
		ByIDParseSuccessful = false;
		return true;
	}

	return i1 <= i2;
}

/////////////////////////////////////////////////

PaletteItem::PaletteItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	if(doLabel) {
		m_partLabel = new PartLabel(this, NULL);
		m_partLabel->setVisible(false);
	} else {
		m_partLabel = NULL;
	}
}

PaletteItem::~PaletteItem() {
	if (m_partLabel) {
		delete m_partLabel;
	}
}

bool PaletteItem::renderImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors, QString & error) {
	LayerAttributes layerAttributes; 
	bool result = setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, this->viewLayerSpec(), doConnectors, layerAttributes, error);

	m_syncMoved = this->pos();
	return result;
}

void PaletteItem::loadLayerKin(const LayerHash & viewLayers, ViewLayer::ViewLayerSpec viewLayerSpec) {

	if (m_modelPart == NULL) return;

	ModelPartShared * modelPartShared = m_modelPart->modelPartShared();
	if (modelPartShared == NULL) return;
	if (modelPartShared->domDocument() == NULL) return;

	qint64 id = m_id + 1;
	ViewGeometry viewGeometry = m_viewGeometry;
	viewGeometry.setZ(-1);

	LayerList notLayers;
	switch (viewLayerSpec) {
		case ViewLayer::ThroughHoleThroughTop_OneLayer:
			if (m_modelPart->flippedSMD()) {
				notLayers << ViewLayer::Copper1 << ViewLayer::Copper1Trace << ViewLayer::Silkscreen1 << ViewLayer::Silkscreen1Label;
			}
			//notLayers << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label << ViewLayer::Copper1 << ViewLayer:: Copper1Trace;
			break;
		case ViewLayer::ThroughHoleThroughTop_TwoLayers:
			if (m_modelPart->flippedSMD()) {
				notLayers << ViewLayer::Copper0 << ViewLayer::Copper0Trace << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label;
			}
			//notLayers << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label;
			break;
		case ViewLayer::ThroughHoleThroughBottom_TwoLayers:
			//notLayers << ViewLayer::Silkscreen << ViewLayer::SilkscreenLabel;
			break;

		case ViewLayer::GroundPlane_Top:
			notLayers << ViewLayer::GroundPlane0;
			break;
		case ViewLayer::GroundPlane_Bottom:
			notLayers << ViewLayer::GroundPlane1;
			break;

		// not sure these ever get used...
		case ViewLayer::SMDOnTop_TwoLayers:
			notLayers << ViewLayer::Copper0 << ViewLayer::Copper0Trace << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label;
			break;
		case ViewLayer::SMDOnBottom_TwoLayers:
		case ViewLayer::SMDOnBottom_OneLayer:
			notLayers << ViewLayer::Copper1 << ViewLayer::Copper1Trace << ViewLayer::Silkscreen1 << ViewLayer::Silkscreen1Label;
			break;

                case ViewLayer::Top:
                case ViewLayer::Bottom:
                case ViewLayer::TopAndBottom:
		case ViewLayer::WireOnTop_TwoLayers:
		case ViewLayer::WireOnBottom_OneLayer:
                case ViewLayer::WireOnBottom_TwoLayers:
                case ViewLayer::UnknownSpec:
			DebugDialog::debug("bad view spec in LoadLayerKin");
			break;
	}

	foreach (ViewLayer::ViewLayerID viewLayerID, viewLayers.keys()) {
		if (viewLayerID == m_viewLayerID) continue;
		if (notLayers.contains(viewLayerID)) continue;
		if (!m_modelPart->hasViewFor(m_viewIdentifier, viewLayerID)) continue;

		LayerKinPaletteItem * lkpi = newLayerKinPaletteItem(this, m_modelPart, m_viewIdentifier, viewGeometry, id, viewLayerID, viewLayerSpec, m_itemMenu, viewLayers);
		if (lkpi->ok()) {
			//DebugDialog::debug(QString("adding layer kin %1 %2 %3").arg(id).arg(m_viewIdentifier).arg(viewLayerID) );
			lkpi->setViewLayerSpec(viewLayerSpec);
			addLayerKin(lkpi);
			id++;
		}
		else {
			delete lkpi;
		}
	}
}

void PaletteItem::addLayerKin(LayerKinPaletteItem * lkpi) {
	m_layerKin.append(lkpi);
}

void PaletteItem::removeLayerKin() {
	// assumes paletteitem is still in scene
	for (int i = 0; i < m_layerKin.size(); i++) {
		//DebugDialog::debug(QString("removing kin %1 %2").arg(m_layerKin[i]->id()).arg(m_layerKin[i]->z()));
		this->scene()->removeItem(m_layerKin[i]);
		delete m_layerKin[i];
	}

	m_layerKin.clear();
}

void PaletteItem::syncKinSelection(bool selected, PaletteItemBase * originator) {
	PaletteItemBase::syncKinSelection(selected, originator);

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi != originator && lkpi->isSelected() != selected) {
				qobject_cast<LayerKinPaletteItem *>(lkpi)->blockItemSelectedChange(selected);
				lkpi->setSelected(selected);
		}
	}

	if (this != originator && this->isSelected() != selected) {
		this->blockItemSelectedChange(selected);
		this->setSelected(selected);
	}
}

QVariant PaletteItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	//DebugDialog::debug(QString("chief item change %1 %2").arg(this->id()).arg(change));
	if (m_layerKin.count() > 0) {
	    if (change == ItemSelectedChange) {
	       	bool selected = value.toBool();
	    	if (m_blockItemSelectedChange && m_blockItemSelectedValue == selected) {
	    		m_blockItemSelectedChange = false;
	   		}
			else {
	       		syncKinSelection(selected, this);
			}
	    }
	    //else if (change == ItemVisibleHasChanged && value.toBool()) {
	    	//this->setSelected(syncSelected());
	    	//this->setPos(m_offset + syncMoved());
	    //}
	    else if (change == ItemPositionHasChanged) {
	    	this->syncKinMoved(this->m_offset, value.toPointF());
	   	}
   	}

	if (m_partLabel && m_partLabel->initialized()) {
		if (change == ItemPositionHasChanged) {
	    	m_partLabel->ownerMoved(value.toPointF());
	   	}
		else if (change == ItemSelectedChange) {
			m_partLabel->update();
		}
	}

    return PaletteItemBase::itemChange(change, value);
}

const QList<class ItemBase *> & PaletteItem::layerKin()
{
	return m_layerKin;
}

void PaletteItem::rotateItem(double degrees) {
	PaletteItemBase::rotateItem(degrees);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->rotateItem(degrees);
	}
}

void PaletteItem::flipItem(Qt::Orientations orientation) {
	PaletteItemBase::flipItem(orientation);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->flipItem(orientation);
	}
}

void PaletteItem::transformItem2(const QMatrix & matrix) {
	PaletteItemBase::transformItem2(matrix);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->transformItem2(matrix);
	}
}

void PaletteItem::setTransforms() {
	setTransform(getViewGeometry().transform());
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->setTransform(m_layerKin[i]->getViewGeometry().transform());
	}
}

void PaletteItem::moveItem(ViewGeometry & viewGeometry) {
	PaletteItemBase::moveItem(viewGeometry);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->moveItem(viewGeometry);
	}
}

void PaletteItem::setItemPos(QPointF & loc) {
	PaletteItemBase::setItemPos(loc);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->setItemPos(loc);
	}
}

void PaletteItem::updateConnections() {
	updateConnectionsAux();
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->updateConnectionsAux();
	}
}

bool PaletteItem::collectFemaleConnectees(QSet<ItemBase *> & items) {
	bool hasMale = PaletteItemBase::collectFemaleConnectees(items);
	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi->collectFemaleConnectees(items)) {
			hasMale = true;
		}
	}
	return hasMale;
}

void PaletteItem::collectWireConnectees(QSet<Wire *> & wires) {
	PaletteItemBase::collectWireConnectees(wires);
	foreach (ItemBase * lkpi, m_layerKin) {
		qobject_cast<LayerKinPaletteItem *>(lkpi)->collectWireConnectees(wires);
	}
}

void PaletteItem::mousePressEvent(PaletteItemBase * originalItem, QGraphicsSceneMouseEvent *event) {
	//DebugDialog::debug("layerkinchief got mouse press event");
	/*

	if (acceptsMousePressConnectorEvent(NULL, event) && isBuriedConnectorHit(event)  ) return;
	foreach(LayerKinPaletteItem * lkpi, m_layerKin) {
		if (lkpi->isBuriedConnectorHit(event)) return;
	}
	*/

	PaletteItemBase::mousePressEvent(originalItem, event);
}

void PaletteItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	InfoGraphicsView *infographics = InfoGraphicsView::getInfoGraphicsView(this);
	if (infographics != NULL && infographics->spaceBarIsPressed()) { 
		event->ignore();
		return;
	}

	if (lowerConnectorLayerVisible(this)) {
		DebugDialog::debug("PaletteItem::mousePressEvent isn't obsolete");
		event->ignore();
		return;
	}

	mousePressEvent(this, event);
}


void PaletteItem::syncKinMoved(QPointF offset, QPointF newPos) {
	Q_UNUSED(offset);    // ignore offset--should all be zeros now

	//DebugDialog::debug(QString("sync kin moved %1 %2").arg(offset.x()).arg(offset.y()) );
	//m_syncMoved = pos - offset;
	//if (newPos != pos()) {
		setPos(newPos);
		foreach (ItemBase * lkpi, m_layerKin) {
			lkpi->setPos(newPos);
		}
	//}
}

void PaletteItem::setInstanceTitle(const QString& title) {
	ItemBase::setInstanceTitle(title);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setInstanceTitle(title);
	}
}

void PaletteItem::setHidden(bool hide) {
	ItemBase::setHidden(hide);
	figureHover();
}

void PaletteItem::setInactive(bool inactivate) {
	ItemBase::setInactive(inactivate);
	figureHover();
}

void PaletteItem::figureHover() {
	// if a layer contains connectors, make it the one that accepts hover events
	// if you make all layers accept hover events, then the topmost layer will get the event
	// and lower layers won't

	// note: this affects which layer responds to tooltips: see FGraphicsScene::helpEvent()

	QList<ItemBase *> allKin;
	allKin.append(this);
	foreach(ItemBase * lkpi, m_layerKin) {
		allKin.append(lkpi);
	}

	qSort(allKin.begin(), allKin.end(), ItemBase::zLessThan);
	foreach (ItemBase * base, allKin) {
		base->setAcceptHoverEvents(false);
		base->setAcceptedMouseButtons(Qt::NoButton);
	}

	int ix = 0;
	foreach (ItemBase * base, allKin) {
		if (!(base->hidden() || base->inactive()) && base->hasConnectors()) {
			base->setAcceptHoverEvents(true);
			base->setAcceptedMouseButtons(ALLMOUSEBUTTONS);
			break;
		}
		ix++;
	}

	for (int i = 0; i < ix; i++) {
		ItemBase * base = allKin[i];
		if (!(base->hidden() || base->inactive())) {
			base->setAcceptHoverEvents(true);
			base->setAcceptedMouseButtons(ALLMOUSEBUTTONS);
			return;
		}
	}
}

void PaletteItem::clearModelPart() {
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setModelPart(NULL);
	}
	ItemBase::clearModelPart();
}

ItemBase * PaletteItem::lowerConnectorLayerVisible(ItemBase * itemBase) {
	if (m_layerKin.count() == 0) return NULL;

	if ((itemBase != this) 
		&& this->isVisible() 
		&& (!this->hidden()) && (!this->inactive()) && (this->zValue() < itemBase->zValue())
		&& this->hasConnectors()) 
	{
		return this;
	}

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi == itemBase) continue;

		if (lkpi->isVisible() 
			&& (!lkpi->hidden()) && (!lkpi->inactive()) 
			&& (lkpi->zValue() < itemBase->zValue()) 
			&& lkpi->hasConnectors() ) 
		{
			return lkpi;
		}
	}

	return NULL;
}

void PaletteItem::resetID() {
	ItemBase::resetID();
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->resetID();
	}
}

void PaletteItem::slamZ(double z) {
	PaletteItemBase::slamZ(z);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->slamZ(z);
	}
}

void PaletteItem::resetImage(InfoGraphicsView * infoGraphicsView) {
	foreach (Connector * connector, modelPart()->connectors()) {
		connector->unprocess(this->viewIdentifier(), this->viewLayerID());
	}

	QString error;
	LayerAttributes layerAttributes;
	this->setUpImage(modelPart(), this->viewIdentifier(), infoGraphicsView->viewLayers(), this->viewLayerID(), this->viewLayerSpec(), true, layerAttributes, error);
	
	foreach (ItemBase * layerKin, m_layerKin) {
		resetKinImage(layerKin, infoGraphicsView);
	}
}

void PaletteItem::resetKinImage(ItemBase * layerKin, InfoGraphicsView * infoGraphicsView) 
{
	foreach (Connector * connector, modelPart()->connectors()) {
		connector->unprocess(layerKin->viewIdentifier(), layerKin->viewLayerID());
	}
	QString error;
	LayerAttributes layerAttributes;
	qobject_cast<PaletteItemBase *>(layerKin)->setUpImage(modelPart(), layerKin->viewIdentifier(), infoGraphicsView->viewLayers(), layerKin->viewLayerID(), layerKin->viewLayerSpec(), true, layerAttributes, error);
}

QString PaletteItem::genFZP(const QString & moduleid, const QString & templateName, int minPins, int maxPins, int steps, bool smd)
{
	QString FzpTemplate = "";
	QString ConnectorFzpTemplate = "";


	QFile file1(QString(":/resources/templates/%1.txt").arg(templateName));
	file1.open(QFile::ReadOnly);
	FzpTemplate = file1.readAll();
	file1.close();
	if (smd) {
		FzpTemplate.replace("<layer layerId=\"copper0\"/>", "");
	}

	QFile file2(":/resources/templates/generic_sip_connectorFzpTemplate.txt");
	file2.open(QFile::ReadOnly);
	ConnectorFzpTemplate = file2.readAll();
	file2.close();
	if (smd) {
		ConnectorFzpTemplate.replace("<p layer=\"copper0\" svgId=\"connector%1pin\"/>", "");
	}

	QStringList ss = moduleid.split("_");
	int count = 0;
	foreach (QString s, ss) {
		bool ok;
		int c = s.toInt(&ok);
		if (ok) {
			count = c;
			break;
		}
	}

	if (count > maxPins || count < minPins) return "";
	if (count % steps != 0) return "";

	QString middle;

	for (int i = 0; i < count; i++) {
		middle += ConnectorFzpTemplate.arg(i).arg(i + 1);
	}

	return FzpTemplate.arg(count).arg(middle);
}

bool PaletteItem::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	if (prop.compare("editable pin labels", Qt::CaseInsensitive) == 0 && value.compare("true") == 0) {
		returnProp = "";
		returnValue = value;

		QPushButton * button = new QPushButton(tr("Edit Pin Labels"));
		button->setObjectName("infoViewButton");
		connect(button, SIGNAL(pressed()), this, SLOT(openPinLabelDialog()));
		button->setEnabled(swappingEnabled);

		returnWidget = button;

		return true;
	}

	return PaletteItemBase::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void PaletteItem::openPinLabelDialog() {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView == NULL) {
		QMessageBox::warning(
			NULL,
			tr("Fritzing"),
			tr("Unable to proceed; unable to find top level view.")
		);		
		return;	
	}

	QStringList labels;
	QList<ConnectorItem *> sortedConnectorItems = sortConnectorItems();
	if (sortedConnectorItems.count() == 0) {
		QMessageBox::warning(
			NULL,
			tr("Fritzing"),
			tr("Unable to proceed; part connectors do no have standard IDs.")
		);
		return;
	}

	foreach (ConnectorItem * connectorItem, sortedConnectorItems) {
		labels.append(connectorItem->connectorSharedName());
	}

	QString chipLabel = modelPart()->localProp("chip label").toString();
	if (chipLabel.isEmpty()) {
		chipLabel = instanceTitle();
	}

	bool singleRow = isSingleRow(sortedConnectorItems);
	PinLabelDialog pinLabelDialog(labels, singleRow, chipLabel, modelPart()->isCore(), NULL);
	int result = pinLabelDialog.exec();
	if (result != QDialog::Accepted) return;

	QStringList newLabels = pinLabelDialog.labels();
	if (newLabels.count() != sortedConnectorItems.count()) {
		QMessageBox::warning(
			NULL,
			tr("Fritzing"),
			tr("Label mismatch.  Nothing was saved.")
		);	
		return;
	}

	infoGraphicsView->renamePins(this, labels, newLabels, singleRow);
}

void PaletteItem::renamePins(const QStringList & labels, bool singleRow)
{
	QList<ConnectorItem *> sortedConnectorItems = sortConnectorItems();
	for (int i = 0; i < labels.count(); i++) {
		ConnectorItem * connectorItem = sortedConnectorItems.at(i);
		connectorItem->setConnectorLocalName(labels.at(i));
	}

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	infoGraphicsView->changePinLabels(this, singleRow);
}

bool PaletteItem::isSingleRow(QList<ConnectorItem *> & connectorItems) {
	if (connectorItems.count() == 2) {
		// no way to tell? so default to double
		return false;
	}
	else if (connectorItems.count() % 2 == 0) {
		QPointF p = connectorItems.at(0)->sceneAdjustedTerminalPoint(NULL);
		double slope = 0;
		for (int i = 1; i < connectorItems.count(); i++) { 
			QPointF q = connectorItems.at(i)->sceneAdjustedTerminalPoint(NULL);
			if (p == q) continue;
			
			double newSlope = q.x() == p.x() ? std::numeric_limits<double>::max() : (q.y()  - p.y()) / (q.x() - p.x());
			if (i == 1) {
				slope = newSlope;
			}
			else {
				double d = qAbs(newSlope - slope);
				if (d != 0 && d / qMax(qAbs(slope), qAbs(newSlope)) > 0.01) {
					return false;
				}
			}
		}
	}

	return true;
}

QList<ConnectorItem *> PaletteItem::sortConnectorItems() {
	QList<ConnectorItem *> sortedConnectorItems(this->cachedConnectorItems());
	ByIDParseSuccessful = true;
	qSort(sortedConnectorItems.begin(), sortedConnectorItems.end(), byID);
	if (!ByIDParseSuccessful || sortedConnectorItems.count() == 0) {		
		sortedConnectorItems.clear();
	}

	return sortedConnectorItems;
}

bool PaletteItem::changePinLabels(bool singleRow, bool sip) {
	Q_UNUSED(singleRow);
	Q_UNUSED(sip);
	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return true;

	return false;
}

QStringList PaletteItem::getPinLabels(bool & hasLocal) {
	hasLocal = false;
	QStringList labels;
	QList<ConnectorItem *> sortedConnectorItems = sortConnectorItems();
	if (sortedConnectorItems.count() == 0) return labels;

	foreach (ConnectorItem * connectorItem, sortedConnectorItems) {
		labels.append(connectorItem->connectorSharedName());
		if (!connectorItem->connector()->connectorLocalName().isEmpty()) {
			hasLocal = true;
		}
	}

	return labels;
}

void PaletteItem::resetConnectors() {
	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return;

	QSizeF size = fsvgRenderer()->defaultSizeF();   // pixels
	QRectF viewBox = fsvgRenderer()->viewBoxF();
	foreach (ConnectorItem * connectorItem, cachedConnectorItems()) {
		SvgIdLayer * svgIdLayer = connectorItem->connector()->fullPinInfo(m_viewIdentifier, m_viewLayerID);
		if (svgIdLayer == NULL) continue;

		QRectF bounds = fsvgRenderer()->boundsOnElement(svgIdLayer->m_svgId);
		QPointF p(bounds.left() * size.width() / viewBox.width(), bounds.top() * size.height() / viewBox.height());
		QRectF r = connectorItem->rect();
		r.moveTo(p.x(), p.y());
		connectorItem->setRect(r);
	}
}


void PaletteItem::resetConnectors(ItemBase * otherLayer,FSvgRenderer * otherLayerRenderer)
{
	// there's only one connector
	foreach (Connector * connector, m_modelPart->connectors().values()) {
		if (connector == NULL) continue;

		connector->unprocess(m_viewIdentifier, m_viewLayerID);
		SvgIdLayer * svgIdLayer = connector->fullPinInfo(m_viewIdentifier, m_viewLayerID);
		if (svgIdLayer == NULL) continue;

		bool result = fsvgRenderer()->setUpConnector(svgIdLayer, false);
		if (!result) continue;

		resetConnector(this, svgIdLayer);
	}

	if (otherLayer) {
		foreach (Connector * connector, m_modelPart->connectors().values()) {
			if (connector == NULL) continue;

			connector->unprocess(m_viewIdentifier, otherLayer->viewLayerID());
			SvgIdLayer * svgIdLayer = connector->fullPinInfo(m_viewIdentifier, otherLayer->viewLayerID());
			if (svgIdLayer == NULL) continue;

			bool result = otherLayerRenderer->setUpConnector(svgIdLayer, false);
			if (!result) continue;

			resetConnector(otherLayer, svgIdLayer);
		}
	}


}

void PaletteItem::resetConnector(ItemBase * itemBase, SvgIdLayer * svgIdLayer) 
{
	foreach (ConnectorItem * connectorItem, itemBase->cachedConnectorItems()) {
		//DebugDialog::debug(QString("via set rect %1").arg(itemBase->viewIdentifier()), svgIdLayer->m_rect);

		connectorItem->setRect(svgIdLayer->m_rect);
		connectorItem->setTerminalPoint(svgIdLayer->m_point);
		connectorItem->setRadius(svgIdLayer->m_radius, svgIdLayer->m_strokeWidth);
		connectorItem->attachedMoved();
		break;
	}
}

void PaletteItem::setUpHoleSizes(QString & holeSize, QString & ringThickness, const QString & attribute, QHash<QString, QString> & holeSizes) {
	QFile file(":/resources/vias.xml");

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("failed loading properties %1 line:%2 col:%3").arg(errorStr).arg(errorLine).arg(errorColumn));
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) return;
	if (root.tagName() != "vias") return;

	QDomElement ve = root.firstChildElement("via");
	while (!ve.isNull()) {
		QString rt = ve.attribute("ringthickness");
		QString hs = ve.attribute("holesize");
		QString name = ve.attribute("name");
        QString ok = ve.attribute(attribute);
        if (ok.toInt() == 1) {
		    if (ve.attribute(attribute + "default").compare("yes") == 0) {
			    if (ringThickness.isEmpty()) {
				    ringThickness = rt;
			    }
			    if (holeSize.isEmpty()) {
				    holeSize = hs;
			    }
		    }
		    holeSizes.insert(name, QString("%1,%2").arg(hs).arg(rt));
        }
		ve = ve.nextSiblingElement("via");
	}
}

QWidget * PaletteItem::createHoleSettings(QWidget * parent, HoleSettings & holeSettings, bool swappingEnabled, const QString & currentHoleSize, bool advanced) {
    static const int RowHeight = 21;

    holeSettings.diameterEdit = NULL;
	holeSettings.thicknessEdit = NULL;
	holeSettings.mmRadioButton = NULL;
	holeSettings.inRadioButton = NULL;
	holeSettings.diameterValidator = NULL;
	holeSettings.thicknessValidator = NULL;

    QFrame * frame = new QFrame(parent);
	frame->setObjectName("infoViewPartFrame");

	QVBoxLayout * vBoxLayout = new QVBoxLayout(frame);
	vBoxLayout->setMargin(0);
	vBoxLayout->setContentsMargins(0, 0, 0, 0);
	vBoxLayout->setSpacing(0);

	holeSettings.sizesComboBox = new QComboBox(frame);
	holeSettings.sizesComboBox->setEditable(false);
	holeSettings.sizesComboBox->setObjectName("infoViewComboBox");
	holeSettings.sizesComboBox->addItems(holeSettings.holeSizes->keys());
	holeSettings.sizesComboBox->setEnabled(swappingEnabled);

	vBoxLayout->addWidget(holeSettings.sizesComboBox);

    if (advanced) {
        QFrame * hFrame = new QFrame(frame);
        QHBoxLayout * hLayout = new QHBoxLayout(hFrame);
	    hLayout->setMargin(0);

	    QGroupBox * subFrame = new QGroupBox(tr("advanced settings"), frame);
	    subFrame->setObjectName("infoViewGroupBox");

	    QGridLayout * gridLayout = new QGridLayout(subFrame);
	    gridLayout->setMargin(0);

	    QGroupBox * rbFrame = new QGroupBox("", subFrame);
	    rbFrame->setObjectName("infoViewGroupBox");
	    QVBoxLayout * vbl = new QVBoxLayout(rbFrame);
	    vbl->setMargin(0);

	    holeSettings.inRadioButton = new QRadioButton(tr("in"), subFrame);
	    gridLayout->addWidget(holeSettings.inRadioButton, 0, 2);
	    holeSettings.inRadioButton->setObjectName("infoViewRadioButton");

	    holeSettings.mmRadioButton = new QRadioButton(tr("mm"), subFrame);
	    gridLayout->addWidget(holeSettings.mmRadioButton, 1, 2);
	    holeSettings.inRadioButton->setObjectName("infoViewRadioButton");

	    vbl->addWidget(holeSettings.inRadioButton);
	    vbl->addWidget(holeSettings.mmRadioButton);
	    rbFrame->setLayout(vbl);

	    gridLayout->addWidget(rbFrame, 0, 2, 2, 1, Qt::AlignVCenter);

	    holeSettings.diameterEdit = new QLineEdit(subFrame);
	    holeSettings.diameterEdit->setMinimumHeight(RowHeight);
	    holeSettings.diameterValidator = new QDoubleValidator(holeSettings.diameterEdit);
	    holeSettings.diameterValidator->setNotation(QDoubleValidator::StandardNotation);
	    holeSettings.diameterEdit->setValidator(holeSettings.diameterValidator);
	    gridLayout->addWidget(holeSettings.diameterEdit, 0, 1);
	    holeSettings.diameterEdit->setObjectName("infoViewLineEdit");

	    QLabel * label = new QLabel(tr("Hole Diameter"));
	    label->setMinimumHeight(RowHeight);
	    label->setObjectName("infoViewLabel");
	    gridLayout->addWidget(label, 0, 0);

	    holeSettings.thicknessEdit = new QLineEdit(subFrame);
	    holeSettings.thicknessEdit->setMinimumHeight(RowHeight);
	    holeSettings.thicknessValidator = new QDoubleValidator(holeSettings.thicknessEdit);
	    holeSettings.thicknessValidator->setNotation(QDoubleValidator::StandardNotation);
	    holeSettings.thicknessEdit->setValidator(holeSettings.thicknessValidator);
	    gridLayout->addWidget(holeSettings.thicknessEdit, 1, 1);
	    holeSettings.thicknessEdit->setObjectName("infoViewLineEdit");

	    label = new QLabel(tr("Ring Thickness"));
	    label->setMinimumHeight(RowHeight);
	    gridLayout->addWidget(label, 1, 0);
	    label->setObjectName("infoViewLabel");

	    gridLayout->setContentsMargins(10, 2, 0, 2);
	    gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 3);
	    gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 3);

        hLayout->addWidget(subFrame);
        hLayout->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
        vBoxLayout->addWidget(hFrame);


	    holeSettings.mmRadioButton->setEnabled(swappingEnabled);
	    holeSettings.inRadioButton->setEnabled(swappingEnabled);
	    holeSettings.diameterEdit->setEnabled(swappingEnabled);
	    holeSettings.thicknessEdit->setEnabled(swappingEnabled);


	    if (currentHoleSize.contains("mm")) {
		    holeSettings.mmRadioButton->setChecked(true);
	    }
	    else {
		    holeSettings.inRadioButton->setChecked(true);
	    }
    }

	updateEditTexts(holeSettings);
	updateValidators(holeSettings);
	updateSizes(holeSettings);

	return frame;
}

void PaletteItem::updateEditTexts(HoleSettings & holeSettings) {
	if (holeSettings.diameterEdit == NULL) return;
	if (holeSettings.thicknessEdit == NULL) return;
	if (holeSettings.mmRadioButton == NULL) return;

	double hd = TextUtils::convertToInches(holeSettings.holeDiameter);
	double rt = TextUtils::convertToInches(holeSettings.ringThickness);

	QString newVal;
	if (holeSettings.currentUnits() == "in") {
		newVal = QString("%1,%2").arg(hd).arg(rt);
	}
	else {
		newVal = QString("%1,%2").arg(hd * 25.4).arg(rt * 25.4);
	}

	QStringList sizes = newVal.split(",");
	holeSettings.diameterEdit->setText(sizes.at(0));
	holeSettings.thicknessEdit->setText(sizes.at(1));
}

void PaletteItem::updateSizes(HoleSettings &  holeSettings) {
	if (holeSettings.sizesComboBox == NULL) return;

	int newIndex = -1;

	QPointF current(TextUtils::convertToInches(holeSettings.holeDiameter), TextUtils::convertToInches(holeSettings.ringThickness));
	for (int ix = 0; ix < holeSettings.sizesComboBox->count(); ix++) {
		QString key = holeSettings.sizesComboBox->itemText(ix);
		QString value = holeSettings.holeSizes->value(key, "");
		QStringList sizes;
		if (value.isEmpty()) {
			sizes = key.split(",");
		}
		else {
			sizes = value.split(",");
		}
		if (sizes.count() < 2) continue;

		QPointF p(TextUtils::convertToInches(sizes.at(0)), TextUtils::convertToInches(sizes.at(1)));
		if (p == current) {
			newIndex = ix;
			break;
		}
	}

	if (newIndex < 0) {
		QString newItem = holeSettings.holeDiameter + "," + holeSettings.ringThickness;
		holeSettings.sizesComboBox->addItem(newItem);
		newIndex = holeSettings.sizesComboBox->findText(newItem);

		holeSettings.holeSizes->insert(newItem, newItem);
	}

	// don't want to trigger another undo command
	bool wasBlocked = holeSettings.sizesComboBox->blockSignals(true);
	holeSettings.sizesComboBox->setCurrentIndex(newIndex);
	holeSettings.sizesComboBox->blockSignals(wasBlocked);
}

void PaletteItem::updateValidators(HoleSettings & holeSettings)
{
	if (holeSettings.diameterValidator == NULL) return;
	if (holeSettings.thicknessValidator == NULL) return;
	if (holeSettings.mmRadioButton == NULL) return;

	QString units = holeSettings.currentUnits();
	QPointF hdRange = holeSettings.holeDiameterRange(holeSettings.ringThickness);
	QPointF rtRange = holeSettings.ringThicknessRange(holeSettings.holeDiameter);

	double multiplier = (units == "mm") ? 25.4 : 1.0;
	holeSettings.diameterValidator->setRange(hdRange.x() * multiplier, hdRange.y() * multiplier, 3);
	holeSettings.thicknessValidator->setRange(rtRange.x() * multiplier, rtRange.y() * multiplier, 3);
}

void PaletteItem::initHoleSettings(HoleSettings & holeSettings, QHash<QString, QString> * holeSizes, RangeCalc holeDiameterRange,  RangeCalc ringThicknessRange) 
{
    holeSettings.holeSizes = holeSizes;
	holeSettings.diameterEdit = holeSettings.thicknessEdit = NULL;
	holeSettings.diameterValidator = holeSettings.thicknessValidator = NULL;
	holeSettings.inRadioButton = holeSettings.mmRadioButton = NULL;
	holeSettings.sizesComboBox = NULL;
	holeSettings.holeDiameterRange = holeDiameterRange;
	holeSettings.ringThicknessRange = ringThicknessRange;
}
