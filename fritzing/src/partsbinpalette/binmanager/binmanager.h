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

$Revision: 2776 $:
$Author: merunga $:
$Date: 2009-04-02 13:54:08 +0200 (Thu, 02 Apr 2009) $

********************************************************************/


#ifndef BINMANAGER_H_
#define BINMANAGER_H_

#include <QStackedWidget>
#include <QTabWidget>

#include "stackwidget.h"

class ModelPart;
class PaletteModel;

class BinManager : public QFrame {
	Q_OBJECT
	public:
		BinManager(class ReferenceModel *refModel, class HtmlInfoView *infoView, class WaitPushUndoStack *undoStack, QWidget* parent = 0);
		virtual ~BinManager();

		void loadFromModel(PaletteModel *model);
		void setPaletteModel(PaletteModel *model);

		void addBin(class PartsBinPaletteWidget* bin);
		void insertBin(PartsBinPaletteWidget* bin, int index, StackTabWidget* tb);
		void addPart(ModelPart *modelPart, int position = -1);
		void addNewPart(ModelPart *modelPart);

		bool beforeClosing();

		bool hasAlienParts();
		QString createIfMyPartsNotExists();

		void setInfoViewOnHover(bool infoViewOnHover);
		void load(const QString&);

		void setDirtyTab(PartsBinPaletteWidget* w, bool dirty=true);
		void updateTitle(PartsBinPaletteWidget* w, const QString& newTitle);

		PartsBinPaletteWidget* newBinIn(StackTabWidget* tb);
		PartsBinPaletteWidget* openBinIn(StackTabWidget* tb, QString fileName="");
		PartsBinPaletteWidget* openCoreBinIn(StackTabWidget* tb);
		void closeBinIn(StackTabWidget* tb);

	public slots:
		void addPartCommand(const QString& moduleID);
		void removeAlienParts();

	signals:
		void saved(bool hasPartsFromBundled);

	protected:
		void createMenu();
		PartsBinPaletteWidget* currentBin(StackTabWidget* tb);

		ReferenceModel *m_refModel;
		PaletteModel *m_paletteModel;
		HtmlInfoView *m_infoView;
		WaitPushUndoStack *m_undoStack;

		StackWidget *m_widget;
		QTabWidget *m_activeBinTabWidget;

		QHash<PartsBinPaletteWidget*,StackTabWidget*> m_tabWidgets;
		QHash<QString /*filename*/,PartsBinPaletteWidget*> m_openedBins;
		int m_unsavedBinsCount;
		QString m_defaultSaveFolder;

	public:
		static QString Title;
		static QString MyPartsBinLocation;
};

#endif /* BINMANAGER_H_ */
