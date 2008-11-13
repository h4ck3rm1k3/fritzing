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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/


#ifndef PARTSBINPALETTEWIDGET_H_
#define PARTSBINPALETTEWIDGET_H_

#include <QFrame>

#include "../fdockwidget.h"
#include "../palettemodel.h"
#include "../modelpart.h"
#include "../htmlinfoview.h"
#include "partsbiniconview.h"
#include "partsbinlistview.h"
#include "simpleeditablelabelwidget.h"

class ImageButton : public QLabel {
	Q_OBJECT
	public:
		ImageButton(QWidget *parent=0) : QLabel(parent) {};

	signals:
		void clicked();

	protected:
		void mouseReleaseEvent(QMouseEvent * event) {
			if(isEnabled()) {
				emit clicked();
			}
			QLabel::mouseReleaseEvent(event);
		}
};

// TODO Mariano post alpha: create a real Bin entity and make this widget wrap it
class PartsBinPaletteWidget : public FDockWidget {
	Q_OBJECT
	public:
		PartsBinPaletteWidget(HtmlInfoView *infoView, QWidget* parent = 0);
		~PartsBinPaletteWidget();

		QSize sizeHint() const;

		void loadFromModel(PaletteModel *model);
		void setPaletteModel(PaletteModel *model, bool clear=false);
		void addPart(ModelPart *modelPart);

		bool currentBinIsCore();
		bool beforeClosing();

		PaletteItem * selected();

	protected slots:
		void toIconView();
		void toListView();
		bool save();
		bool saveAs();
		void open();
		void openCore();
		void undoStackCleanChanged(bool isClean);

	protected:
		void closeEvent(QCloseEvent* event);

		void setupFooter();
		void setupButtons();
		void setupPixmaps();

		void grabTitle(PaletteModel *model);
		void load(const QString&);

		void setView(PartsBinView *view, QPixmap *showIconPixmap, QPixmap *showListPixmap);
		void saveAsAux(const QString &filename);

		void afterModelSetted(PaletteModel *model);
		void setSaveButtonEnabled(bool enabled);
		void saveAsLastBin();

		bool alreadyIn(QString moduleID);

	protected:
		PaletteModel *m_model;
		QString m_fileName;
		QString m_defaultSaveFolder;
		QString m_untitledFileName;

		SimpleEditableLabelWidget *m_binTitle;

		PartsBinView *m_currentView;
		PartsBinIconView *m_iconView;
		PartsBinListView *m_listView;

		QFrame *m_container;

		QFrame *m_footer;
		ImageButton *m_showIconViewButton;
		ImageButton *m_showListViewButton;
		ImageButton *m_openBinButton;
		ImageButton *m_saveBinButton;
		ImageButton *m_coreBinButton;

		QPixmap *m_iconViewActive;
		QPixmap *m_iconViewInactive;
		QPixmap *m_listViewActive;
		QPixmap *m_listViewInactive;
		QPixmap *m_saveButtonEnabled;
		QPixmap *m_saveButtonDisabled;

		QUndoStack *m_undoStack;
};

#endif /* PARTSBINPALETTEWIDGET_H_ */
