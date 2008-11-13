/*
 * (c) Fachhochschule Potsdam
 */

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
