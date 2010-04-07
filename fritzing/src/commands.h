/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify\
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
#include <QPainterPath>

#include "viewgeometry.h"
#include "utils/misc.h"

class BaseCommand : public QUndoCommand
{
public:
	enum CrossViewType {
		SingleView,
		CrossView
	};

public:
	BaseCommand(BaseCommand::CrossViewType, class SketchWidget*, QUndoCommand* parent);
	~BaseCommand();

	BaseCommand::CrossViewType crossViewType() const;
	void setCrossViewType(BaseCommand::CrossViewType);
	class SketchWidget* sketchWidget() const;
	int subCommandCount() const;
	const BaseCommand * subCommand(int index) const;
	virtual QString getDebugString() const;
	const QUndoCommand * parentCommand() const;
	void addSubCommand(BaseCommand * subCommand);
	void subUndo();
	void subRedo();
	void subUndo(int index);
	void subRedo(int index);
	int index() const;

protected:
	virtual QString getParamString() const;

	static int nextIndex;

protected:
	BaseCommand::CrossViewType m_crossViewType;
    class SketchWidget *m_sketchWidget;
	QList<BaseCommand *> m_commands;
	QUndoCommand * m_parentCommand;
	int m_index;
};

class AddDeleteItemCommand : public BaseCommand
{
public:
    AddDeleteItemCommand(class SketchWidget * sketchWidget, BaseCommand::CrossViewType, QString moduleID, bool flippedSMD, ViewGeometry &, qint64 id, long modelIndex, QUndoCommand *parent);

	long itemID() const;
	void setDropOrigin(class SketchWidget *);
	class SketchWidget * dropOrigin();

protected:
	QString getParamString() const;

protected:
    QString m_moduleID;
    long m_itemID;
    ViewGeometry m_viewGeometry;
	long m_modelIndex;
	class SketchWidget * m_dropOrigin;
	bool m_flippedSMD;
};

class AddItemCommand : public AddDeleteItemCommand
{
public:
    AddItemCommand(class SketchWidget *sketchWidget, BaseCommand::CrossViewType, QString moduleID, bool flippedSMD, ViewGeometry &, qint64 id, bool updateInfoView, long modelIndex, QUndoCommand *parent);
    void undo();
    void redo();
	void turnOffFirstRedo();
	void addRestoreIndexesCommand(class RestoreIndexesCommand *);

protected:
	QString getParamString() const;

protected:
	bool m_updateInfoView;
	bool m_firstRedo;
	bool m_doFirstRedo;
	bool m_module;
	RestoreIndexesCommand * m_restoreIndexesCommand;
};

class DeleteItemCommand : public AddDeleteItemCommand
{
public:
    DeleteItemCommand(class SketchWidget *sketchWidget, BaseCommand::CrossViewType, QString moduleID, bool flippedSMD, ViewGeometry &, qint64 id, long modelIndex, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

};

class MoveItemCommand : public BaseCommand
{
public:
    MoveItemCommand(class SketchWidget *sketchWidget, long id, ViewGeometry & oldG, ViewGeometry & newG, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    ViewGeometry m_old;
    ViewGeometry m_new;
};

class RotateItemCommand : public BaseCommand
{
public:
    RotateItemCommand(class SketchWidget *sketchWidget, long id, qreal degrees, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	virtual QString getParamString() const;

protected:
    long m_itemID;
    qreal m_degrees;
};

class FlipItemCommand : public BaseCommand
{

public:
    FlipItemCommand(class SketchWidget *sketchWidget, long id, Qt::Orientations orientation, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    Qt::Orientations m_orientation;
};

class TransformItemCommand : public BaseCommand
{

public:
    TransformItemCommand(class SketchWidget *sketchWidget, long id, const class QMatrix & oldMatrix, const class QMatrix & newMatrix, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    QMatrix m_oldMatrix;
    QMatrix m_newMatrix;
};

class ChangeConnectionCommand : public BaseCommand
{
public:
	ChangeConnectionCommand(class SketchWidget * sketchWidget, BaseCommand::CrossViewType,
							long fromID, const QString & fromConnectorID,
							long toID, const QString & toConnectorID,
							bool connect, bool seekLayerKin,
							QUndoCommand * parent);
	void undo();
	void redo();
	void setUpdateConnections(bool updatem);

protected:
	QString getParamString() const;

protected:
    long m_fromID;
    long m_toID;
    bool m_seekLayerKin;
    QString m_fromConnectorID;
    QString m_toConnectorID;
	bool m_connect;
	bool m_updateConnections;

};

class ChangeWireCommand : public BaseCommand
{
public:
    ChangeWireCommand(class SketchWidget *sketchWidget, long fromID,
    					QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos,
    					bool useLine,
    					QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

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
		NormalDeselect,
		SelectAll,
		DeselectAll
	};

public:
    SelectItemCommand(class SketchWidget *sketchWidget, SelectItemType type, QUndoCommand *parent);

    void undo();
    void redo();
    void addUndo(long id);
    void addRedo(long id);
	void clearRedo();
    int id() const;
	bool mergeWith(const QUndoCommand *other);
	void copyUndo(SelectItemCommand * sother);
	void setSelectItemType(SelectItemType);
	bool updated();
	void setUpdated(bool);

protected:
	void selectAllFromStack(QList<long> & stack, bool select, bool updateInfoView);
	QString getParamString() const;

protected:
    QList<long> m_undoIDs;
    QList<long> m_redoIDs;
    SelectItemType m_type;
	bool m_updated;

protected:
    static int selectItemCommandID;
};


class ChangeZCommand : public BaseCommand
{
public:
    ChangeZCommand(class SketchWidget *sketchWidget, QUndoCommand *parent);
	~ChangeZCommand();

    void addTriplet(long id, qreal oldZ, qreal newZ);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    static qreal first(RealPair *);
    static qreal second(RealPair *);

protected:
    QHash<long, RealPair *> m_triplets;
};

struct StickyThing {
	SketchWidget * sketchWidget;
	bool stickem;
	long fromID;
	long toID;
};

class CheckStickyCommand : public BaseCommand
{
public:
	CheckStickyCommand(class SketchWidget *sketchWidget, BaseCommand::CrossViewType, long itemID, bool checkCurrent, QUndoCommand *parent);
	~CheckStickyCommand();
	
	void undo();
    void redo();
	void stick(SketchWidget *, long fromID, long toID, bool stickem);

protected:
	QString getParamString() const;

protected:
	long m_itemID;
	QList<StickyThing *> m_stickyList;
	bool m_firstTime;
	bool m_checkCurrent;
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
	QString getParamString() const;

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
		long wireId, qreal oldWidth, qreal newWidth,
		QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
	long m_wireId;
	qreal m_oldWidth;
	qreal m_newWidth;
};


class WireFlagChangeCommand : public BaseCommand
{
public:
	WireFlagChangeCommand(
		SketchWidget* sketchWidget,
		long wireId, ViewGeometry::WireFlags oldFlags, ViewGeometry::WireFlags newFlags,
		QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
	long m_wireId;
	ViewGeometry::WireFlags m_oldFlags;
	ViewGeometry::WireFlags m_newFlags;
};

class RatsnestCommand : public ChangeConnectionCommand
{
public:
	RatsnestCommand(class SketchWidget * sketchWidget, BaseCommand::CrossViewType,
					long fromID, const QString & fromConnectorID,
					long toID, const QString & toConnectorID,
					bool connect, bool seekLayerKin,
					QUndoCommand * parent);
    void undo();
    void redo();
	void addWire(class SketchWidget *, class Wire *, class ConnectorItem * source, class ConnectorItem * dest, bool select);

protected:
	QString getParamString() const;

protected:
	bool m_firstTime;

};

struct RoutingStatus {
	int m_netCount;
	int m_netRoutedCount;
	int m_connectorsLeftToRoute;
	int m_jumperWireCount;
	int m_jumperItemCount;

public:
	void zero() {
		m_netCount = m_netRoutedCount = m_connectorsLeftToRoute = m_jumperWireCount = m_jumperItemCount = 0;
	}

	bool operator!=(const RoutingStatus &other) const {
		return 
			(m_netCount != other.m_netCount) ||
			(m_netRoutedCount != other.m_netRoutedCount) ||
			(m_connectorsLeftToRoute != other.m_connectorsLeftToRoute) ||
			(m_jumperWireCount != other.m_jumperWireCount) ||
			(m_jumperItemCount != other.m_jumperItemCount);
	}
};

class RoutingStatusCommand : public BaseCommand 
{
public:
	RoutingStatusCommand(class SketchWidget *, const RoutingStatus & oldRoutingStatus, const RoutingStatus & newRoutingStatus, QUndoCommand * parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
	RoutingStatus m_oldRoutingStatus;
	RoutingStatus m_newRoutingStatus;
};

class CleanUpWiresCommand : public BaseCommand
{
public:
	CleanUpWiresCommand(class SketchWidget * sketchWidget, bool skipMe, QUndoCommand * parent);
    void undo();
    void redo();
	void addWire(SketchWidget *, class Wire *);
	void addRoutingStatus(SketchWidget *, const RoutingStatus & oldRoutingStatus, const RoutingStatus & newRoutingStatus);

protected:
	QString getParamString() const;

protected:

	bool m_firstTime;
	bool m_skipMe;
};

class RestoreLabelCommand : public BaseCommand
{
public:
    RestoreLabelCommand(class SketchWidget *sketchWidget, long id, QDomElement &, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    QDomElement m_element;
};

class ShowLabelFirstTimeCommand : public BaseCommand
{
public:
    ShowLabelFirstTimeCommand(class SketchWidget *sketchWidget, CrossViewType crossView, long id, bool oldVis, bool newVis, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    bool m_oldVis;
    bool m_newVis;
};

class MoveLabelCommand : public BaseCommand
{
public:
    MoveLabelCommand(class SketchWidget *sketchWidget, long id, QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    QPointF m_oldPos;
    QPointF m_newPos;
    QPointF m_oldOffset;
    QPointF m_newOffset;
};

class ChangeLabelTextCommand : public BaseCommand
{
public:
    ChangeLabelTextCommand(class SketchWidget *sketchWidget, long id, const QString & oldText, const QString & newText, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    QString m_oldText;
    QString m_newText;
};

class ChangeNoteTextCommand : public BaseCommand
{
public:
    ChangeNoteTextCommand(class SketchWidget *sketchWidget, long id, const QString & oldText, const QString & newText, QSizeF oldSize, QSizeF newSize, QUndoCommand *parent);
    void undo();
    void redo();
	void setFirstTime(bool);
	int id() const;
	bool mergeWith(const QUndoCommand *other);

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    QString m_oldText;
    QString m_newText;
	QSizeF m_oldSize;
	QSizeF m_newSize;
	bool m_firstTime;

	static int changeNoteTextCommandID;
};

class RotateFlipLabelCommand : public BaseCommand
{
public:
	RotateFlipLabelCommand(class SketchWidget *sketchWidget, long id, qreal degrees, Qt::Orientations, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    qreal m_degrees;
	Qt::Orientations m_orientation;
};

class ResizeNoteCommand : public BaseCommand
{
public:
	ResizeNoteCommand(class SketchWidget *sketchWidget, long id, const QSizeF & oldSize, const QSizeF & newSize, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    QSizeF m_oldSize;
	QSizeF m_newSize;
};

class ResizeBoardCommand : public BaseCommand
{
public:
	ResizeBoardCommand(class SketchWidget *, long itemID, qreal oldWidth, qreal oldHeight, qreal newWidth, qreal newHeight, QUndoCommand * parent);
	void undo();
	void redo();

protected:
	QString getParamString() const;

protected:
	qreal m_oldWidth;
	qreal m_oldHeight;
	qreal m_newWidth;
	qreal m_newHeight;
	long m_itemID;
};

class SketchBackgroundColorChangeCommand : public BaseCommand
{
public:
	SketchBackgroundColorChangeCommand(SketchWidget* sketchWidget, const QString &oldColor, const QString &newColor, QUndoCommand *parent=0);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
	QString m_oldColor;
	QString m_newColor;
};

class SetResistanceCommand : public BaseCommand
{
public:
	SetResistanceCommand(class SketchWidget *, long itemID, QString oldResistance, QString newResistance, QString oldPinSpacing, QString newPinSpacing, QUndoCommand * parent);
	void undo();
	void redo();

protected:
	QString getParamString() const;

protected:
	QString m_oldResistance;
	QString m_newResistance;
	QString m_oldPinSpacing;
	QString m_newPinSpacing;
	long m_itemID;
};

class SetPropCommand : public BaseCommand
{
public:
	SetPropCommand(class SketchWidget *, long itemID, QString prop, QString oldValue, QString newValue, QUndoCommand * parent);
	void undo();
	void redo();

protected:
	QString getParamString() const;

protected:
	QString m_prop;
	QString m_oldValue;
	QString m_newValue;
	long m_itemID;
};

class ResizeJumperItemCommand : public BaseCommand
{
public:
	ResizeJumperItemCommand(class SketchWidget *, long itemID, QPointF oldPos, QPointF oldC0, QPointF oldC1, QPointF newPos, QPointF newC0, QPointF newC1, QUndoCommand * parent);
	void undo();
	void redo();

protected:
	QString getParamString() const;

protected:
	QPointF m_oldPos;
	QPointF m_oldC0;
	QPointF m_oldC1;
	QPointF m_newPos;
	QPointF m_newC0;
	QPointF m_newC1;
	long m_itemID;
};

class ShowLabelCommand : public BaseCommand
{
public:
    ShowLabelCommand(class SketchWidget *sketchWidget, QUndoCommand *parent);

    void undo();
    void redo();
    void add(long id, bool prev, bool post);

protected:
	QString getParamString() const;

protected:
    QHash<long, int> m_idStates;

};


class LoadLogoImageCommand : public BaseCommand
{
public:
    LoadLogoImageCommand(class SketchWidget *sketchWidget, long id, const QString & oldSvg, const QSizeF oldAspectRatio, const QString & oldFilename, const QString & newFilename, bool addName, QUndoCommand *parent);
    void undo();
    void redo();

protected:
	QString getParamString() const;

protected:
    long m_itemID;
    QString m_oldSvg;
    QSizeF m_oldAspectRatio;
	QString m_oldFilename;
	QString m_newFilename;
	bool m_addName;
};

#endif // COMMANDS_H
