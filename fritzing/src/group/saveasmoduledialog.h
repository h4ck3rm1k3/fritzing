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


#ifndef SAVEASMODULEDIALOG_H
#define SAVEASMODULEDIALOG_H

#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QHash>

class SaveAsModuleDialog : public QDialog
{
	Q_OBJECT

public:
	SaveAsModuleDialog(class SketchWidget *, QList<class ConnectorItem *> & externalConnectors, QWidget *parent = 0);
	~SaveAsModuleDialog();

	QString author();
	QString title();
	QString createdOn();
	QString label();
	QString description();
	QStringList tags();
	const QHash<QString, QString> & properties();
	const QList<class ConnectorItem *> & externalConnectorItems();

protected:
	bool eventFilter(QObject *obj, QEvent *);
	void handleSceneMousePress(QEvent *);
	
protected slots:
	//void updateDateAndAuthor();
	void checkAccept();

protected:
	class SketchWidget * m_sketchWidget;
	class SketchModel * m_sketchModel;
	QList<class ConnectorItem *> m_externalConnectorItems;
	class EditableLineWidget * m_titleWidget;
	class EditableLineWidget * m_labelWidget;
	class EditableTextWidget * m_descriptionWidget;
	class HashPopulateWidget * m_propertiesWidget;
	class EditableLineWidget * m_tagsWidget;
	class EditableLineWidget * m_authorWidget;
	class EditableDateWidget * m_createdOnWidget;
	class WaitPushUndoStack * m_undoStack;
	//QLabel * m_createdByTextWidget;
};

#endif 
