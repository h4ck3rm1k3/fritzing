/*
 * (c) Fachhochschule Potsdam
 */

#include "simpleeditablelabelwidget.h"
#include "../debugdialog.h"

#include <QHBoxLayout>
#include <QKeyEvent>

SimpleEditableLabelWidget::SimpleEditableLabelWidget(QUndoStack *undoStack, QWidget *parent, const QString &text, bool edited)
	: QFrame(parent)
{
	setObjectName("partsBinTitle");
	m_label = new QLabel(this);
	m_lineEdit = new QLineEdit(this);
	connect(m_lineEdit,SIGNAL(editingFinished()),this,SLOT(toStandardMode()));

	QHBoxLayout *lo = new QHBoxLayout(this);
	lo->setMargin(3);
	lo->setSpacing(0);

	m_hasBeenEdited = edited;
	m_isInEditionMode = false;
	m_label->setText(text);

	m_undoStack = undoStack;
	updateUndoStackIfNecessary();

	toStandardMode(edited);
}

QString SimpleEditableLabelWidget::text() {
	return m_label->text();
}

void SimpleEditableLabelWidget::setText(const QString &text, bool markAsEdited) {
	if(m_label->text() != text) {
		m_label->setText(text);
		m_hasBeenEdited = markAsEdited;
		updateUndoStackIfNecessary();
	}
}

void SimpleEditableLabelWidget::toStandardMode(bool markAsEdited) {
	setText(m_lineEdit->text(), markAsEdited);
	swapWidgets(m_label, m_lineEdit);
}

void SimpleEditableLabelWidget::toEditionMode() {
	//if(m_hasBeenEdited) {
		m_lineEdit->setText(m_label->text());
	//} else { // Remove this part of the branch if we want the lineedit to remember what was typed the last time
		//m_lineEdit->setText("");
	//}
	swapWidgets(m_lineEdit, m_label);
	m_lineEdit->setFocus();
}

void SimpleEditableLabelWidget::swapWidgets(QWidget *toShow, QWidget *toHide) {
	layout()->removeWidget(toHide);
	toHide->hide();

	layout()->addWidget(toShow);
	toShow->show();

	m_isInEditionMode = (toShow == m_lineEdit);
}

void SimpleEditableLabelWidget::swapMode() {
	if(m_isInEditionMode) {
		toStandardMode();
	} else {
		toEditionMode();
	}
}

void SimpleEditableLabelWidget::mousePressEvent(QMouseEvent *event) {
	if(!m_isInEditionMode) {
		swapMode();
	}
	QFrame::mousePressEvent(event);
}

void SimpleEditableLabelWidget::keyPressEvent(QKeyEvent *event) {
	if(m_isInEditionMode && event->key() == Qt::Key_Escape) {
		QString prevText = m_label->text();
		toStandardMode();
		setText(prevText);
	}
	QFrame::keyPressEvent(event);
}

void SimpleEditableLabelWidget::updateUndoStackIfNecessary() {
	if(m_hasBeenEdited) {
		m_undoStack->push(new QUndoCommand("Palette Widget title modified"));
	}
}
