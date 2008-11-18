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


#ifndef PARTSEDITORMAINWINDOW_H_
#define PARTSEDITORMAINWINDOW_H_

#include <QMainWindow>
#include <QStackedWidget>
#include <QtGui/qwidget.h>

#include "../fritzingwindow.h"
#include "partsymbolswidget.h"
#include "connectorsviewswidget.h"
#include "connectorsinfowidget.h"
#include "partconnectorswidget.h"
#include "partseditorviewimagewidget.h"
#include "editabletextwidget.h"
#include "editablelinewidget.h"
#include "editabledatewidget.h"
#include "hashpopulatewidget.h"
#include "connectorswidget.h"
#include "partinfowidget.h"
#include "../itembase.h"
#include "../palettemodel.h"
#include "../sketchmodel.h"
#include "../zoomcombobox.h"
#include "../mainwindow.h"

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)
QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSplitter)

#define TITLE_FRESH_START_TEXT tr("Please find a name for me!")
#define LABEL_FRESH_START_TEXT tr("Please provide a label")
#define DESCRIPTION_FRESH_START_TEXT tr("You could tell a little bit about this part")
#define TAXONOMY_FRESH_START_TEXT tr("Please clasify this part")
#define TAGS_FRESH_START_TEXT tr("You can add your tags to find things easier")
#define FOOTER_TEXT tr("<i>created by</i> %1 <i>on</i> %2")


class PartsEditorMainWindow : public FritzingWindow
{
Q_OBJECT

public:
	PartsEditorMainWindow(long id, QWidget * parent = 0, Qt::WFlags f = 0, ModelPart * modelPart=0, bool fromTemplate = false);
	static const QString templatePath;
	const QDir& tempDir();

signals:
	void partUpdated(QString);
	void closed(long id);

public slots:
	void parentAboutToClose();

protected slots:
	void loadPcbFootprint();
	void updateDateAndAuthor();
	bool saveAs();

protected:
	bool createTemplate();
	void saveAsAux(const QString & fileName);
	const QDir& createTempFolderIfNecessary();
	void closeEvent(QCloseEvent *event);

	void createHeader(ModelPart * = 0);
	void createCenter(ModelPart * = 0);
	void connectWidgetsToSave(const QList<QWidget*> &widgets);
	void createFooter();

	ModelPartStuff* modelPartStuff();

	const QString untitledFileName();
	int &untitledFileCount();
	const QString fileExtension();
	const QString defaultSaveFolder();

	void updateSaveButton();

	void cleanUp();

protected:
	long m_id;

	PaletteModel *m_paletteModel;
	SketchModel *m_sketchModel;

	PartsEditorViewImageWidget *m_iconViewImage;
	EditableLineWidget *m_title;
	PartSymbolsWidget *m_symbols;
	EditableLineWidget *m_label;
	EditableTextWidget *m_description;
	//EditableLineWidget *m_taxonomy;
	EditableLineWidget *m_tags;
	HashPopulateWidget *m_properties;
	EditableLineWidget *m_author;
	EditableDateWidget *m_createdOn;
	QLabel *m_createdByText;

	ConnectorsViewsWidget *m_connsViews;
	ConnectorsInfoWidget *m_connsInfo;

	QString m_version;
	QString m_moduleId;
	QString m_uri;


	QPushButton *m_saveAsNewPartButton;
	QPushButton *m_saveButton;
	QPushButton *m_cancelButton;

	QTabWidget *m_tabWidget;

    QFrame *m_headerFrame;
    QFrame *m_centerFrame;
    QFrame *m_footerFrame;

    bool m_updateEnabled;

    static PartsEditorMainWindow *m_lastOpened;
    static int m_closedBeforeCount;

public:
	static const QString UntitledPartName;
	static int UntitledPartIndex;
	static QGraphicsItem *emptyViewItem(QString iconFile, QString text="");
};
#endif /* PARTSEDITORMAINWINDOW_H_ */
