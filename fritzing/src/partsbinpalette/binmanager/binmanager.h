/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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


#ifndef BINMANAGER_H_
#define BINMANAGER_H_

#include <QStackedWidget>
#include <QTabWidget>
#include <QMenu>
#include <QLabel>
#include <QDir>

class ModelPart;
class PaletteModel;
class MainWindow;

class BinManager : public QFrame {
	Q_OBJECT
	public:
		BinManager(class ReferenceModel *refModel, class HtmlInfoView *infoView, class WaitPushUndoStack *undoStack, MainWindow* parent);
		virtual ~BinManager();

		void loadFromModel(PaletteModel *model);
		void setPaletteModel(PaletteModel *model);

		void addBin(class PartsBinPaletteWidget* bin);
		void insertBin(PartsBinPaletteWidget* bin, int index);
		void addPart(ModelPart *modelPart, int position = -1);
		void addToMyPart(ModelPart *modelPart);

		void addNewPart(ModelPart *modelPart);

		bool beforeClosing();

		bool hasAlienParts();
        QString createIfMyPartsNotExists();
        QString createIfSearchNotExists();
        QString createIfBinNotExists(const QString & dest, const QString & source);

		void setInfoViewOnHover(bool infoViewOnHover);
		void load(const QString&);

		void setDirtyTab(PartsBinPaletteWidget* w, bool dirty=true);
		void updateTitle(PartsBinPaletteWidget* w, const QString& newTitle);

		void openBin(const QString &fileName);
		PartsBinPaletteWidget* openBinIn(QString fileName, bool fastLoad);
		PartsBinPaletteWidget* openCoreBinIn();
		void closeBinIn(int index=-1);

		void addPartTo(PartsBinPaletteWidget* bin, ModelPart* mp);
		void newPartTo(PartsBinPaletteWidget* bin);
		void importPartTo(PartsBinPaletteWidget* bin);
		void editSelectedPartFrom(PartsBinPaletteWidget* bin);

		void dockedInto(class FDockWidget* dock);

		const QString &getSelectedModuleIDFromSketch();
		QList<QAction*> openedBinsActions(const QString &moduleId);

		MainWindow* mainWindow();
        void search(const QString & searchText);
		bool currentViewIsIconView();
		void updateViewChecks(bool iconView);
		QMenu * binContextMenu();
		QMenu * partContextMenu();
		QMenu * combinedMenu();
		void setTabIcon(PartsBinPaletteWidget* w, QIcon *);

	signals:
		void savePartAsBundled(const QString &moduleId);


	public slots:
		void updateBinCombinedMenu();
		void toIconView();
		void toListView();

	protected slots:
		void updateFileName(PartsBinPaletteWidget* bin, const QString &newFileName, const QString &oldFilename);
		void setAsCurrentBin(PartsBinPaletteWidget* bin);
		void currentChanged(int);
		void tabCloseRequested(int);
		PartsBinPaletteWidget* newBinIn();
		void openNewBin();
		void closeBin();
		void newPart();
		void importPart();
		void editSelected();
		void saveBin();
		void saveBinAs();
		void renameBin();
		void exportSelected();
		bool removeSelected();
		void saveBundledBin();

	protected:
		void createMenu();
		PartsBinPaletteWidget* newBin();
		void registerBin(PartsBinPaletteWidget* bin);
		PartsBinPaletteWidget* getBin(int index);
		PartsBinPaletteWidget* currentBin();
		void saveStateAndGeometry();
		void restoreStateAndGeometry();
		void setAsCurrentTab(PartsBinPaletteWidget* bin);
        PartsBinPaletteWidget* getOrOpenMyPartsBin();
        PartsBinPaletteWidget* getOrOpenSearchBin();
        PartsBinPaletteWidget* getOrOpenBin(const QString & dest, const QString & source);
        void connectTabWidget();
		void addPartAux(PartsBinPaletteWidget *bin, ModelPart *modelPart, int position = -1);
		PartsBinPaletteWidget* findBin(const QString & binLocation);
		void createCombinedMenu();
		void createContextMenus();
		void loadAllBins();
		void loadBins(QDir &);

protected:
		ReferenceModel *m_refModel;
		PaletteModel *m_paletteModel;

		HtmlInfoView *m_infoView;
		WaitPushUndoStack *m_undoStack;

		MainWindow *m_mainWindow;
		PartsBinPaletteWidget *m_currentBin;
		class StackTabWidget* m_stackTabWidget;

		QHash<QString /*filename*/,PartsBinPaletteWidget*> m_openedBins;
		int m_unsavedBinsCount;
		QString m_defaultSaveFolder;

		QMenu *m_binContextMenu;
		QMenu *m_combinedMenu;		

		QAction *m_newBinAction;
		QAction *m_openBinAction;
		QAction *m_closeBinAction;
		QAction *m_saveBinAction;
		QAction *m_saveBinAsAction;
		QAction *m_saveBinAsBundledAction;
		QAction *m_renameBinAction;

		QAction *m_showListViewAction;
		QAction *m_showIconViewAction;

		QMenu *m_partContextMenu;
		QMenu *m_partMenu;	
		QAction *m_newPartAction;
		QAction *m_importPartAction;
		QAction *m_editPartAction;
		QAction *m_exportPartAction;
		QAction *m_removePartAction;

	protected:
		static QString StandardBinStyle;
		static QString CurrentBinStyle;

	public:
		static QString Title;
        static QString CorePartsBinLocation;
        static QString MyPartsBinLocation;
        static QString MyPartsBinTemplateLocation;
        static QString SearchBinLocation;
        static QString SearchBinTemplateLocation;
		static QString ContribPartsBinLocation;
		static QHash<QString, QString> StandardBinIcons;
		static bool isTabReorderingEvent(QDropEvent* event);
		static void initNames();
};

#endif /* BINMANAGER_H_ */
