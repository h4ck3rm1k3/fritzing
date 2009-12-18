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


#ifndef BINMANAGER_H_
#define BINMANAGER_H_

#include <QStackedWidget>
#include <QTabWidget>

#include "stackwidget.h"

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
		void insertBin(PartsBinPaletteWidget* bin, int index, StackTabWidget* tb);
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

		PartsBinPaletteWidget* newBinIn(StackTabWidget* tb);
		void openBin(const QString &fileName);
		PartsBinPaletteWidget* openBinIn(StackTabWidget* tb, QString fileName="");
		PartsBinPaletteWidget* openCoreBinIn(StackTabWidget* tb);
		void closeBinIn(StackTabWidget* tb, int index=-1);

		void addPartTo(PartsBinPaletteWidget* bin, ModelPart* mp);
		void newPartTo(PartsBinPaletteWidget* bin);
		void importPartTo(PartsBinPaletteWidget* bin);
		void editSelectedPartFrom(PartsBinPaletteWidget* bin);

		void dockedInto(class FDockWidget* dock);

		const QString &getSelectedModuleIDFromSketch();
		QList<QAction*> openedBinsActions(const QString &moduleId);

		MainWindow* mainWindow();
        void search(const QString & searchText);

	protected slots:
		void updateFileName(PartsBinPaletteWidget* bin, const QString &newFileName, const QString &oldFilename);
		void setAsCurrentBin(PartsBinPaletteWidget* bin);
		void currentChanged(StackTabWidget*,int);
		void tabCloseRequested(StackTabWidget*,int);

	protected:
		void createMenu();
		PartsBinPaletteWidget* newBin();
		void registerBin(PartsBinPaletteWidget* bin, StackTabWidget *tb);
		PartsBinPaletteWidget* getBin(StackTabWidget* tb, int index);
		PartsBinPaletteWidget* currentBin(StackTabWidget* tb);
		void saveStateAndGeometry();
		void restoreStateAndGeometry();
		void setAsCurrentTab(PartsBinPaletteWidget* bin);
        PartsBinPaletteWidget* getOrOpenMyPartsBin();
        PartsBinPaletteWidget* getOrOpenSearchBin();
        PartsBinPaletteWidget* getOrOpenBin(const QString & dest, const QString & source);
        void connectTabWidget(StackTabWidget *tw);
		void addPartAux(PartsBinPaletteWidget *bin, ModelPart *modelPart, int position = -1);

		ReferenceModel *m_refModel;
		PaletteModel *m_paletteModel;

		HtmlInfoView *m_infoView;
		WaitPushUndoStack *m_undoStack;

		MainWindow *m_mainWindow;
		StackWidget *m_stackWidget;
		PartsBinPaletteWidget *m_currentBin;

		QHash<PartsBinPaletteWidget*,StackTabWidget*> m_tabWidgets;
		QHash<QString /*filename*/,PartsBinPaletteWidget*> m_openedBins;
		int m_unsavedBinsCount;
		QString m_defaultSaveFolder;

	protected:
		static QString StandardBinStyle;
		static QString CurrentBinStyle;

	public:
		static QString Title;
        static QString CoreBinLocation;
        static QString MyPartsBinLocation;
        static QString MyPartsBinTemplateLocation;
        static QString SearchBinLocation;
        static QString SearchBinTemplateLocation;
        static QString AllPartsBinLocation;
		static QString NonCorePartsBinLocation;
		static bool isTabReorderingEvent(QDropEvent* event);
		static void initNames();
};

#endif /* BINMANAGER_H_ */
