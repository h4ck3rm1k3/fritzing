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


#ifndef PARTSBINPALETTEWIDGET_H_
#define PARTSBINPALETTEWIDGET_H_

#include <QFrame>
#include <QToolButton>

#include "../palettemodel.h"
#include "../modelpart.h"
#include "../waitpushundostack.h"
#include "../abstractimagebutton.h"

class ImageButton : public AbstractImageButton {
public:
	ImageButton(const QString &imageName, QWidget *parent=0, bool hasStates=true)
		: AbstractImageButton(parent)
	{
		setupIcons(imageName, hasStates);
	};
protected:
	QString imagePrefix() {
		return ":/resources/images/icons/partsBin";
	}
};

class PartsBinPaletteWidget : public QFrame {
	Q_OBJECT
	public:
		PartsBinPaletteWidget(class ReferenceModel *refModel, class HtmlInfoView *infoView, WaitPushUndoStack *undoStack, class BinManager* manager);
		~PartsBinPaletteWidget();

		QSize sizeHint() const;
		QString title() const;
		void setTitle(const QString &title);

		void setTabWidget(class StackTabWidget *tabWidget);

		void loadFromModel(PaletteModel *model);
		void setPaletteModel(PaletteModel *model, bool clear=false);

		void addPart(ModelPart *modelPart, int position = -1);

		bool currentBinIsCore();
		bool beforeClosing();

		ModelPart * selected();
		bool hasAlienParts();

		void setInfoViewOnHover(bool infoViewOnHover);
		void addPart(const QString& moduleID, int position = -1);
		void addNewPart(ModelPart *modelPart);
		void removePart(const QString& moduleID);
		void load(const QString&);

		bool alreadyIn(QString moduleID);
		void setDirty(bool dirty=true);

		const QString &fileName();

	public slots:
		void addPartCommand(const QString& moduleID);
		void removePartCommand(const QString& moduleID);
		void removeAlienParts();
		bool save();
		bool open(QString fileName="");
		void openCore();
		void itemMoved();

	protected slots:
		void toIconView();
		void toListView();
		bool removeSelected();
		bool saveAs();
		void undoStackCleanChanged(bool isClean);
		void saveAsLastBin();
		void addPart();
		void exportPart();
		void newBin();
		void openBin();
		void openCoreBin();
		void rename();
		void closeBin();
		void updateButtonStates();

	signals:
		void saved(bool hasPartsFromBundled);
		void fileNameUpdated(PartsBinPaletteWidget*, const QString &newFileName, const QString &oldFilename);
		void savePartAsBundled(const QString &moduleId);

	protected:
		void closeEvent(QCloseEvent* event);

		void setupFooter();
		void setupButtons();

		void grabTitle(PaletteModel *model);

		void setView(class PartsBinView *view);
		void saveAsAux(const QString &filename);

		void afterModelSetted(PaletteModel *model);
		void createMenu();

	protected:
		PaletteModel *m_model;
		ReferenceModel *m_refModel;
		bool m_canDeleteModel;
		bool m_orderHasChanged;

		QString m_fileName;
		QString m_defaultSaveFolder;
		QString m_untitledFileName;

		//class SimpleEditableLabelWidget *m_binTitle;
		QString m_title;

		PartsBinView *m_currentView;
		class PartsBinIconView *m_iconView;
		class PartsBinListView *m_listView;

		QFrame *m_footer;

		ImageButton *m_showIconViewButton;
		ImageButton *m_showListViewButton;
		ImageButton *m_removeSelectedButton;
		/*ImageButton *m_openBinButton;
		ImageButton *m_saveBinButton;
		ImageButton *m_coreBinButton;*/
		ImageButton *m_addPartButton;
		ImageButton *m_exportPartButton;

		QToolButton *m_menuButton;
		QAction *m_newBinAction;
		QAction *m_openBinAction;
		QAction *m_openCoreBinAction;
		QAction *m_closeBinAction;
		QAction *m_saveAction;
		QAction *m_saveAsAction;
		QAction *m_renameAction;

		WaitPushUndoStack *m_undoStack;
		BinManager *m_manager;
		StackTabWidget *m_tabWidget;

		QStringList m_alienParts;

	public:
		static QString Title;
};

#endif /* PARTSBINPALETTEWIDGET_H_ */
