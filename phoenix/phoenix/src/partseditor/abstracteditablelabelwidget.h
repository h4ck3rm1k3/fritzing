/*
 * (c) Fachhochschule Potsdam
 */

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QGridLayout>

#include "editablelabel.h"
#include "../waitpushundostack.h"

#ifndef ABSTRACTEDITABLELABELWIDGET_H_
#define ABSTRACTEDITABLELABELWIDGET_H_

class AbstractEditableLabelWidget : public QFrame {
Q_OBJECT
	public:
		AbstractEditableLabelWidget(QString text, WaitPushUndoStack *undoStack, QWidget *parent=0, QString title="", bool edited=false, bool noSpacing=false);
		QString text();

	protected slots:
		void editionStarted(QString text);
		void informEditionCompleted();
		void editionCanceled();

	signals:
		void editionCompleted(QString text);
		void editionStarted();
		void editionFinished();

	protected:
		void toStandardMode();
		void toEditionMode();
		void setNoSpacing(QLayout *layout);

		virtual QString editionText()=0;
		virtual void setEditionText(QString text)=0;
		virtual QWidget* myEditionWidget()=0;
		virtual void setEmptyTextToEdit()=0;

		QLabel *m_title;
		EditableLabel *m_label;

		QPushButton *m_acceptButton;
		QPushButton *m_cancelButton;

		class WaitPushUndoStack * m_undoStack;

		bool m_noSpacing;
		bool m_edited;
		volatile bool m_isInEditionMode;
};

#endif /* ABSTRACTEDITABLELABELWIDGET_H_ */
