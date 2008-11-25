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



#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QHash>

#include "viewgeometry.h"
#include "misc.h"


class BaseCommand : public QUndoCommand
{
public:
	enum CrossViewType {
		SingleView,
		CrossView
	};

public:
	BaseCommand(BaseCommand::CrossViewType, class SketchWidget*, QUndoCommand* parent = 0);
	BaseCommand::CrossViewType crossViewType();
	class SketchWidget* sketchWidget();

protected:
	BaseCommand::CrossViewType m_crossViewType;
    class SketchWidget *m_sketchWidget;
};

class AddDeleteItemCommand : public BaseCommand
{
public:
    AddDeleteItemCommand(class SketchWidget * sketchWidget, BaseCommand::CrossViewType, QString moduleID, ViewGeometry &, qint64 id, QUndoCommand *parent = 0);

protected:
    QString m_moduleID;
    long m_itemID;
    ViewGeometry m_viewGeometry;

};

class AddItemCommand : public AddDeleteItemCommand
{
public:
    AddItemCommand(class SketchWidget *sketchWidget, BaseCommand::CrossViewType, QString moduleID, ViewGeometry &, qint64 id, QUndoCommand *parent = 0, bool updateInfoView=true);
    void undo();
    void redo();

protected:
	bool m_updateInfoView;

};

class DeleteItemCommand : public AddDeleteItemCommand
{
public:
    DeleteItemCommand(class SketchWidget *sketchWidget, BaseCommand::CrossViewType, QString moduleID, ViewGeometry &, qint64 id, QUndoCommand *parent = 0);
    void undo();
    void redo();

};

class MoveItemCommand : public BaseCommand
{
public:
    MoveItemCommand(class SketchWidget *sketchWidget, long id, ViewGeometry & oldG, ViewGeometry & newG, QUndoCommand *parent = 0);
    void undo();
    void redo();

protected:
    long m_itemID;
    ViewGeometry m_old;
    ViewGeometry m_new;
};

class RotateItemCommand : public BaseCommand
{
public:
    RotateItemCommand(class SketchWidget *sketchWidget, long id, qreal degrees, QUndoCommand *parent = 0);
    void undo();
    void redo();

protected:
    long m_itemID;
    qreal m_degrees;
};

class FlipItemCommand : public BaseCommand
{

public:
    FlipItemCommand(class SketchWidget *sketchWidget, long id, Qt::Orientations orientation, QUndoCommand *parent = 0);
    void undo();
    void redo();

protected:
    long m_itemID;
    Qt::Orientations m_orientation;
};

class ChangeConnectionCommand : public BaseCommand
{
public:
	ChangeConnectionCommand(class SketchWidget * sketchWidget, BaseCommand::CrossViewType,
							long fromID, const QString & fromConnectorID,
							long toID, const QString & toConnectorID,
							bool connect, bool seekLayerKin,
							bool chain, QUndoCommand * parent);
	void undo();
	void redo();

protected:
    long m_fromID;
    long m_toID;
    bool m_seekLayerKin;
    QString m_fromConnectorID;
    QString m_toConnectorID;
	bool m_connect;
	bool m_chain;

};

class ChangeWireCommand : public BaseCommand
{
public:
    ChangeWireCommand(class SketchWidget *sketchWidget, long fromID,
    					QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos,
    					bool useLine,
    					QUndoCommand *parent = 0);
    void undo();
    void redo();

protected:
    long m_fromID;
    QLineF m_newLine;
    QLineF m_oldLine;
    QPointF m_newPos;
    QPointF m_oldPos;
    bool m_useLine;
};


class SelectItemCommand : public BaseCommand
{
public:
	enum SelectItemType {
		NormalSelect,
		SelectAll,
		DeselectAll
	};

public:
    SelectItemCommand(class SketchWidget *sketchWidget, SelectItemType type = NormalSelect, QUndoCommand *parent = 0);

    void undo();
    void redo();
    void addUndo(long id);
    void addRedo(long id);
	void clearRedo();
    int id() const;
	bool mergeWith(const QUndoCommand *other);
	void copyUndo(SelectItemCommand * sother);
	void setSelectItemType(SelectItemType);

protected:
	void selectAllFromStack(QList<long> & stack);

    QList<long> m_undoIDs;
    QList<long> m_redoIDs;
    SelectItemType m_type;

    static int selectItemCommandID;
};


class ChangeZCommand : public BaseCommand
{
public:
    ChangeZCommand(class SketchWidget *sketchWidget, QUndoCommand *parent = 0);
    void addTriplet(long id, qreal oldZ, qreal newZ);
    void undo();
    void redo();

    static qreal first(RealPair *);
    static qreal second(RealPair *);

protected:
    QHash<long, RealPair *> m_triplets;
};


class StickyCommand : public BaseCommand
{
public:
	StickyCommand(class SketchWidget *sketchWidget, long stickTargetID, long stickSourceID, bool stick, QUndoCommand *parent = 0);
    void undo();
    void redo();

protected:
	long m_stickTargetID;
	long m_stickSourceID;
	bool m_stick;
};

class CleanUpWiresCommand : public BaseCommand
{
public:
	CleanUpWiresCommand(class SketchWidget * sketchWidget, bool execRedo, QUndoCommand * parent = 0);
    void undo();
    void redo();

protected:
	bool m_execRedo;
};

class SwapCommand : public BaseCommand
{
public:
	SwapCommand(
		SketchWidget* sketchWidget,
		long itemId, const QString &oldModID, const QString &newModID,
		QUndoCommand *parent=0);
    void undo();
    void redo();

protected:
	long m_itemId;
	QString m_oldModuleID;
	QString m_newModuleID;
};

class WireColorChangeCommand : public BaseCommand
{
public:
	WireColorChangeCommand(
		SketchWidget* sketchWidget,
		long wireId, const QString &oldColor, const QString &newColor,
		qreal oldOpacity, qreal newOpacity,
		QUndoCommand *parent=0);
    void undo();
    void redo();

protected:
	long m_wireId;
	QString m_oldColor;
	QString m_newColor;
	qreal m_oldOpacity;
	qreal m_newOpacity;
};


class WireWidthChangeCommand : public BaseCommand
{
public:
	WireWidthChangeCommand(
		SketchWidget* sketchWidget,
		long wireId, int oldWidth, int newWidth,
		QUndoCommand *parent=0);
    void undo();
    void redo();

protected:
	long m_wireId;
	int m_oldWidth;
	int m_newWidth;
};


class WireFlagChangeCommand : public BaseCommand
{
public:
	WireFlagChangeCommand(
		SketchWidget* sketchWidget,
		long wireId, ViewGeometry::WireFlags oldFlags, ViewGeometry::WireFlags newFlags,
		QUndoCommand *parent=0);
    void undo();
    void redo();

protected:
	long m_wireId;
	ViewGeometry::WireFlags m_oldFlags;
	ViewGeometry::WireFlags m_newFlags;
};


#endif // COMMANDS_H
