/*
 * (c) Fachhochschule Potsdam
 */

#include "editablelabel.h"

EditableLabel::EditableLabel(QWidget *parent) : QLabel(parent) {

}

EditableLabel::EditableLabel(const QString &text, QWidget *parent) : QLabel(text,parent) {

}

void EditableLabel::editionCompleted(QString newText) {
	setText(newText);
}

void EditableLabel::mousePressEvent (QMouseEvent *event) {
	emit editionStarted(text());

	QWidget::mousePressEvent(event);
}
