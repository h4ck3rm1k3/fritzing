/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/



#include<QLineEdit>
#include<QKeyEvent>
#include<QFile>
#include<QTextStream>
#include<QTimer>

#include "zoomcombobox.h"

QStringList ZoomComboBox::ZoomFactors;
qreal ZoomComboBox::ZoomStep;

ZoomComboBox::ZoomComboBox(QWidget * parent) : QComboBox(parent){
	this->setEditable(true);
	QRegExp regexp("[1-9][\\d]{0,3}(\\.[\\d]{1,2})?\\%?");
	this->setValidator(new QRegExpValidator(regexp,this));
	this->setAutoCompletion(false);
	this->addItems(ZoomComboBox::ZoomFactors);
	setFixedWidth(100);

	m_userStillWriting = false;

	connect(lineEdit(),SIGNAL(returnPressed()),this,SLOT(itemAdded()));
	connect(this,SIGNAL(editTextChanged(QString)),this,SLOT(inputTextChanged()));
	connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(updateBackupFieldsIfOptionSelected(int)));

	m_valueBackup = editText();
	m_indexBackup = itemIndex(m_valueBackup);
}

void ZoomComboBox::itemAdded() {
	qreal lineEditValue = lineEdit()->text().toFloat();

	addPercentageToInputText();

	// Sort zoom items
	QList<qreal> options;
	for(int i=0; i < count(); i++) {
		QString text = itemText(i);
		text = text.trimmed();
		text.remove("%");
		options << text.toFloat();
	}

	qSort(options);

	QStringList strOptions;
	int selIndex = -1;
	for(int i=0; i<options.size(); i++) {
		qreal opt = options.at(i);
		if(opt == lineEditValue) {
			selIndex = i;
		}
		strOptions << QString::number(opt)+"%";
	}

	clear();
	addItems(strOptions);
	setCurrentIndex(selIndex);
}

QString ZoomComboBox::editText() {
	return lineEdit()->text();
}

void ZoomComboBox::setEditText(QString newText) {
	// hack to avoid delay
	int prevKeyPressed = m_lastKeyPressed;
	m_lastKeyPressed = Qt::Key_Escape;

	m_indexBackup = itemIndex(newText);
	m_valueBackup = newText;
	setCurrentIndex(m_indexBackup);
	lineEdit()->insert(m_valueBackup);

	m_lastKeyPressed = prevKeyPressed;
}

void ZoomComboBox::inputTextChanged() {
	if(!m_userStillWriting) {
		int msec = m_lastKeyPressed == Qt::Key_Escape || itemIndex(editText()) != -1 ? 1 : 1000;
		QTimer::singleShot(msec, this, SLOT(emitZoomChanged()));
		m_userStillWriting = true;
	}
}

void ZoomComboBox::emitZoomChanged() {
	QString newText = editText();
	if(newText.trimmed() != "") {
		emit zoomChanged(newText.remove("%").toFloat());
	}
	m_userStillWriting = false;
}

int ZoomComboBox::itemIndex(QString value) {
	int retval = -1;
	for(int i = 0; i < count(); i++) {
		if(itemText(i) == value) {
			retval = i;
			break;
		}
	}
	return retval;
}

void ZoomComboBox::addPercentageToInputText() {
	QLineEdit * le = this->lineEdit();
	if(le->text().indexOf("%") == -1) {
		le->setText(le->text()+"%");
	}
}

void ZoomComboBox::focusOutEvent ( QFocusEvent * event ) {
	if(lineEdit()->text().trimmed() == "") {
		setPreviousValue();
	} else {
		addPercentageToInputText();
		updateBackupFields();
		QComboBox::focusOutEvent(event);
	}
}

void ZoomComboBox::updateBackupFieldsIfOptionSelected(int index) {
	if(index != -1) {
		updateBackupFields();
	}
}

void ZoomComboBox::updateBackupFields() {
	QString newText = editText();
	m_valueBackup = newText;
	m_indexBackup = itemIndex(newText);
	setCurrentIndex(m_indexBackup);
	if(m_indexBackup == -1) {
		// setCurrentIndex erases the input text value if the index is -1
		setEditText(m_valueBackup);
	}
}

void ZoomComboBox::keyPressEvent ( QKeyEvent * event ) {
	if(event->key() == Qt::Key_Escape) {
		setPreviousValue();
		m_lastKeyPressed = Qt::Key_Escape;
	}
	QComboBox::keyPressEvent(event);
}

void ZoomComboBox::setPreviousValue() {
	setCurrentIndex(m_indexBackup);
	setEditText(m_valueBackup);
}

void ZoomComboBox::zoomIn () {
	int selIndex = currentIndex();
	if(selIndex == -1) {
		setCurrentIndex(findCloserIndexToCurrentValue(true));
		return;
	}
	if(selIndex < count()-1) {
		selIndex++;
		setCurrentIndex(selIndex);
	}
}


void ZoomComboBox::zoomOut () {
	int selIndex = currentIndex();
	if(selIndex == -1) {
		setCurrentIndex(findCloserIndexToCurrentValue(false));
		return;
	}
	if(selIndex > 0) {
		selIndex--;
		setCurrentIndex(selIndex);
	}
}

int ZoomComboBox::findCloserIndexToCurrentValue(bool upper) {
	// FIXME: could be don with a binary search, because we assume that the option list is ordered
	qreal value = editText().remove("%").toFloat();
	qreal min = 9999999;
	int idx = -1;
	for(int i=0; i<count(); i++) {
		qreal curValue = itemText(i).remove("%").toFloat();
		qreal curDiff = qAbs(curValue - value);
		if(curValue > value && upper) {
			if(curDiff < min) {
				min = curDiff;
				idx = i;
				break;
			}
		} else if(curValue < value && !upper) {
			if(curDiff < min) {
				idx = i;
			}
		}
	}

	return idx;
}

void ZoomComboBox::loadFactors() {
	QFile file(":/resources/zoomfactors.txt");
	file.open(QFile::ReadOnly);
	QTextStream stream( &file );
	int lineNumber = 1;
	while(!stream.atEnd()) {
		QString line = stream.readLine();
		if(lineNumber != 1) {
			ZoomFactors << line+"%";
		} else {
			ZoomStep = line.toFloat();
		}
		lineNumber++;
	}
	file.close();
}
