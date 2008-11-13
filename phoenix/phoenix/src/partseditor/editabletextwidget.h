/*
 * (c) Fachhochschule Potsdam
 */

#ifndef EDITABLETEXTWIDGET_H_
#define EDITABLETEXTWIDGET_H_

#include <QTextEdit>

#include "abstracteditablelabelwidget.h"
#include "../modelpartstuff.h"

class EditableTextWidget : public AbstractEditableLabelWidget {
Q_OBJECT
	public:
		EditableTextWidget(QString text, WaitPushUndoStack *undoStack, QWidget *parent=0, QString title="", bool noSpacing=false, bool edited=false);

	protected:
		QString editionText();
		void setEditionText(QString text);
		QWidget* myEditionWidget();
		void setEmptyTextToEdit();

		QTextEdit *m_textEdit;
};

#endif /* EDITABLETEXTWIDGET_H_ */
