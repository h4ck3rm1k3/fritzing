/*
 * (c) Fachhochschule Potsdam
 */

#include "editabletextwidget.h"

EditableTextWidget::EditableTextWidget(QString text, WaitPushUndoStack *undoStack, QWidget *parent, QString title, bool edited, bool noSpacing)
	: AbstractEditableLabelWidget(text, undoStack, parent, title, edited, noSpacing) {
	m_textEdit = new QTextEdit(this);
	m_textEdit->setFixedHeight(70);
	toStandardMode();
}

QString EditableTextWidget::editionText() {
	return m_textEdit->toPlainText();
}
void EditableTextWidget::setEditionText(QString text) {
	m_textEdit->setText(text);
}
QWidget* EditableTextWidget::myEditionWidget() {
	return m_textEdit;
}

void EditableTextWidget::setEmptyTextToEdit() {
	m_textEdit->setText("");
}
