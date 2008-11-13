/*
 * (c) Fachhochschule Potsdam
 */

#include "editabledatewidget.h"

EditableDateWidget::EditableDateWidget(QDate date, WaitPushUndoStack *undoStack, QWidget *parent, QString title, bool edited, bool noSpacing)
	: AbstractEditableLabelWidget(date.toString(Qt::ISODate), undoStack, parent, title, edited, noSpacing) {
	m_dateEdit = new QDateEdit(this);
	toStandardMode();
}

QString EditableDateWidget::editionText() {
	return m_dateEdit->date().toString(Qt::ISODate);
}
void EditableDateWidget::setEditionText(QString text) {
	m_dateEdit->setDate(QDate::fromString(text, Qt::ISODate));
}
QWidget* EditableDateWidget::myEditionWidget() {
	return m_dateEdit;
}

void EditableDateWidget::setEmptyTextToEdit() {
	m_dateEdit->setDate(QDate());
}
