/*
 * (c) Fachhochschule Potsdam
 */

#include "editablelinewidget.h"

EditableLineWidget::EditableLineWidget(QString text, WaitPushUndoStack *undoStack, QWidget *parent, QString title, bool edited, bool noSpacing)
	: AbstractEditableLabelWidget(text, undoStack, parent, title, edited, noSpacing) {
	m_lineEdit = new QLineEdit(this);
	toStandardMode();
}

void EditableLineWidget::setValidator(const QValidator * v ) {
	m_lineEdit->setValidator(v);
}

QString EditableLineWidget::editionText() {
	return m_lineEdit->text();
}
void EditableLineWidget::setEditionText(QString text) {
	m_lineEdit->setText(text);
}
QWidget* EditableLineWidget::myEditionWidget() {
	return m_lineEdit;
}

void EditableLineWidget::setEmptyTextToEdit() {
	m_lineEdit->setText("");
}
