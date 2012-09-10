/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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


#include "pecommands.h"
#include "pemainwindow.h"
#include "../debugdialog.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PEBaseCommand::PEBaseCommand(PEMainWindow * peMainWindow, QUndoCommand *parent)
	: BaseCommand(BaseCommand::CrossView, NULL, parent)
{
	m_peMainWindow = peMainWindow;
}

PEBaseCommand::~PEBaseCommand()
{
}

QString PEBaseCommand::getParamString() const {
    return "";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeMetadataCommand::ChangeMetadataCommand(PEMainWindow * peMainWindow, const QString & name, const QString  & oldValue, const QString & newValue, QUndoCommand *parent)
    : PEBaseCommand(peMainWindow, parent)
{
 	m_name = name;
	m_oldValue = oldValue;
	m_newValue = newValue;
}

void ChangeMetadataCommand::undo()
{
    m_peMainWindow->changeMetadata(m_name, m_oldValue, true);
}

void ChangeMetadataCommand::redo()
{
    if (m_skipFirstRedo) {
        m_skipFirstRedo = false;
    }
    else {
        m_peMainWindow->changeMetadata(m_name, m_newValue, true);
    }
}

QString ChangeMetadataCommand::getParamString() const {
	return "ChangeMetadataCommand " + 
        QString(" name:%1, old:%2, new:%3")
            .arg(m_name)
            .arg(m_oldValue)
            .arg(m_newValue)
        ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeTagsCommand::ChangeTagsCommand(PEMainWindow * peMainWindow, const QStringList  & oldTags, const QStringList & newTags, QUndoCommand *parent)
    : PEBaseCommand(peMainWindow, parent)
{
	m_old = oldTags;
	m_new = newTags;
}

void ChangeTagsCommand::undo()
{
    m_peMainWindow->changeTags(m_old, true);
}

void ChangeTagsCommand::redo()
{
    if (m_skipFirstRedo) {
        m_skipFirstRedo = false;
    }
    else {
        m_peMainWindow->changeTags(m_new, true);
    }
}

QString ChangeTagsCommand::getParamString() const {
	return "ChangeTagsCommand " + 
        QString(" old:%1, new:%2")
            .arg(m_old.count())
            .arg(m_new.count())
        ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangePropertiesCommand::ChangePropertiesCommand(PEMainWindow * peMainWindow, const QHash<QString, QString> & oldProps, const QHash<QString, QString> & newProps, QUndoCommand *parent)
    : PEBaseCommand(peMainWindow, parent)
{
	m_old = oldProps;
	m_new = newProps;
}

void ChangePropertiesCommand::undo()
{
    m_peMainWindow->changeProperties(m_old, true);
}

void ChangePropertiesCommand::redo()
{
    if (m_skipFirstRedo) {
        m_skipFirstRedo = false;
    }
    else {
        m_peMainWindow->changeProperties(m_new, true);
    }
}

QString ChangePropertiesCommand::getParamString() const {
	return "ChangePropertiesCommand " + 
        QString(" old:%1, new:%2")
            .arg(m_old.count())
            .arg(m_new.count())
        ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeConnectorMetadataCommand::ChangeConnectorMetadataCommand(PEMainWindow * peMainWindow, const ConnectorMetadata  & oldValue, const ConnectorMetadata & newValue, QUndoCommand *parent)
    : PEBaseCommand(peMainWindow, parent)
{
	m_oldcm = oldValue;
	m_newcm = newValue;
}

void ChangeConnectorMetadataCommand::undo()
{
    m_peMainWindow->changeConnectorMetadata(m_oldcm, true);
}

void ChangeConnectorMetadataCommand::redo()
{
    if (m_skipFirstRedo) {
        m_skipFirstRedo = false;
    }
    else {
        m_peMainWindow->changeConnectorMetadata(m_newcm, true);
    }
}

QString ChangeConnectorMetadataCommand::getParamString() const {
	return "ChangeConnectorMetadataCommand " + 
        QString(" name:%1, old:%2, new:%3")
            .arg(m_oldcm.connectorName)
            .arg(m_newcm.connectorName)
        ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeSvgCommand::ChangeSvgCommand(PEMainWindow * peMainWindow, SketchWidget * sketchWidget, const QString  & oldFilename, const QString & newFilename, QUndoCommand *parent)
    : PEBaseCommand(peMainWindow, parent)
{
 	m_sketchWidget = sketchWidget;
	m_oldFilename = oldFilename;
	m_newFilename = newFilename;
}

void ChangeSvgCommand::undo()
{
    m_peMainWindow->changeSvg(m_sketchWidget, m_oldFilename, -1);
}

void ChangeSvgCommand::redo()
{
    m_peMainWindow->changeSvg(m_sketchWidget, m_newFilename, 1);
}

QString ChangeSvgCommand::getParamString() const {
	return "ChangeSvgCommand " + 
        QString(" id:%1, old:%2, new:%3")
            .arg(m_sketchWidget->viewIdentifier())
            .arg(m_oldFilename)
            .arg(m_newFilename)
        ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RelocateConnectorSvgCommand::RelocateConnectorSvgCommand(PEMainWindow * peMainWindow, SketchWidget * sketchWidget, const QString  & id, const QString & terminalID, 
        const QString & oldGorn, const QString & oldGornTerminal, const QString & newGorn, const QString & newGornTerminal, 
        QUndoCommand *parent)
    : PEBaseCommand(peMainWindow, parent)
{
 	m_sketchWidget = sketchWidget;
	m_id = id;
	m_terminalID = terminalID;
	m_oldGorn = oldGorn;
	m_oldGornTerminal = oldGornTerminal;
	m_newGorn = newGorn;
	m_newGornTerminal = newGornTerminal;
}

void RelocateConnectorSvgCommand::undo()
{
    m_peMainWindow->relocateConnectorSvg(m_sketchWidget, m_id, m_terminalID, m_newGorn, m_newGornTerminal, m_oldGorn, m_oldGornTerminal, -1);
}

void RelocateConnectorSvgCommand::redo()
{
    m_peMainWindow->relocateConnectorSvg(m_sketchWidget, m_id, m_terminalID, m_oldGorn, m_oldGornTerminal, m_newGorn, m_newGornTerminal, 1);
}

QString RelocateConnectorSvgCommand::getParamString() const {
	return "RelocateConnectorSvgCommand " + 
        QString(" id:%1, terminalid:%2, oldgorn:%3, oldgornterminal:%4, newgorn:%5, newgornterminal:%6")
            .arg(m_sketchWidget->viewIdentifier())
            .arg(m_id)
            .arg(m_terminalID)
            .arg(m_oldGorn)
            .arg(m_oldGornTerminal)
            .arg(m_newGorn)
            .arg(m_newGornTerminal)
        ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
