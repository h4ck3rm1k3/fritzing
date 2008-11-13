/*
 * (c) Fachhochschule Potsdam
 */

#ifndef EDITABLEDATEWIDGET_H_
#define EDITABLEDATEWIDGET_H_

#include "abstracteditablelabelwidget.h"
#include "../modelpartstuff.h"

#include <QDateEdit>

class EditableDateWidget : public AbstractEditableLabelWidget {
Q_OBJECT
	public:
		EditableDateWidget(QDate date, WaitPushUndoStack *undoStack, QWidget *parent=0, QString title="", bool noSpacing=false, bool edited=false);

	protected:
		QString editionText();
		void setEditionText(QString text);
		QWidget* myEditionWidget();
		void setEmptyTextToEdit();

		QDateEdit *m_dateEdit;
};

#endif /* EDITABLEDATEWIDGET_H_ */
