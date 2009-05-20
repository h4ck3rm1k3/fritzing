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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUndoView>
#include <QUndoGroup>
#include <QToolButton>
#include <QPushButton>
#include <QStackedWidget>
#include <QSizeGrip>

#include "fritzingwindow.h"
#include "sketchareawidget.h"
#include "navigator/miniviewcontainer.h"
#include "viewlayer.h"
#include "zoomcombobox.h"
#include "sketchtoolbutton.h"

QT_BEGIN_NAMESPACE
class QAction;
class QListWidget;
class QMenu;
QT_END_NAMESPACE

class Helper;
class DockManager;
class FSizeGrip;

// help struct to create the example menu from a xml file
struct SketchDescriptor {
	SketchDescriptor(const QString &_id, const QString &_name, const QString &_src) {
		id = _id;
		name = _name;
		src = _src;
	}

	QString id;
	QString name;
	QString src;
};



#define SketchIndex QHash<QString /*id*/, SketchDescriptor*>

bool sortPartList(ItemBase * b1, ItemBase * b2);

class MainWindow : public FritzingWindow
{
    Q_OBJECT
public:
    MainWindow(class PaletteModel *, ReferenceModel *refModel);
    MainWindow(QFile & fileToLoad);
	~MainWindow();

    void load(const QString & fileName, bool setAsLastOpened = true, bool addToRecent = true);
	bool loadWhich(const QString & fileName, bool setAsLastOpened = true, bool addToRecent = true);
	void notClosableForAWhile();
	QAction *raiseWindowAction();
	QSizeGrip *sizeGrip();
	QStatusBar *realStatusBar();
	void showAllFirstTimeHelp(bool show);
	void enableCheckUpdates(bool enabled);

	class PartsEditorMainWindow* getPartsEditor(ModelPart *modelPart, long id=-1, class PartsBinPaletteWidget* requester=NULL);
	ModelPart *loadPartFromFile(const QString& newPartPath);
	void addBoard();
	void init();
	void showFileProgressDialog();
	void clearFileProgressDialog();

public:
	static void initExportConstants();
	static MainWindow * newMainWindow(PaletteModel * paletteModel, ReferenceModel *refModel, bool showProgress);

signals:
	void alienPartsDismissed();
	void aboutToClose();
	void viewSwitched(int);
	void mainWindowMoved(QWidget *);

public slots:
	void ensureClosable();
	void changeActivation(bool activate);
	void swapSelected(const QVariant & currProps, const QString &family, const QString & name);
	ModelPart* loadBundledPart(const QString &fileName, bool addToBin=true);
	void partsEditorClosed(long id);

protected slots:
	void load();
	void openRecentOrExampleFile();
    void print();
    void doExport();
	void exportEtchable();
	void exportEtchableSvg();
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
    void updateWireMenu();
	void updateFileMenu();
    void updateTransformationActions();
	void updateRecentFileActions();
	void updateTraceMenu();
    void tabWidget_currentChanged(int index);
    // TODO PARTS EDITOR REMOVE
    void createNewPartInOldEditor();
    void createNewPart();
    void createNewSketch();
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
	void pageSetup();
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
	void showInViewHelp();
	void addBendpoint();

	// TODO PARTS EDITOR REMOVE
	void openInOldPartsEditor();
	void openInPartsEditor();
	// TODO PARTS EDITOR REMOVE
	void openOldPartsEditor(PaletteItem *);
	void openPartsEditor(PaletteItem *);
	void addToBin();

	void updateZoomOptions(qreal zoom);
	void updateZoomOptionsNoMatterWhat(qreal zoom);
	void updateViewZoom(qreal newZoom);

	void loadPart(const QString &newPartPath, long partsEditorId=-1);

	void findSketchWidgetSlot(ViewIdentifierClass::ViewIdentifier, SketchWidget * &);

	void setInfoViewOnHover(bool infoViewOnHover);
	void updateItemMenu();

	void autoroute();
	void createTrace();
	void createJumper();
	void excludeFromAutoroute();
	void selectAllTraces();
	void selectAllExcludedTraces();
	void selectAllJumpers();

	void saveBundledSketch();
	void saveBundledPart(const QString &moduleId=___emptyString___);
	void saveBundledAux(ModelPart *mp, const QDir &destFolder);
	void loadBundledSketch(const QString &fileName);
	void loadBundledPart();
	void loadBundledPartFromWeb();
	void saveAsModule();
	void editModule();

	void binSaved(bool hasAlienParts);
	void routingStatusSlot(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumpers);
	void clearRoutingSlot(SketchWidget *, QUndoCommand * parentCommand);

	void applyReadOnlyChange(bool isReadOnly);
	void currentNavigatorChanged(MiniViewContainer *);
	void viewSwitchedTo(int viewIndex);

	void raiseAndActivate();
	void activateWindowAux();
	void showPartLabels();
	void addNote();
	void reportBug();

protected:
	void initSketchWidget(SketchWidget *);

    void createActions();
    void createFileMenuActions();
    void createOpenExampleMenu();
    void populateMenuFromXMLFile(
    		QMenu *parentMenu, QStringList &actionsTracker,
    		const QString &folderPath, const QString &indexFileName/*,
    		const QString &rootNode, const QString &indexNode,
    		const QString &submenuNode*/
    );
    SketchIndex indexAvailableElements(QDomElement &domElem, const QString &srcPrefix="");
    void populateMenuWithIndex(const SketchIndex &index, QMenu * parentMenu, QStringList &actionsTracker, QDomElement &domElem);
    void populateMenuFromFolderContent(QMenu *parentMenu, const QString &path);
    void createOpenRecentMenu();
    void createEditMenuActions();
    void createPartMenuActions();
    void createViewMenuActions();
    void createWindowMenuActions();
    void createHelpMenuActions();
    void createMenus();
    void createToolBars();
    void createSketchButtons();
    void createStatusBar();
	void connectPairs();
	void connectPair(SketchWidget * signaller, SketchWidget * slotter);
	void closeEvent(QCloseEvent * event);
	void saveAsAux(const QString & fileName);
	void printAux(QPrinter &printer, const QString & message, bool removeBackground=true);
	void exportAux(QString fileName, QImage::Format format);
	void notYetImplemented(QString action);
	void setZoomComboBoxValue(qreal value, ZoomComboBox* zoomComboBox = NULL);
	void setCurrentFile(const QString &fileName, bool addToRecent=true);
	bool eventFilter(QObject *obj, QEvent *event);
	void setShowViewActionsIcons(QAction * active, QAction * inactive1, QAction * inactive2);
	void exportToEagle();
	void exportToGerber();
    void exportBOM();
    void exportSvg();
	void exportEtchable(bool wantPDF, bool wantSVG);

	QList<QWidget*> getButtonsForView(ViewIdentifierClass::ViewIdentifier viewId);

	const QString untitledFileName();
	int &untitledFileCount();
	const QString fileExtension();
	const QString defaultSaveFolder();
	const QString & fileName();
	bool undoStackIsEmpty();

	void createTraceMenuActions();
	void hideShowTraceMenu();

	QList<ModelPart*> moveToPartsFolder(QDir &unzipDir, MainWindow* mw, bool addToBin=true);
	void loadBundledSketchAux(QDir &unzipDir, MainWindow* mw);
	void copyToSvgFolder(const QFileInfo& file, const QString &destFolder = "contrib");
	ModelPart* copyToPartsFolder(const QFileInfo& file, bool addToBin=true, const QString &destFolder="contrib");

	void closeIfEmptySketch(MainWindow* mw);
	bool whatToDoWithAlienFiles();
	void backupExistingFileIfExists(const QString &destFilePath);
	void recoverBackupedFiles();
	void resetTempFolder();

	QMenu *breadboardItemMenu();
	QMenu *schematicItemMenu();
	QMenu *pcbItemMenu();
	QMenu *pcbWireMenu();

	QMenu *viewItemMenuAux(QMenu* menu);

	ZoomComboBox *createZoomOptions(SketchAreaWidget* parent);
	SketchToolButton *createRotateButton(SketchAreaWidget *parent);
	SketchToolButton *createFlipButton(SketchAreaWidget *parent);
	SketchToolButton *createAutorouteButton(SketchAreaWidget *parent);
	SketchToolButton *createExportEtchableButton(SketchAreaWidget *parent);
	SketchToolButton *createNoteButton(SketchAreaWidget *parent);
	QWidget *createToolbarSpacer(SketchAreaWidget *parent);

	SketchAreaWidget *currentSketchArea();
	const QString fritzingTitle();

	void updateRaiseWindowAction();

	void moveEvent(QMoveEvent * event);
	bool event(QEvent *);
	void resizeEvent(QResizeEvent * event);
	QString genIcon(SketchWidget *, QList<ViewLayer::ViewLayerID> &  partViewLayerIDs, QList<ViewLayer::ViewLayerID> & wireViewLayerIDs);
	void initExternalConnectors(QList<ConnectorItem *> & externalConnectors);

	bool alreadyOpen(const QString & fileName);
	bool loadCustomBoardShape();
	void svgMissingLayer(const QString & layername, const QString & path);
	void swapSelectedAux(ItemBase * itemBase, const QString & moduleID);
	void enableAddBendpointAct(QGraphicsItem *);
	class FileProgressDialog * exportProgress();

protected:
	//static qreal getSvgWidthInInches(const QString & filename);
	//static qreal getSvgWidthInInches(QFile & file);
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
	class BreadboardSketchWidget *m_breadboardGraphicsView;

	SketchAreaWidget *m_schematicWidget;
	class SchematicSketchWidget *m_schematicGraphicsView;

	SketchAreaWidget *m_pcbWidget;
	class PCBSketchWidget *m_pcbGraphicsView;

    class BinManager *m_paletteWidget;
    MiniViewContainer *m_miniViewContainerBreadboard;
    MiniViewContainer *m_miniViewContainerSchematic;
    MiniViewContainer *m_miniViewContainerPCB;
	QList <MiniViewContainer *> m_navigators;
	QStackedWidget * m_tabWidget;
    class PaletteModel *m_paletteModel;
    ReferenceModel *m_refModel;
    class SketchModel *m_sketchModel;
    class HtmlInfoView * m_infoView;
    QToolBar *m_toolbar;

    QHash <long,class PartsEditorMainWindow*> m_partsEditorWindows;
    QHash <long,class PartsBinPaletteWidget*> m_binsWithPartsEditorRequests;

    class Console * m_consoleView;
    SavedState m_savedState;
    bool m_closing;
	bool m_dontClose;
    bool m_firstOpen;

    SketchAreaWidget *m_currentWidget;
    SketchWidget * m_currentGraphicsView;

    //QToolBar *m_fileToolBar;
    //QToolBar *m_editToolBar;

    QAction *m_raiseWindowAct;

    // Fritzing Menu
    QMenu *m_fritzingMenu;
    QAction *m_aboutAct;
    QAction *m_preferencesAct;
    QAction *m_quitAct;

    // File Menu
    enum { MaxRecentFiles = 10 };

	QMenu *m_fileMenu;
	QAction *m_newAct;
	QAction *m_openAct;
	QMenu *m_openRecentFileMenu;
    QAction *m_openRecentFileActs[MaxRecentFiles];
	QMenu *m_openExampleMenu;
	QAction *m_saveAct;
	QAction *m_saveAsAct;
	QAction *m_pageSetupAct;
	QAction *m_printAct;
	QAction *m_saveAsBundledAct;
	QAction *m_saveAsModuleAct;
	QAction *m_editModuleAct;

	QMenu *m_zOrderMenu;
	QAction *m_bringToFrontAct;
	QAction *m_bringForwardAct;
	QAction *m_sendBackwardAct;
	QAction *m_sendToBackAct;

	// Export Menu
	QMenu *m_exportMenu;
	QAction *m_exportJpgAct;
	QAction *m_exportPsAct;
	QAction *m_exportPngAct;
	QAction *m_exportPdfAct;
	QAction *m_exportEagleAct;
	QAction *m_exportGerberAct;
	QAction *m_exportEtchableAct;
	QAction *m_exportEtchableSvgAct;
	QAction *m_exportBomAct;
	QAction *m_exportSvgAct;

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
	QAction *m_addNoteAct;

    // Part Menu
    QMenu *m_partMenu;
    QAction *m_createNewPartActInOldEditor;
    // TODO PARTS EDITOR REMOVE
    QAction *m_createNewPart;
	QAction *m_openInOldPartsEditorAct;
	QAction *m_infoViewOnHoverAction;
	// TODO PARTS EDITOR REMOVE
    QAction *m_openInPartsEditorAct;
    QAction *m_addToBinAct;
	QAction *m_rotate90cwAct;
	QAction *m_rotate180Act;
	QAction *m_rotate90ccwAct;
	QAction *m_flipHorizontalAct;
	QAction *m_flipVerticalAct;
	QAction *m_groupAct;
	QAction *m_showPartLabelAct;
	QAction *m_loadBundledPart;
	QAction *m_saveBundledPart;

	QAction *m_addBendpointAct;

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
	QAction *m_excludeFromAutorouteAct;
	QAction *m_selectAllTracesAct;
	QAction *m_selectAllExcludedTracesAct;
	QAction *m_selectAllJumpersAct;


	// Help Menu
    QMenu *m_helpMenu;
    QAction *m_openHelpAct;
    QAction *m_examplesAct;
    QAction *m_partsRefAct;
    QAction *m_showInViewHelpAct;;
    QAction *m_visitFritzingDotOrgAct;
    QAction *m_checkForUpdatesAct;
    QAction *m_aboutQtAct;
    QAction *m_reportBugAct;

    // Dot icons
    QIcon m_dotIcon;
    QIcon m_emptyIcon;

	class ExpandingLabel *m_routingStatusLabel;
	QList<SketchToolButton*> m_rotateButtons;
	QList<SketchToolButton*> m_flipButtons;

    bool m_comboboxChanged;

    QStringList m_alienFiles;
    QString m_alienPartsMsg;
    QStringList m_filesReplacedByAlienOnes;

    QStringList m_openExampleActions;

	class TripleNavigator * m_tripleNavigator;
	class FSizeGrip *m_sizeGrip;

	friend class Helper;
	friend class DockManager;

	class ViewSwitcher * m_viewSwitcher;

	Helper *m_helper;
	QTimer m_setUpDockManagerTimer;
	class DockManager * m_dockManager;
	class FileProgressDialog * m_fileProgressDialog;

protected:
	static const QString UntitledSketchName;
	static int UntitledSketchIndex;
	static int CascadeFactorX;
	static int CascadeFactorY;
};

#endif
