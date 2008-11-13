/*
 * (c) Fachhochschule Potsdam
 */

#ifndef SIMPLEEDITABLELABELWIDGET_H_
#define SIMPLEEDITABLELABELWIDGET_H_

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QUndoStack>

class SimpleEditableLabelWidget : public QFrame {
	Q_OBJECT
	public:
		SimpleEditableLabelWidget(QUndoStack *undoStack, QWidget *parent=0, const QString &text = "", bool edited=false);
		void setText(const QString &text, bool markAsEdited = true);
		QString text();

	protected slots:
		void toStandardMode(bool markAsEdited = true);
		void toEditionMode();

	protected:
		void swapWidgets(QWidget *toShow, QWidget *toHide);
		void swapMode();

		void mousePressEvent(QMouseEvent *event);
		void keyPressEvent(QKeyEvent *event);

		void updateUndoStackIfNecessary();

	protected:
		QLabel *m_label;
		QLineEdit *m_lineEdit;
		QUndoStack *m_undoStack;

		bool m_hasBeenEdited;
		volatile bool m_isInEditionMode;
};

#endif /* SIMPLEEDITABLELABELWIDGET_H_ */
