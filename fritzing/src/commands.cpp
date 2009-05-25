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


#include "commands.h"
#include "debugdialog.h"
#include "sketchwidget.h"
#include "waitpushundostack.h"
#include "items/wire.h"
#include "connectoritem.h"

int SelectItemCommand::selectItemCommandID = 3;
int ChangeLabelTextCommand::changeLabelTextCommandID = 4;
int BaseCommand::nextIndex = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseCommand::BaseCommand(BaseCommand::CrossViewType crossViewType, SketchWidget* sketchWidget, QUndoCommand *parent)
	: QUndoCommand(parent)
{
	m_crossViewType = crossViewType;
	m_sketchWidget = sketchWidget;
	m_parentCommand = parent;
	m_index = BaseCommand::nextIndex++;
}

BaseCommand::~BaseCommand() {
	foreach (BaseCommand * baseCommand, m_commands) {
		delete baseCommand;
	}
	m_commands.clear();
}

BaseCommand::CrossViewType BaseCommand::crossViewType() const {
	return m_crossViewType;
}

void BaseCommand::setCrossViewType(BaseCommand::CrossViewType crossViewType) {
	m_crossViewType = crossViewType;
}

SketchWidget* BaseCommand::sketchWidget() const {
	return m_sketchWidget;
}

QString BaseCommand::getDebugString() const {
	return QString("%1 %2").arg(getParamString()).arg(text());
}

QString BaseCommand::getParamString() const {
	return QString("%1 %2")
		.arg(m_sketchWidget->viewName())
		.arg((m_crossViewType == BaseCommand::SingleView) ? "single-view" : "cross-view");
}

int BaseCommand::subCommandCount() const {
	return m_commands.count();
}

const BaseCommand * BaseCommand::subCommand(int ix) const {
	if (ix < 0) return NULL;
	if (ix >= m_commands.count()) return NULL;

	return m_commands.at(ix);
}

void BaseCommand::addSubCommand(BaseCommand * subCommand) {
	m_commands.append(subCommand);
#ifndef QT_NO_DEBUG
	if (m_sketchWidget != NULL) {
		m_sketchWidget->undoStack()->writeUndo(subCommand, 4, this);
	}
#endif
}

const QUndoCommand * BaseCommand::parentCommand() const {
	return m_parentCommand;
}

void BaseCommand::subUndo() {
	for (int i = m_commands.count() - 1; i >= 0; i--) { 
		m_commands[i]->undo();
	}
}

void BaseCommand::subRedo() {
	foreach (BaseCommand * command, m_commands) {
		command->redo();
	}
}

void BaseCommand::subUndo(int index) {
	if (index < 0 || index >= m_commands.count()) return;

	m_commands[index]->undo();

}

void BaseCommand::subRedo(int index) {
	if (index < 0 || index >= m_commands.count()) return;

	m_commands[index]->redo();
}

int BaseCommand::index() const {
	return m_index;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddDeleteItemCommand::AddDeleteItemCommand(SketchWidget* sketchWidget, BaseCommand::CrossViewType crossViewType, QString moduleID, ViewGeometry & viewGeometry, qint64 id, long modelIndex, long originalModelIndex, QUndoCommand *parent)
    : BaseCommand(crossViewType, sketchWidget, parent)
{
 	m_dropOrigin = NULL;
    m_moduleID = moduleID;
    m_viewGeometry = viewGeometry;
    m_itemID = id;
	m_modelIndex = modelIndex;
	m_originalModelIndex = originalModelIndex;
}

QString AddDeleteItemCommand::getParamString() const {
	return BaseCommand::getParamString() + 
		QString(" moduleid:%1 id:%2 modelindex:%3")
		.arg(m_moduleID)
		.arg(m_itemID)
		.arg(m_modelIndex);
}

long AddDeleteItemCommand::itemID() const {
	return m_itemID;
}

void AddDeleteItemCommand::setDropOrigin(SketchWidget * sketchWidget) {
	m_dropOrigin = sketchWidget;
}

SketchWidget * AddDeleteItemCommand::dropOrigin() {
	return m_dropOrigin;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddItemCommand::AddItemCommand(SketchWidget* sketchWidget, BaseCommand::CrossViewType crossViewType, QString moduleID, ViewGeometry & viewGeometry, qint64 id, bool updateInfoView, long modelIndex, long originalModelIndex, QUndoCommand *parent)
    : AddDeleteItemCommand(sketchWidget, crossViewType, moduleID, viewGeometry, id, modelIndex, originalModelIndex, parent)
{
	m_doFirstRedo = m_firstRedo = true;
	m_module = false;
	m_updateInfoView = updateInfoView;
	m_restoreIndexesCommand = NULL;
}

void AddItemCommand::undo()
{
    m_sketchWidget->deleteItem(m_itemID, true, true, false, m_restoreIndexesCommand);
}

void AddItemCommand::redo()
{
	if (!m_firstRedo || m_doFirstRedo) {
		m_sketchWidget->addItem(m_moduleID, m_crossViewType, m_viewGeometry, m_itemID, m_modelIndex, m_originalModelIndex, this);
	}
	m_firstRedo = false;
}

void AddItemCommand::turnOffFirstRedo() {
	m_doFirstRedo = false;
}

void AddItemCommand::addRestoreIndexesCommand(RestoreIndexesCommand * restoreIndexesCommand) {
	m_restoreIndexesCommand = restoreIndexesCommand;
}

QString AddItemCommand::getParamString() const {
	return "AddItemCommand " + AddDeleteItemCommand::getParamString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DeleteItemCommand::DeleteItemCommand(SketchWidget* sketchWidget,BaseCommand::CrossViewType crossViewType,  QString moduleID, ViewGeometry & viewGeometry, qint64 id, long modelIndex, long originalModelIndex, QUndoCommand *parent)
    : AddDeleteItemCommand(sketchWidget, crossViewType, moduleID, viewGeometry, id, modelIndex, originalModelIndex, parent)
{
}

void DeleteItemCommand::undo()
{
    m_sketchWidget->addItem(m_moduleID, m_crossViewType, m_viewGeometry, m_itemID, m_modelIndex, m_originalModelIndex, this);
}

void DeleteItemCommand::redo()
{
	m_sketchWidget->deleteItem(m_itemID, true, m_crossViewType == BaseCommand::CrossView, false, NULL);
}

QString DeleteItemCommand::getParamString() const {
	return "DeleteItemCommand " + AddDeleteItemCommand::getParamString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MoveItemCommand::MoveItemCommand(SketchWidget* sketchWidget, long itemID, ViewGeometry & oldG, ViewGeometry & newG, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
    m_old = oldG;
    m_new = newG;
}

void MoveItemCommand::undo()
{
    m_sketchWidget->moveItem(m_itemID, m_old);
}

void MoveItemCommand::redo()
{
    m_sketchWidget->moveItem(m_itemID, m_new);
}

QString MoveItemCommand::getParamString() const {
	return QString("MoveItemCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" id:%1")
		.arg(m_itemID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RotateItemCommand::RotateItemCommand(SketchWidget* sketchWidget, long itemID, qreal degrees, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
    m_degrees = degrees;
}

void RotateItemCommand::undo()
{
    m_sketchWidget->rotateItem(m_itemID, -m_degrees);
}

void RotateItemCommand::redo()
{
    m_sketchWidget->rotateItem(m_itemID, m_degrees);
}

QString RotateItemCommand::getParamString() const {
	return QString("RotateItemCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" id:%1 by:%2")
		.arg(m_itemID)
		.arg(m_degrees);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FlipItemCommand::FlipItemCommand(SketchWidget* sketchWidget, long itemID, Qt::Orientations orientation, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
    m_orientation = orientation;
}

void FlipItemCommand::undo()
{
    redo();
}

void FlipItemCommand::redo()
{
    m_sketchWidget->flipItem(m_itemID, m_orientation);
}


QString FlipItemCommand::getParamString() const {
	return QString("FlipItemCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" id:%1 by:%2")
		.arg(m_itemID)
		.arg(m_orientation);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeConnectionCommand::ChangeConnectionCommand(SketchWidget * sketchWidget, BaseCommand::CrossViewType crossView,
												 long fromID, const QString & fromConnectorID,
												 long toID, const QString & toConnectorID,
												 bool connect, bool seekLayerKin,
												 QUndoCommand * parent)
: BaseCommand(crossView, sketchWidget, parent)
{
	//DebugDialog::debug(QString("ccc: from %1 %2; to %3 %4").arg(fromID).arg(fromConnectorID).arg(toID).arg(toConnectorID) );
    m_fromID = fromID;
    m_fromConnectorID = fromConnectorID;
    m_toID = toID;
    m_toConnectorID = toConnectorID;
	m_connect = connect;
	m_seekLayerKin = seekLayerKin;
	m_updateConnections = true;
}

void ChangeConnectionCommand::undo()
{
    m_sketchWidget->changeConnection(m_fromID, m_fromConnectorID, m_toID, m_toConnectorID, !m_connect, m_crossViewType == CrossView, m_seekLayerKin, m_updateConnections);
}

void ChangeConnectionCommand::redo()
{
    m_sketchWidget->changeConnection(m_fromID, m_fromConnectorID, m_toID, m_toConnectorID, m_connect, m_crossViewType == CrossView, m_seekLayerKin, m_updateConnections);
}

void ChangeConnectionCommand::setUpdateConnections(bool updatem) {
	m_updateConnections = updatem;
}


QString ChangeConnectionCommand::getParamString() const {
	return QString("ChangeConnectionCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" fromid:%1 connid:%2 toid:%3 connid:%4 connect:%5")
		.arg(m_fromID)
		.arg(m_fromConnectorID)
		.arg(m_toID)
		.arg(m_toConnectorID)
		.arg(m_connect);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeWireCommand::ChangeWireCommand(SketchWidget* sketchWidget, long fromID,
									 QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, bool useLine,
									 QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_fromID = fromID;
	m_oldLine = oldLine;
    m_newLine = newLine;
    m_oldPos = oldPos;
    m_newPos = newPos;
    m_useLine = useLine;
}

void ChangeWireCommand::undo()
{
    m_sketchWidget->changeWire(m_fromID, m_oldLine, m_oldPos, m_useLine);
}

void ChangeWireCommand::redo()
{
    m_sketchWidget->changeWire(m_fromID, m_newLine, m_newPos, m_useLine);
}

QString ChangeWireCommand::getParamString() const {
	return QString("ChangeWireCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" fromid:%1 oldp:%2,%3 newP:%4,%5 oldr:%7,%8,%9,%10 newr:%11,%12,%13,%14")
		.arg(m_fromID)
		.arg(m_oldPos.x()).arg(m_oldPos.y())
		.arg(m_newPos.x()).arg(m_newPos.y())
		.arg(m_oldLine.x1()).arg(m_oldLine.y1()).arg(m_oldLine.x2()).arg(m_oldLine.y2())		
		.arg(m_newLine.x1()).arg(m_newLine.y1()).arg(m_newLine.x2()).arg(m_newLine.y2())		
		;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SelectItemCommand::SelectItemCommand(SketchWidget* sketchWidget, SelectItemType type, QUndoCommand *parent)
    : BaseCommand(BaseCommand::CrossView, sketchWidget, parent)
{
	m_type = type;
	m_updated = false;
}

int SelectItemCommand::id() const {
	return selectItemCommandID;
}

void SelectItemCommand::setSelectItemType(SelectItemType type) {
	m_type = type;
}

void SelectItemCommand::copyUndo(SelectItemCommand * sother) {
   	this->m_undoIDs.clear();
   	for (int i = 0; i < sother->m_undoIDs.size(); i++) {
   		this->m_undoIDs.append(sother->m_undoIDs[i]);
  	}
}

void SelectItemCommand::clearRedo() {
	m_redoIDs.clear();
}

bool SelectItemCommand::mergeWith(const QUndoCommand *other)
{
	// "this" is earlier; "other" is later

    if (other->id() != id()) {
        return false;
   	}

    const SelectItemCommand * sother = dynamic_cast<const SelectItemCommand *>(other);
    if (sother == NULL) return false;

	if (sother->crossViewType() != this->crossViewType()) {
		return false;
	}

   	this->m_redoIDs.clear();
   	for (int i = 0; i < sother->m_redoIDs.size(); i++) {
   		this->m_redoIDs.append(sother->m_redoIDs[i]);
  	}

  	this->setText(sother->text());
    return true;
}

void SelectItemCommand::undo()
{
	selectAllFromStack(m_undoIDs, true, true);
}

void SelectItemCommand::redo()
{
	switch( m_type ){
		case NormalSelect:
			selectAllFromStack(m_redoIDs, true, true);
			break;
		case NormalDeselect:
			selectAllFromStack(m_redoIDs, false, false);
			break;
		case SelectAll: 
			m_sketchWidget->selectAllItems(true, m_crossViewType == BaseCommand::CrossView); 
			break;
		case DeselectAll: 
			m_sketchWidget->selectAllItems(false, m_crossViewType == BaseCommand::CrossView); 
			break;
	}
}

void SelectItemCommand::selectAllFromStack(QList<long> & stack, bool select, bool updateInfoView) {
	m_sketchWidget->clearSelection();
	for (int i = 0; i < stack.size(); i++) {
		m_sketchWidget->selectItem(stack[i], select, updateInfoView, m_crossViewType == BaseCommand::CrossView);
	}
}

void SelectItemCommand::addUndo(long id) {
	m_undoIDs.append(id);
}

void SelectItemCommand::addRedo(long id) {
	if(m_type == NormalSelect) {
		m_redoIDs.append(id);
	}
}

QString SelectItemCommand::getParamString() const {
	return QString("SelectItemCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" type:%1")
		.arg(m_type);
}

bool SelectItemCommand::updated() {
	return m_updated;
}


void SelectItemCommand::setUpdated(bool updated) {
	m_updated = updated;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeZCommand::ChangeZCommand(SketchWidget* sketchWidget, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
}

ChangeZCommand::~ChangeZCommand() {
	foreach (RealPair * realpair, m_triplets) {
		delete realpair;
	}
	m_triplets.clear();
}

void ChangeZCommand::addTriplet(long id, qreal oldZ, qreal newZ) {
	m_triplets.insert(id, new RealPair (oldZ, newZ));
}

void ChangeZCommand::undo()
{
   m_sketchWidget->changeZ(m_triplets, first);
}

void ChangeZCommand::redo()
{
   m_sketchWidget->changeZ(m_triplets, second);
}

qreal ChangeZCommand::first(RealPair * pair) {
	return pair->first;
}

qreal ChangeZCommand::second(RealPair * pair) {
	return pair->second;
}

QString ChangeZCommand::getParamString() const {
	return QString("ChangeZCommand ") 
		+ BaseCommand::getParamString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CheckStickyCommand::CheckStickyCommand(SketchWidget* sketchWidget, BaseCommand::CrossViewType crossViewType, long itemID, bool checkCurrent, QUndoCommand *parent)
: BaseCommand(crossViewType, sketchWidget, parent)
{
	m_itemID = itemID;
	m_firstTime = true;
	m_checkCurrent = checkCurrent;
}

CheckStickyCommand::~CheckStickyCommand() {
	foreach (StickyThing * stickyThing, m_stickyList) {
		delete stickyThing;
	}

	m_stickyList.clear();
}

void CheckStickyCommand::undo()
{
	foreach (StickyThing * stickyThing, m_stickyList) {
		stickyThing->sketchWidget->stickem(stickyThing->fromID, stickyThing->toID, !stickyThing->stickem);
	}
}

void CheckStickyCommand::redo()
{
	if (m_firstTime) {
		m_sketchWidget->checkSticky(m_itemID, m_crossViewType == BaseCommand::CrossView, m_checkCurrent, this);
		m_firstTime = false;
	}
	else {
		foreach (StickyThing * stickyThing, m_stickyList) {
			stickyThing->sketchWidget->stickem(stickyThing->fromID, stickyThing->toID, stickyThing->stickem);
		}
	}
}

QString CheckStickyCommand::getParamString() const {
	return QString("CheckStickyCommand ") 
		+ BaseCommand::getParamString()
		+ QString("id:%1") 
			.arg(this->m_stickyList.count());
}

void CheckStickyCommand::stick(SketchWidget * sketchWidget, long fromID, long toID, bool stickem) {
	m_firstTime = false;
	StickyThing * stickyThing = new StickyThing;
	stickyThing->sketchWidget = sketchWidget;
	stickyThing->fromID = fromID;
	stickyThing->toID = toID;
	stickyThing->stickem = stickem;
	m_stickyList.append(stickyThing);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CleanUpWiresCommand::CleanUpWiresCommand(SketchWidget* sketchWidget, bool skipMe, QUndoCommand *parent)
: BaseCommand(BaseCommand::CrossView, sketchWidget, parent)
{
	m_firstTime = true;
	m_skipMe = skipMe;
}

void CleanUpWiresCommand::undo()
{
	subUndo();
}

void CleanUpWiresCommand::redo()
{
	subRedo();

	m_sketchWidget->cleanUpWires(m_crossViewType == BaseCommand::CrossView, m_firstTime ? this : NULL, m_skipMe);
	m_firstTime = false;
}

void CleanUpWiresCommand::addWire(SketchWidget * sketchWidget, Wire * wire) 
{
	if (m_parentCommand) {
		for (int i = 0; i < m_parentCommand->childCount(); i++) {
			const DeleteItemCommand * command = dynamic_cast<const DeleteItemCommand *>(m_parentCommand->child(i));

			if (command == NULL) continue;
			if (command->itemID() == wire->id()) {
				return;
			}		
		}
	}

	addSubCommand(new WireColorChangeCommand(sketchWidget, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), NULL));
	addSubCommand(new WireWidthChangeCommand(sketchWidget, wire->id(), wire->width(), wire->width(), NULL));
	
	foreach (ConnectorItem * toConnectorItem, wire->connector0()->connectedToItems()) {	
		addSubCommand(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
				wire->id(), "connector0", false, true, NULL));
	}
	foreach (ConnectorItem * toConnectorItem, wire->connector1()->connectedToItems()) {	
		addSubCommand(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
				wire->id(), "connector1", false, true, NULL));
	}

	addSubCommand(new DeleteItemCommand(sketchWidget, BaseCommand::SingleView, ItemBase::wireModuleIDName, wire->getViewGeometry(), wire->id(), wire->modelPart()->modelIndex(), -1, NULL));
}


void CleanUpWiresCommand::addRoutingStatus(SketchWidget * sketchWidget, int oldNetCount, int oldNetRoutedCount, int oldConnectorsLeftToRoute, int oldJumpers,
										  int newNetCount, int newNetRoutedCount, int newConnectorsLeftToRoute, int newJumpers)
{
	addSubCommand(new RoutingStatusCommand(sketchWidget, oldNetCount, oldNetRoutedCount, oldConnectorsLeftToRoute,  oldJumpers,
										   newNetCount, newNetRoutedCount, newConnectorsLeftToRoute, newJumpers, NULL));
}

QString CleanUpWiresCommand::getParamString() const {
	return QString("CleanUpWiresCommand ") 
		+ BaseCommand::getParamString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WireColorChangeCommand::WireColorChangeCommand(SketchWidget* sketchWidget, long wireId, const QString &oldColor, const QString &newColor, qreal oldOpacity, qreal newOpacity, QUndoCommand *parent)
: BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
	m_wireId = wireId;
	m_oldColor = oldColor;
	m_newColor = newColor;
	m_oldOpacity = oldOpacity;
	m_newOpacity = newOpacity;
}

void WireColorChangeCommand::undo() {
	m_sketchWidget->changeWireColor(m_wireId, m_oldColor, m_oldOpacity);
}

void WireColorChangeCommand::redo() {
	m_sketchWidget->changeWireColor(m_wireId, m_newColor, m_newOpacity);
}

QString WireColorChangeCommand::getParamString() const {
	return QString("WireColorChangeCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 oldcolor:%2 oldop:%3 newcolor:%4 newop:%5") 
			.arg(m_wireId).arg(m_oldColor).arg(m_oldOpacity).arg(m_newColor).arg(m_newOpacity);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WireWidthChangeCommand::WireWidthChangeCommand(SketchWidget* sketchWidget, long wireId, int oldWidth, int newWidth, QUndoCommand *parent)
: BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
	m_wireId = wireId;
	m_oldWidth = oldWidth;
	m_newWidth = newWidth;
}

void WireWidthChangeCommand::undo() {
	m_sketchWidget->changeWireWidth(m_wireId, m_oldWidth);
}

void WireWidthChangeCommand::redo() {
	m_sketchWidget->changeWireWidth(m_wireId, m_newWidth);
}


QString WireWidthChangeCommand::getParamString() const {
	return QString("WireWidthChangeCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 oldw:%2 neww:%3") 
			.arg(m_wireId).arg(m_oldWidth).arg(m_newWidth);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WireFlagChangeCommand::WireFlagChangeCommand(SketchWidget* sketchWidget, long wireId, ViewGeometry::WireFlags oldFlags, ViewGeometry::WireFlags newFlags, QUndoCommand *parent)
: BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
	m_wireId = wireId;
	m_oldFlags = oldFlags;
	m_newFlags = newFlags;
}

void WireFlagChangeCommand::undo() {
	m_sketchWidget->changeWireFlags(m_wireId, m_oldFlags);
}

void WireFlagChangeCommand::redo() {
	m_sketchWidget->changeWireFlags(m_wireId, m_newFlags);
}


QString WireFlagChangeCommand::getParamString() const {
	return QString("WireFlagChangeCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 old:%2 new:%3") 
			.arg(m_wireId).arg(m_oldFlags).arg(m_newFlags);

}

//////////////////////////////////////////

RatsnestCommand::RatsnestCommand(class SketchWidget * sketchWidget, BaseCommand::CrossViewType crossViewType,
									long fromID, const QString & fromConnectorID,
									long toID, const QString & toConnectorID,
									bool connect, bool seekLayerKin,
									QUndoCommand * parent) 
: ChangeConnectionCommand(sketchWidget, crossViewType, fromID, fromConnectorID, toID, toConnectorID,
						connect, seekLayerKin, parent)
{
	m_firstTime = true;
}

void RatsnestCommand::undo() {
	subUndo();
}

void RatsnestCommand::redo() {
	if (m_firstTime) {
		m_firstTime = false;
		m_sketchWidget->dealWithRatsnest(m_fromID, m_fromConnectorID, m_toID, m_toConnectorID, m_connect, this, m_crossViewType == BaseCommand::CrossView);
	}
	else {
		subRedo();
	}
}

void RatsnestCommand::addWire(SketchWidget * sketchWidget, Wire * wire, ConnectorItem * source, ConnectorItem * dest, bool select) 
{
	addSubCommand(new AddItemCommand(sketchWidget, BaseCommand::SingleView, ItemBase::wireModuleIDName, wire->getViewGeometry(), wire->id(), true, -1, -1, NULL));
	addSubCommand(new WireColorChangeCommand(sketchWidget, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), NULL));
	addSubCommand(new WireWidthChangeCommand(sketchWidget, wire->id(), wire->width(), wire->width(), NULL));
	addSubCommand(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, source->attachedToID(), source->connectorSharedID(),
			wire->id(), "connector0", true, true, NULL));
	addSubCommand(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, dest->attachedToID(), dest->connectorSharedID(),
			wire->id(), "connector1", true, true, NULL));
	if (!select) {
		SelectItemCommand * sic = new SelectItemCommand(sketchWidget, SelectItemCommand::NormalDeselect, NULL);
		sic->addRedo(wire->id());
		addSubCommand(sic);
	}
	addSubCommand(new CheckStickyCommand(sketchWidget, BaseCommand::SingleView, wire->id(), false, NULL));

}

QString RatsnestCommand::getParamString() const {
	return QString("RatsnestCommand ") 
		+ BaseCommand::getParamString();

}

///////////////////////////////////////////

RoutingStatusCommand::RoutingStatusCommand(class SketchWidget * sketchWidget, int oldNetCount, int oldNetRoutedCount, int oldConnectorsLeftToRoute, int oldJumpers,
												int newNetCount, int newNetRoutedCount, int newConnectorsLeftToRoute, int newJumpers, QUndoCommand * parent)												
	: BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
	m_oldNetCount = oldNetCount;
	m_oldNetRoutedCount = oldNetRoutedCount;
	m_oldConnectorsLeftToRoute = oldConnectorsLeftToRoute;
	m_oldJumpers = oldJumpers;
	m_newNetCount = newNetCount;
	m_newNetRoutedCount = newNetRoutedCount;
	m_newConnectorsLeftToRoute = newConnectorsLeftToRoute;
	m_newJumpers = newJumpers;
}

void RoutingStatusCommand::undo() {
	m_sketchWidget->forwardRoutingStatusSignal(m_oldNetCount, m_oldNetRoutedCount, m_oldConnectorsLeftToRoute, m_oldJumpers);
}

void RoutingStatusCommand::redo() {
	m_sketchWidget->forwardRoutingStatusSignal(m_newNetCount, m_newNetRoutedCount, m_newConnectorsLeftToRoute, m_newJumpers);
}

QString RoutingStatusCommand::getParamString() const {
	return QString("RoutingStatusCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" oldnet:%1 oldnetrouted:%2 oldconnectors:%3 oldjumpers:%4 newnet:%51 newnetrouted:%6 newconnectors:%7 newjumpers:%8 ") 
			.arg(m_oldNetCount).arg(m_oldNetRoutedCount).arg(m_oldConnectorsLeftToRoute).arg(m_oldJumpers)
			.arg(m_newNetCount).arg(m_newNetRoutedCount).arg(m_newConnectorsLeftToRoute).arg(m_newJumpers);

}

///////////////////////////////////////////////

MoveLabelCommand::MoveLabelCommand(class SketchWidget *sketchWidget, long id, QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = id;
    m_oldPos = oldPos;
    m_newPos = newPos;
    m_oldOffset = oldOffset;
    m_newOffset = newOffset;
}

void MoveLabelCommand::undo()
{
    m_sketchWidget->movePartLabel(m_itemID, m_oldPos, m_oldOffset);
}

void MoveLabelCommand::redo()
{
    m_sketchWidget->movePartLabel(m_itemID, m_newPos, m_newOffset);
}


QString MoveLabelCommand::getParamString() const {
	return QString("MoveLabelCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1") 
			.arg(m_itemID);

}

///////////////////////////////////////////////

ChangeLabelTextCommand::ChangeLabelTextCommand(class SketchWidget *sketchWidget, long id, 
											   const QString & oldText, const QString & newText, 
											   QSizeF oldSize, QSizeF newSize, bool isLabel, QUndoCommand *parent)
	: BaseCommand(BaseCommand::CrossView, sketchWidget, parent)
{
    m_itemID = id;
    m_oldText = oldText;
    m_newText = newText;
	m_oldSize = oldSize;
	m_newSize = newSize;
	m_firstTime = true;
	m_isLabel = isLabel;
}

void ChangeLabelTextCommand::undo() {
	m_sketchWidget->setInstanceTitle(m_itemID, m_oldText, m_isLabel, false);
	if (m_oldSize != m_newSize) {
		m_sketchWidget->resizeNote(m_itemID, m_oldSize);
	}
}

void ChangeLabelTextCommand::redo() {
	if (m_firstTime) {
		m_firstTime = false;
		return;
	}

    m_sketchWidget->setInstanceTitle(m_itemID, m_newText, m_isLabel, false);
	if (m_oldSize != m_newSize) {
		m_sketchWidget->resizeNote(m_itemID, m_newSize);
	}

}

int ChangeLabelTextCommand::id() const {
	return changeLabelTextCommandID;
}

bool ChangeLabelTextCommand::mergeWith(const QUndoCommand *other)
{
	// "this" is earlier; "other" is later

    if (other->id() != id()) {
        return false;
   	}

    const ChangeLabelTextCommand * sother = dynamic_cast<const ChangeLabelTextCommand *>(other);
    if (sother == NULL) return false;

	if (sother->m_itemID != m_itemID) {
		// this is not the same label so don't merge
		return false;
	}

	if (m_isLabel != sother->m_isLabel) {
		return false;
	}

	m_newSize = sother->m_newSize;
	m_newText = sother->m_newText;
    return true;
}

QString ChangeLabelTextCommand::getParamString() const {
	return QString("ChangeLabelTextCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 old:%2 new:%3") 
			.arg(m_itemID).arg(m_oldText).arg(m_newText);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RotateFlipLabelCommand::RotateFlipLabelCommand(SketchWidget* sketchWidget, long itemID, qreal degrees, Qt::Orientations orientation, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
    m_degrees = degrees;
	m_orientation = orientation;
}

void RotateFlipLabelCommand::undo()
{
    m_sketchWidget->rotateFlipPartLabel(m_itemID, -m_degrees, m_orientation);
}

void RotateFlipLabelCommand::redo()
{
    m_sketchWidget->rotateFlipPartLabel(m_itemID, m_degrees, m_orientation);
}

QString RotateFlipLabelCommand::getParamString() const {
	return QString("RotateFlipLabelCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 degrees:%2 orientation:%3") 
			.arg(m_itemID).arg(m_degrees).arg(m_orientation);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ResizeNoteCommand::ResizeNoteCommand(SketchWidget* sketchWidget, long itemID, const QSizeF & oldSize, const QSizeF & newSize, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
    m_oldSize = oldSize;
	m_newSize = newSize;
}

void ResizeNoteCommand::undo()
{
    m_sketchWidget->resizeNote(m_itemID, m_oldSize);
}

void ResizeNoteCommand::redo()
{
    m_sketchWidget->resizeNote(m_itemID, m_newSize);
}

QString ResizeNoteCommand::getParamString() const {
	return QString("ResizeNoteCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 oldsz:%2 %3 newsz:%4 %5") 
			.arg(m_itemID).arg(m_oldSize.width()).arg(m_oldSize.height()).arg(m_newSize.width()).arg(m_newSize.height());

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GroupCommand::GroupCommand(SketchWidget* sketchWidget, const QString & moduleID, long itemID, const ViewGeometry & viewGeometry, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
	m_moduleID = moduleID;
	m_viewGeometry = viewGeometry;
}

void GroupCommand::undo()
{
	m_sketchWidget->deleteItem(m_itemID, true, m_crossViewType == BaseCommand::CrossView, false, NULL);
}

void GroupCommand::redo()
{
    m_sketchWidget->group(m_moduleID, m_itemID, m_itemIDs, m_viewGeometry, m_crossViewType == BaseCommand::CrossView);
}

void GroupCommand::addItemID(long itemID) {
	m_itemIDs.append(itemID);
}

QString GroupCommand::getParamString() const {
	return QString("GroupCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 moduleID:%2").arg(m_itemID).arg(m_moduleID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ModuleChangeConnectionCommand::ModuleChangeConnectionCommand(class SketchWidget * sketchWidget, BaseCommand::CrossViewType cv,
							long fromID, const QString & fromConnectorID,
							QList<long> & toIDs, const QString & toConnectorID, bool doRatsnest,
							bool connect, bool seekLayerKin,
							QUndoCommand * parent)
	: ChangeConnectionCommand(sketchWidget, cv, fromID, fromConnectorID, 0, toConnectorID, connect, seekLayerKin, parent)
{
	m_doRatsnest = doRatsnest;
	foreach (long ix, toIDs) {
		m_toIDs.append(ix);
	}
}

void ModuleChangeConnectionCommand::undo()
{
    m_sketchWidget->moduleChangeConnection(m_fromID, m_fromConnectorID, m_toIDs, m_toConnectorID, m_doRatsnest, !m_connect, m_crossViewType == CrossView, m_seekLayerKin, m_updateConnections);
}

void ModuleChangeConnectionCommand::redo()
{
    m_sketchWidget->moduleChangeConnection(m_fromID, m_fromConnectorID, m_toIDs, m_toConnectorID, m_doRatsnest, m_connect, m_crossViewType == CrossView, m_seekLayerKin, m_updateConnections);
}

QString ModuleChangeConnectionCommand::getParamString() const {
	return QString("ModuleChangeConnectionCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" fromid:%1 connid:%2 toid:%3 connid:%4 connect:%5")
		.arg(m_fromID)
		.arg(m_fromConnectorID)
		.arg(m_toID)
		.arg(m_toConnectorID)
		.arg(m_connect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RestoreIndexesCommand::RestoreIndexesCommand(SketchWidget * sketchWidget, long id, ModelPartTiny * modelPartTiny, bool addType, QUndoCommand * parent) 
	: BaseCommand(BaseCommand::CrossView, sketchWidget, parent)
{
	m_modelPartTiny = modelPartTiny;
	m_itemID = id;
	m_addType = addType;
}

void RestoreIndexesCommand::undo() 
{
	if (m_modelPartTiny  && !m_addType) {
		m_sketchWidget->restoreIndexes(m_itemID, m_modelPartTiny, true);		
	}
}

void RestoreIndexesCommand::redo() {
	if (m_modelPartTiny && m_addType) {
		m_sketchWidget->restoreIndexes(m_itemID, m_modelPartTiny, true);		
	}
}

struct ModelPartTiny * RestoreIndexesCommand::modelPartTiny() {
	return m_modelPartTiny;
}

void RestoreIndexesCommand::setModelPartTiny(ModelPartTiny * modelPartTiny) {
	m_modelPartTiny = modelPartTiny;
}

QString RestoreIndexesCommand::getParamString() const {
	return QString("RestoreIndexesCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" id:%1 5")
		.arg(m_itemID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ResizeBoardCommand::ResizeBoardCommand(SketchWidget * sketchWidget, long itemID, qreal oldWidth, qreal oldHeight, qreal newWidth, qreal newHeight, QUndoCommand * parent)
: BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
	m_itemID = itemID;
	m_oldWidth = oldWidth;
	m_newWidth = newWidth;
	m_oldHeight = oldHeight;
	m_newHeight = newHeight;
}

void ResizeBoardCommand::undo() {
	m_sketchWidget->resizeBoard(m_itemID, m_oldWidth, m_oldHeight);
}

void ResizeBoardCommand::redo() {
	m_sketchWidget->resizeBoard(m_itemID, m_newWidth, m_newHeight);
}

QString ResizeBoardCommand::getParamString() const {

	return QString("ResizeBoardCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" id:%1 ow:%2 oh:%3 nw:%4 nh:%5")
		.arg(m_itemID)
		.arg(m_oldWidth)
		.arg(m_oldHeight)
		.arg(m_newWidth)
		.arg(m_newHeight);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TransformItemCommand::TransformItemCommand(class SketchWidget *sketchWidget, long id, const QMatrix & oldMatrix, const QMatrix & newMatrix, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = id;
    m_oldMatrix = oldMatrix;
    m_newMatrix = newMatrix;
}

void TransformItemCommand::undo()
{
    m_sketchWidget->transformItem(m_itemID, m_oldMatrix);
}

void TransformItemCommand::redo()
{
    m_sketchWidget->transformItem(m_itemID, m_newMatrix);
}

QString TransformItemCommand::getParamString() const {
	return QString("TransformItemCommand ") 
		+ BaseCommand::getParamString() + 
		QString(" id:%1")
		.arg(m_itemID);
}
