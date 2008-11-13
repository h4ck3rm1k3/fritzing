#ifndef MAINPARTSEDITORWINDOW_H
#define MAINPARTSEDITORWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QtGui/qwidget.h>
#include "partseditorsketchwidget.h"
#include "connectorswidget.h"
#include "partinfowidget.h"
#include "../itembase.h"
#include "../palettemodel.h"
#include "../sketchmodel.h"
#include "../zoomcombobox.h"
//

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)
QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSplitter)

class MainPartsEditorWindow : public QMainWindow
{
Q_OBJECT

public:
	MainPartsEditorWindow(long id, QWidget * parent = 0, Qt::WFlags f = 0, ModelPart * modelPart=0, bool fromTemplate = false);
	static const QString templatePath;

signals:
	void partUpdated(QString);
	void closed(long id);

protected slots:
	void save();
    void saveAs();
	void updateZoomOptions(qreal);
	void updateViewZoom(QString);
	void updateZoomOptionsNoMatterWhat(qreal);
	void zoomIn(int);
	void zoomOut(int);
	void updateZoomText(int);
	void loadFileIntoView(StringPair *svgFilePath, PartsEditorSketchWidget * view, ItemBase::ViewIdentifier viewIdentifier, QString layer);
	void tabWidget_currentChanged(int index);

	void loadPcbFootprint();
	void loadSchemFile();
	void loadIconFile();
	void loadBreadFile();

protected:
	void createDockWindows();
	bool createTemplate();
	void createMenus();
	void createZoomOptions();
	void replicateDir(QDir srcDir, QDir targDir);

	void saveAsAux(const QString & fileName);

	void makeDock(const QString & title, QWidget * widget,  Qt::DockWidgetArea area);
	void connectViewToConnectorsDock(PartsEditorSketchWidget * view);

	void copyToTempAndRename(QString filePathOrig, StringPair *filePathDest, ItemBase::ViewIdentifier viewId);
	QString getRandText();

	void copySvgFilesToDestiny();
	void copySvgFilesToDestinyAux(StringPair *path);
	QDir createTempFolderIfNecessary();

	void loadSketchWidgetFromModel(PartsEditorSketchWidget *widget, StringPair *filePath, ModelPart * modelPart, ItemBase::ViewIdentifier viewId);

	void closeEvent(QCloseEvent *event);
	void rmdir(QDir & dir);

	long m_id;

	QString m_fileName;
	QString m_pcbSvgFile;
	StringPair *m_schemSvgFile;
	StringPair *m_iconSvgFile;
	StringPair *m_breadSvgFile;
	QString m_partPath;
	QDir m_tempDir;
	QDir m_partDir;

	PaletteModel *m_paletteModel;
	SketchModel *m_sketchModel;
	class WaitPushUndoStack *m_undoStack;
	PartsEditorSketchWidget *m_pcbView;
	PartsEditorSketchWidget *m_iconView;
	PartsEditorSketchWidget *m_schemView;
	PartsEditorSketchWidget *m_breadView;
	PartsEditorSketchWidget * m_currentWidget;

	QTabWidget *m_tabWidget;
	QFrame *m_mainFrame;
    QStackedWidget *m_settingsStack;
    ZoomComboBox *m_zoomOptsComboBox;
    bool m_comboboxChanged;

    ConnectorsWidget *m_connWidget;
    PartInfoWidget *m_partInfoWidget;

public:
	//static QString QtFunkyPlaceholder;  // this is some wierd hack Qt uses in window titles as a placeholder to setr the modified state
	static const QString UntitledPartName;
	static int UntitledPartIndex;
};
#endif




