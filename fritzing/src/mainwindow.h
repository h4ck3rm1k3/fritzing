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



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUndoView>
#include <QUndoGroup>
#include <QToolButton>
#include <QPushButton>

#include "fritzingwindow.h"
#include "sketchareawidget.h"
#include "sketchwidget.h"
#include "miniviewcontainer.h"
#include "palettemodel.h"
#include "sketchmodel.h"
#include "htmlinfoview.h"
#include "viewlayer.h"
#include "console.h"
#include "zoomcombobox.h"
#include "ftabwidget.h"
#include "sketchtoolbutton.h"

#include "partsbinpalette/partsbinpalettewidget.h"

QT_BEGIN_NAMESPACE
class QAction;
class QListWidget;
class QMenu;
QT_END_NAMESPACE

class MainWindow : public FritzingWindow
{
    Q_OBJECT

public:
    MainWindow(PaletteModel *, ReferenceModel *refModel);
    MainWindow(QFile & fileToLoad);
    void load(const QString & fileName, bool setAsLastOpened = true);
	void doOnce();

public:
	static void initExportConstants();

signals:
	void partsFromBundledDiscarded();
	void aboutToClose();

protected slots:
	void load();
	void openRecentOrExampleFile();
    void print();
    void doExport(QAction *);
	void exportDiy(QAction * action = NULL);
    void about();
    void copy();
    void cut();
    void paste();
    void group();
    void duplicate();
    void doDelete();
    void selectAll();
    void deselect();
    void zoomIn();
    void zoomOut();
    void zoomIn(int steps);
    void zoomOut(int steps);
    void fitInWindow();
    void actualSize();
    void showBreadboardView();
    void showSchematicView();
    void showPCBView();
    void updateEditMenu();
    void updateLayerMenu();
    void updatePartMenu();
    void updatePartMenuAux();
	void updateTraceMenu();
    void tabWidget_currentChanged(int index);
    // TODO PARTS EDITOR REMOVE
    void createNewPartInOldEditor();
    void createNewPart();
    void createNewSketch();
    void createNewSketchFromTemplate();
    void minimize();
    void toggleToolbar(bool toggle);
    void togglePartLibrary(bool toggle);
    void toggleInfo(bool toggle);
    void toggleNavigator(bool toggle);
    void toggleUndoHistory(bool toggle);
	void toggleDebuggerOutput(bool toggle);
	void openHelp();
	void openExamples();
	void openPartsReference();
	void visitFritzingDotOrg();
	void updateWindowMenu();
	void preferences();
	void pageSetup();
	void dockChangeActivation(class FDockWidget *);
	void sendToBack();
	void sendBackward();
	void bringForward();
	void bringToFront();
	void rotate90cw();
	void rotate90ccw();
	void rotate180();
	void flipHorizontal();
	void flipVertical();
	void showAllLayers();
	void hideAllLayers();

	// TODO PARTS EDITOR REMOVE
	void openInOldPartsEditor();
	void openInPartsEditor();
	// TODO PARTS EDITOR REMOVE
	void openOldPartsEditor(PaletteItem *);
	void openPartsEditor(PaletteItem *);

	void updateZoomOptions(qreal zoom);
	void updateZoomOptionsNoMatterWhat(qreal zoom);
	void updateViewZoom(qreal newZoom);

	void loadPart(QString newPartPath);
	void partsEditorClosed(long id);

	void findSketchWidgetSlot(ItemBase::ViewIdentifier, SketchWidget * &);

	void setInfoViewOnHover(bool infoViewOnHover);
	void swapSelected();
	void updateItemMenu();

	void autoroute();
	void createTrace();
	void createJumper();

	void saveBundledSketch();
	void loadBundledSketch(QString &fileName);

	void binSaved(bool hasPartsFromBundled);
	void routingStatusSlot(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumpers);
	void clearRoutingSlot(SketchWidget *, QUndoCommand * parentCommand);

protected:
    void createActions();
    void createFileMenuActions();
    void createOpenExampleMenu(QMenu * parentMenu, QString directory);
    void createOpenRecentMenu();
    void updateRecentFileActions();
    void createEditMenuActions();
    void createPartMenuActions();
    void createViewMenuActions();
    void createWindowMenuActions();
    void createHelpMenuActions();
    void createMenus();
    void createToolBars();
    void createZoomOptions();
    void createSketchButtons();
    void createStatusBar();
    void createDockWindows();
	void connectPairs();
	void connectPair(SketchWidget * signaller, SketchWidget * slotter);
	void closeEvent(QCloseEvent * event);
	void saveAsAux(const QString & fileName);
	void printAux(QPrinter &printer, QString message, bool removeBackground=true);
	void exportAux(QString fileName, QImage::Format format);
	void changeActivation(QEvent * event);
	void changeEvent ( QEvent * event );
	class FDockWidget * makeDock(const QString & title, QWidget * widget, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area = Qt::RightDockWidgetArea);
	class FDockWidget * dockIt(FDockWidget* dock, int dockMinHeight, int dockDefaultHeight, Qt::DockWidgetArea area = Qt::RightDockWidgetArea);
	void notYetImplemented(QString action);
	void calcPrinterScale();
	void preloadSlowParts();
	void setZoomComboBoxValue(qreal value);
	void setCurrentFile(const QString &fileName);
	bool eventFilter(QObject *obj, QEvent *event);
	void setShowViewActionsIcons(QAction * active, QAction * inactive1, QAction * inactive2);
	void exportToEagle();

	QList<QWidget*> getButtonsForView(ItemBase::ViewIdentifier viewId);

	const QString untitledFileName();
	int &untitledFileCount();
	const QString fileExtension();
	const QString defaultSaveFolder();
	const QString & fileName();
	bool undoStackIsEmpty();

	void createTraceMenuActions();
	void hideShowTraceMenu();

	void moveToPartsFolderAndLoad(const QString &unzipDir);
	void copyToSvgFolder(const QFileInfo& file, const QString &destFolder = "contrib");
	void copyToPartsFolder(const QFileInfo& file, const QString &destFolder = "contrib");

	void closeIfEmptySketch();
	bool whatToDoWithFilesAddedFromBundled();
	void backupExistingFileIfExists(const QString &destFilePath);
	void recoverBackupedFiles();
	void resetTempFolder();

	QMenu *breadboardItemMenu();
	QMenu *schematicItemMenu();
	QMenu *pcbItemMenu();

	QMenu *viewItemMenuAux(QMenu* menu);

protected:
	static qreal getSvgWidthInInches(const QString & filename);
	static qreal getSvgWidthInInches(QFile & file);
	static void removeActionsStartingAt(QMenu *menu, int start=0);


protected:
    enum SavedState {
    	NeverSaved,
    	Saved,
    	Restored
   	};

	QUndoGroup *m_undoGroup;
	QUndoView *m_undoView;

	SketchAreaWidget *m_breadboardWidget;
	SketchWidget *m_breadboardGraphicsView;

	SketchAreaWidget *m_schematicWidget;
	SketchWidget *m_schematicGraphicsView;

	SketchAreaWidget *m_pcbWidget;
	SketchWidget *m_pcbGraphicsView;

    PartsBinPaletteWidget *m_paletteWidget;
    MiniViewContainer *m_miniViewContainer;
    FTabWidget * m_tabWidget;
    PaletteModel *m_paletteModel;
    ReferenceModel *m_refModel;
    SketchModel *m_sketchModel;
    HtmlInfoView * m_infoView;

    QHash <long,class PartsEditorMainWindow*> m_partsEditorWindows;

    Console * m_consoleView;
    SavedState m_savedState;
    bool m_closing;
	bool m_dontClose;
    bool m_firstOpen;
    SketchWidget * m_currentWidget;

    //QToolBar *m_fileToolBar;
    //QToolBar *m_editToolBar;

    // Fritzing Menu
    QMenu *m_fritzingMenu;
    QAction *m_aboutAct;
    QAction *m_preferencesAct;
    QAction *m_quitAct;

    // File Menu
    enum { MaxRecentFiles = 10 };

	QMenu *m_fileMenu;
	QAction *m_newAct;
	QAction *m_newFromTemplateAct;
	QAction *m_openAct;
	QMenu *m_openRecentFileMenu;
    QAction *m_openRecentFileActs[MaxRecentFiles];
	QMenu *m_openExampleMenu;
	QAction *m_saveAct;
	QAction *m_saveAsAct;
	QAction *m_pageSetupAct;
	QAction *m_printAct;
	QAction *m_saveAsBundledAct;

	// Export Menu
	QMenu *m_exportMenu;
	QAction *m_exportJpgAct;
	QAction *m_exportPsAct;
	QAction *m_exportPngAct;
	QAction *m_exportPdfAct;
	QAction *m_exportEagleAct;
	QAction *m_exportDiyAct;

    // Edit Menu
    QMenu *m_editMenu;
    QAction *m_undoAct;
    QAction *m_redoAct;
    QAction *m_cutAct;
    QAction *m_copyAct;
    QAction *m_pasteAct;
    QAction *m_duplicateAct;
    QAction *m_deleteAct;
    QAction *m_selectAllAct;
    QAction *m_deselectAct;

    // Part Menu
    QMenu *m_partMenu;
    QAction *m_createNewPartActInOldEditor;
    // TODO PARTS EDITOR REMOVE
    QAction *m_createNewPart;
	QAction *m_openInOldPartsEditorAct;
	QAction *m_infoViewOnHoverAction;
	QAction *m_swapPartAction;
	// TODO PARTS EDITOR REMOVE
    QAction *m_openInPartsEditorAct;
	QAction *m_rotate90cwAct;
	QAction *m_rotate180Act;
	QAction *m_rotate90ccwAct;
	QAction *m_flipHorizontalAct;
	QAction *m_flipVerticalAct;
	QAction *m_bringToFrontAct;
	QAction *m_bringForwardAct;
	QAction *m_sendBackwardAct;
	QAction *m_sendToBackAct;
	QAction *m_groupAct;

    QAction *m_showAllLayersAct;
	QAction *m_hideAllLayersAct;

    // View Menu
    QMenu *m_viewMenu;
    QAction *m_zoomInAct;
    QAction *m_zoomOutAct;
    QAction *m_fitInWindowAct;
    QAction *m_actualSizeAct;
    QAction *m_showBreadboardAct;
    QAction *m_showSchematicAct;
    QAction *m_showPCBAct;
    //QAction *m_toggleToolbarAct;
    int m_numFixedActionsInViewMenu;

    // Window Menu
	QMenu *m_windowMenu;
	QAction *m_minimizeAct;
	QAction *m_togglePartLibraryAct;
	QAction *m_toggleInfoAct;
	QAction *m_toggleNavigatorAct;
	QAction *m_toggleUndoHistoryAct;
	QAction *m_toggleDebuggerOutputAct;

	// Trace Menu
	QMenu *m_traceMenu;
	QAction *m_autorouteAct;
	QAction *m_createTraceAct;
	QAction *m_createJumperAct;


	// Help Menu
    QMenu *m_helpMenu;
    QAction *m_openHelpAct;
    QAction *m_examplesAct;
    QAction *m_partsRefAct;
    QAction *m_visitFritzingDotOrgAct;

	QMenu *m_itemMenu;

    // Dot icons
    QIcon m_dotIcon;
    QIcon m_emptyIcon;

    // Sketch toolbar buttons
	SketchToolButton *m_exportToPdfButton;
	SketchToolButton *m_rotateButton;
	SketchToolButton *m_flipButton;
	SketchToolButton *m_exportDiyButton;
	SketchToolButton *m_autorouteButton;

	QLabel *m_routingStatusLabel;

    ZoomComboBox * m_zoomOptsComboBox;
    bool m_comboboxChanged;

    QStringList m_filesAddedFromBundled;
    QStringList m_filesReplacedByBundleds;

public:
	static const int PartsBinDefaultHeight = 220;
	static const int PartsBinMinHeight = 122;
	static const int NavigatorDefaultHeight = 70;
	static const int NavigatorMinHeight = NavigatorDefaultHeight;
	static const int InfoViewDefaultHeight = 50;
	static const int InfoViewMinHeight = InfoViewDefaultHeight;
	static const int UndoHistoryDefaultHeight = 70;
	static const int UndoHistoryMinHeight = UndoHistoryDefaultHeight;
public:
	static const int DockDefaultWidth = 185;
	static const int DockMinWidth = 130;
	static const int DockDefaultHeight = 50;
	static const int DockMinHeight = 30;

protected:
	static const QString UntitledSketchName;
	static int UntitledSketchIndex;
	static qreal m_printerScale;
};

#endif
