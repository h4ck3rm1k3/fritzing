/*
 * (c) Fachhochschule Potsdam
 */

#ifndef EDITABLELINEWIDGET_H_
#define EDITABLELINEWIDGET_H_

#include <QLineEdit>

#include "abstracteditablelabelwidget.h"
#include "../modelpartstuff.h"

class EditableLineWidget : public AbstractEditableLabelWidget {
Q_OBJECT
	public:
		EditableLineWidget(QString text, WaitPushUndoStack *undoStack, QWidget *parent=0, QString title="", bool edited=false, bool noSpacing=false);
		void setValidator(const QValidator * v );

	protected:
		QString editionText();
		void setEditionText(QString text);
		QWidget* myEditionWidget();
		void setEmptyTextToEdit();

		QLineEdit *m_lineEdit;
};

#endif /* EDITABLELINEWIDGET_H_ */
