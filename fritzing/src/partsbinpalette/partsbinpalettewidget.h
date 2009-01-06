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

#include "../fdockwidget.h"
#include "../palettemodel.h"
#include "../modelpart.h"
#include "../htmlinfoview.h"
#include "../waitpushundostack.h"
#include "../abstractimagebutton.h"
#include "partsbiniconview.h"
#include "partsbinlistview.h"
#include "simpleeditablelabelwidget.h"

class ImageButton : public AbstractImageButton {
public:
	ImageButton(const QString &imageName, QWidget *parent=0)
		: AbstractImageButton(parent)
	{
		setupIcons(imageName);
	};
protected:
	QString imagePrefix() {
		return ":/resources/images/icons/partsBin";
	}
};

class PartsBinPaletteWidget : public QFrame {
	Q_OBJECT
	public:
		PartsBinPaletteWidget(ReferenceModel *refModel, HtmlInfoView *infoView, WaitPushUndoStack *undoStack, QWidget* parent = 0);

		QSize sizeHint() const;

		void loadFromModel(PaletteModel *model);
		void setPaletteModel(PaletteModel *model, bool clear=false);

		void addPart(ModelPart *modelPart, int position = -1);

		bool currentBinIsCore();
		bool beforeClosing();

		ModelPart * selected();
		bool hasAlienParts();

		void setInfoViewOnHover(bool infoViewOnHover);
		void addPart(const QString& moduleID, int position = -1);
		void removePart(const QString& moduleID);

	public slots:
		void addPartCommand(const QString& moduleID);
		void removePartCommand(const QString& moduleID);
		void removeAlienParts();

	protected slots:
		void toIconView();
		void toListView();
		bool removeSelected();
		bool save();
		bool saveAs();
		void open();
		void openCore();
		void undoStackCleanChanged(bool isClean);

	signals:
		void saved(bool hasPartsFromBundled);

	protected:
		void closeEvent(QCloseEvent* event);

		void setupFooter();
		void setupButtons();

		void grabTitle(PaletteModel *model);
		void load(const QString&);

		void setView(PartsBinView *view);
		void saveAsAux(const QString &filename);

		void afterModelSetted(PaletteModel *model);
		void setSaveButtonEnabled(bool enabled);
		void saveAsLastBin();

		bool alreadyIn(QString moduleID);

	protected:
		PaletteModel *m_model;
		ReferenceModel *m_refModel;

		QString m_fileName;
		QString m_defaultSaveFolder;
		QString m_untitledFileName;

		SimpleEditableLabelWidget *m_binTitle;

		PartsBinView *m_currentView;
		PartsBinIconView *m_iconView;
		PartsBinListView *m_listView;

		QFrame *m_footer;
		ImageButton *m_showIconViewButton;
		ImageButton *m_showListViewButton;
		ImageButton *m_removeSelected;
		ImageButton *m_openBinButton;
		ImageButton *m_saveBinButton;
		ImageButton *m_coreBinButton;

		WaitPushUndoStack *m_undoStack;

		QStringList m_alienParts;

	public:
		static QString Title;
};

#endif /* PARTSBINPALETTEWIDGET_H_ */
