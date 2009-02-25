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

int SelectItemCommand::selectItemCommandID = 3;
int ChangeLabelTextCommand::changeLabelTextCommandID = 4;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseCommand::BaseCommand(BaseCommand::CrossViewType crossViewType, SketchWidget* sketchWidget, QUndoCommand *parent)
	: QUndoCommand(parent)
{
	m_crossViewType = crossViewType;
	m_sketchWidget = sketchWidget;
	m_parentCommand = parent;
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

SketchWidget* BaseCommand::sketchWidget() {
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
}

const QUndoCommand * BaseCommand::parentCommand() const {
	return m_parentCommand;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddDeleteItemCommand::AddDeleteItemCommand(SketchWidget* sketchWidget, BaseCommand::CrossViewType crossViewType, QString moduleID, ViewGeometry & viewGeometry, qint64 id, long modelIndex, QUndoCommand *parent)
    : BaseCommand(crossViewType, sketchWidget, parent)
{
    m_moduleID = moduleID;
    m_viewGeometry = viewGeometry;
    m_itemID = id;
	m_modelIndex = modelIndex;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddItemCommand::AddItemCommand(SketchWidget* sketchWidget, BaseCommand::CrossViewType crossViewType, QString moduleID, ViewGeometry & viewGeometry, qint64 id, bool updateInfoView, long modelIndex, QUndoCommand *parent)
    : AddDeleteItemCommand(sketchWidget, crossViewType, moduleID, viewGeometry, id, modelIndex, parent)
{
	m_doFirstRedo = m_firstRedo = true;
	m_updateInfoView = updateInfoView;
}

void AddItemCommand::undo()
{
    m_sketchWidget->deleteItem(m_itemID, true, true);
}

void AddItemCommand::redo()
{
	if (!m_firstRedo || m_doFirstRedo) {
		m_sketchWidget->addItem(m_moduleID, m_crossViewType, m_viewGeometry, m_itemID, m_modelIndex);
		m_sketchWidget->selectItem(m_itemID,true,m_updateInfoView, m_crossViewType == BaseCommand::CrossView);
	}
	m_firstRedo = false;
}

void AddItemCommand::turnOffFirstRedo() {
	m_doFirstRedo = false;
}

QString AddItemCommand::getParamString() const {
	return "AddItemCommand " + AddDeleteItemCommand::getParamString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DeleteItemCommand::DeleteItemCommand(SketchWidget* sketchWidget,BaseCommand::CrossViewType crossViewType,  QString moduleID, ViewGeometry & viewGeometry, qint64 id, long modelIndex, QUndoCommand *parent)
    : AddDeleteItemCommand(sketchWidget, crossViewType, moduleID, viewGeometry, id, modelIndex, parent)
{
}

void DeleteItemCommand::undo()
{
    m_sketchWidget->addItem(m_moduleID, m_crossViewType, m_viewGeometry, m_itemID, m_modelIndex);
}

void DeleteItemCommand::redo()
{
    m_sketchWidget->deleteItem(m_itemID, true, true);
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
		QString(" fromid:%1")
		.arg(m_fromID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SelectItemCommand::SelectItemCommand(SketchWidget* sketchWidget, SelectItemType type, QUndoCommand *parent)
    : BaseCommand(BaseCommand::CrossView, sketchWidget, parent)
{
	m_type = type;
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

StickyCommand::StickyCommand(SketchWidget* sketchWidget, long stickTargetID, long stickSourceID, bool stick, QUndoCommand *parent)
: BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
	m_stickTargetID = stickTargetID;
	m_stickSourceID = stickSourceID;
	m_stick = stick;
}

void StickyCommand::undo()
{
	m_sketchWidget->stickem(m_stickTargetID, m_stickSourceID, !m_stick);
}

void StickyCommand::redo()
{
	m_sketchWidget->stickem(m_stickTargetID, m_stickSourceID, m_stick);
}

QString StickyCommand::getParamString() const {
	return QString("StickyCommand ") 
		+ BaseCommand::getParamString()
		+ QString("target:%1 src:%2 sticks:%3") 
			.arg(m_stickTargetID).arg(m_stickSourceID).arg(m_stick);
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
	for (int i = m_commands.count() - 1; i >= 0; i--) { 
		m_commands[i]->undo();
	}
}

void CleanUpWiresCommand::redo()
{
	foreach (BaseCommand * command, m_commands) {
		command->redo();
	}

	m_sketchWidget->cleanUpWires(true, m_firstTime ? this : NULL, m_skipMe);
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

	m_commands.append(new WireColorChangeCommand(sketchWidget, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), NULL));
	m_commands.append(new WireWidthChangeCommand(sketchWidget, wire->id(), wire->width(), wire->width(), NULL));
	
	foreach (ConnectorItem * toConnectorItem, wire->connector0()->connectedToItems()) {	
		m_commands.append(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
				wire->id(), "connector0", false, true, NULL));
	}
	foreach (ConnectorItem * toConnectorItem, wire->connector1()->connectedToItems()) {	
		m_commands.append(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
				wire->id(), "connector1", false, true, NULL));
	}

	m_commands.append(new DeleteItemCommand(sketchWidget, BaseCommand::SingleView, Wire::moduleIDName, wire->getViewGeometry(), wire->id(), wire->modelPart()->modelIndex(), NULL));
}


void CleanUpWiresCommand::addRoutingStatus(SketchWidget * sketchWidget, int oldNetCount, int oldNetRoutedCount, int oldConnectorsLeftToRoute, int oldJumpers,
										  int newNetCount, int newNetRoutedCount, int newConnectorsLeftToRoute, int newJumpers)
{
	m_commands.append(new RoutingStatusCommand(sketchWidget, oldNetCount, oldNetRoutedCount, oldConnectorsLeftToRoute,  oldJumpers,
										   newNetCount, newNetRoutedCount, newConnectorsLeftToRoute, newJumpers, NULL));
}

QString CleanUpWiresCommand::getParamString() const {
	return QString("CleanUpWiresCommand ") 
		+ BaseCommand::getParamString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SwapCommand::SwapCommand(SketchWidget* sketchWidget, long itemId, const QString &oldModID, const QString &newModID, QUndoCommand *parent)
: BaseCommand(BaseCommand::CrossView, sketchWidget, parent)
{
	m_itemId = itemId;
	m_oldModuleID = oldModID;
	m_newModuleID = newModID;
	m_firstTime = true;
}

void SwapCommand::undo() {
	m_sketchWidget->swap(m_itemId, m_oldModuleID, true, NULL);

	// reconnect everyone, if necessary
	for (int i = m_commands.count() - 1; i >= 0; i--) { 
		m_commands[i]->undo();
	}
}

void SwapCommand::redo() {
	// disconnect everyone, if necessary
	foreach (BaseCommand * command, m_commands) {
		command->redo();
	}

	m_sketchWidget->swap(m_itemId, m_newModuleID, true, m_firstTime ? this : NULL);
	m_firstTime = false;
}

void SwapCommand::addDisconnect(class ConnectorItem * from, class ConnectorItem * to) 
{
	ChangeConnectionCommand * ccc = new ChangeConnectionCommand(m_sketchWidget, BaseCommand::CrossView, from->attachedToID(), from->connectorSharedID(),
																to->attachedToID(), to->connectorSharedID(), false, true, NULL);
	m_commands.append(ccc);
	ccc->redo();
}

void SwapCommand::addAfterDisconnect() {
	CleanUpWiresCommand * cuw = new CleanUpWiresCommand(m_sketchWidget, false, NULL);
	m_commands.append(cuw);
	cuw->redo();
}


QString SwapCommand::getParamString() const {
	return QString("SwapCommand ") 
		+ BaseCommand::getParamString()
		+ QString(" id:%1 oldModule:%2 newModule:%3") 
			.arg(m_itemId).arg(m_oldModuleID).arg(m_newModuleID);
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
	for (int i = m_commands.count() - 1; i >= 0; i--) { 
		m_commands[i]->undo();
	}
}

void RatsnestCommand::redo() {
	if (m_firstTime) {
		m_firstTime = false;
		m_sketchWidget->dealWithRatsnest(m_fromID, m_fromConnectorID, m_toID, m_toConnectorID, m_connect, this, m_crossViewType == BaseCommand::CrossView);
	}
	else {
		foreach (BaseCommand * command, m_commands) {
			command->redo();
		}
	}
}

void RatsnestCommand::addWire(SketchWidget * sketchWidget, Wire * wire, ConnectorItem * source, ConnectorItem * dest, bool select) 
{
	m_commands.append(new AddItemCommand(sketchWidget, BaseCommand::SingleView, Wire::moduleIDName, wire->getViewGeometry(), wire->id(), true, -1, NULL));
	m_commands.append(new WireColorChangeCommand(sketchWidget, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), NULL));
	m_commands.append(new WireWidthChangeCommand(sketchWidget, wire->id(), wire->width(), wire->width(), NULL));
	m_commands.append(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, source->attachedToID(), source->connectorSharedID(),
			wire->id(), "connector0", true, true, NULL));
	m_commands.append(new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, dest->attachedToID(), dest->connectorSharedID(),
			wire->id(), "connector1", true, true, NULL));
	if (!select) {
		SelectItemCommand * sic = new SelectItemCommand(sketchWidget, SelectItemCommand::NormalDeselect, NULL);
		sic->addRedo(wire->id());
		m_commands.append(sic);
	}

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
											   QSizeF oldSize, QSizeF newSize, QUndoCommand *parent)
	: BaseCommand(BaseCommand::CrossView, sketchWidget, parent)
{
    m_itemID = id;
    m_oldText = oldText;
    m_newText = newText;
	m_oldSize = oldSize;
	m_newSize = newSize;
	m_firstTime = true;
}

void ChangeLabelTextCommand::undo() {
    m_sketchWidget->setInstanceTitle(m_itemID, m_oldText, false);
	if (m_oldSize != m_newSize) {
		m_sketchWidget->resizeNote(m_itemID, m_oldSize);
	}
}

void ChangeLabelTextCommand::redo() {
	if (m_firstTime) {
		m_firstTime = false;
		return;
	}

    m_sketchWidget->setInstanceTitle(m_itemID, m_newText, false);
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


GroupCommand::GroupCommand(SketchWidget* sketchWidget, long itemID, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
}

void GroupCommand::undo()
{
	m_sketchWidget->deleteItem(m_itemID, true, m_crossViewType == BaseCommand::CrossView);
}

void GroupCommand::redo()
{
    m_sketchWidget->group(m_itemID, m_itemIDs, true);
}

void GroupCommand::addItemID(long itemID) {
	m_itemIDs.append(itemID);
}


SetConnectorExternalCommand::SetConnectorExternalCommand(SketchWidget* sketchWidget, long itemID, const QString & connectorID, bool external, QUndoCommand *parent)
    : BaseCommand(BaseCommand::SingleView, sketchWidget, parent)
{
    m_itemID = itemID;
	m_external = external;
	m_connectorID = connectorID;
}

void SetConnectorExternalCommand::undo()
{
    m_sketchWidget->setConnectorExternal(m_itemID, m_connectorID, !m_external);
}

void SetConnectorExternalCommand::redo()
{
    m_sketchWidget->setConnectorExternal(m_itemID, m_connectorID, m_external);
}


