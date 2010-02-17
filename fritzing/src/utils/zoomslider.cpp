/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 3930 $:
$Author: cohen@irascible.com $:
$Date: 2010-01-31 00:36:25 +0100 (Sun, 31 Jan 2010) $

********************************************************************/

// TODO:
//	+ and - buttons: autorepeat; better art
//  menu functions: zoom in, zoom out, etc
//	load the change values from the zoom combo box?

#include <QLineEdit>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QHBoxLayout>
#include <QValidator>
#include <QLabel>

#include "zoomslider.h"

//qreal ZoomComboBox::ZoomStep;

static const int MIN_VALUE = 10;
static const int MAX_VALUE = 2010;
static const int STARTING_VALUE = 100;
static const int HEIGHT = 16;
static const int STEP = 100;

ZoomSlider::ZoomSlider(QWidget * parent) : QFrame(parent) 
{
	// layout doesn't seem to work: the slider appears too far down in the status bar
	// because the status bar layout is privileged for the message text

	this->setStyleSheet("border:0px; margin:0px; padding:0px;");

	int soFar = 0;
	QLabel * label = new QLabel(tr("Zoom:"), this);
	label->setObjectName("ZoomSliderLabel");
	label->setGeometry(soFar, 0, 40, HEIGHT);
	label->setAlignment(Qt::AlignRight);
	soFar += 40 + 2;

	m_lineEdit = new QLineEdit(this);
	m_lineEdit->setGeometry(soFar, -1, 35, HEIGHT - 1);
	m_lineEdit->setText(QString("%1").arg(STARTING_VALUE));
	m_lineEdit->setValidator(new QIntValidator(MIN_VALUE, MAX_VALUE, this));
	soFar += 35 + 5;

	QPixmap temp(":/resources/images/icons/zoomSliderMinus.png");
	m_minusButton = new QPushButton(this);
	m_minusButton->setObjectName("ZoomSliderMinusButton");
	m_minusButton->setGeometry(soFar, 0, temp.width(), temp.height());
	connect(m_minusButton, SIGNAL(clicked()), this, SLOT(minusClicked()));
	soFar += temp.width() + 5;

	m_slider = new QSlider(this);
	m_slider->setOrientation(Qt::Horizontal);
	m_slider->setRange(MIN_VALUE, MAX_VALUE);
	m_slider->setValue(STARTING_VALUE);
	m_slider->setGeometry(soFar, 0, 100, HEIGHT);
	soFar += 100 + 5;

	m_plusButton = new QPushButton(this);
	m_plusButton->setObjectName("ZoomSliderPlusButton");
	m_plusButton->setGeometry(soFar, 0, temp.width(), temp.height());
	connect(m_plusButton, SIGNAL(clicked()), this, SLOT(plusClicked()));
	soFar += temp.width() + 5;

	this->setFixedWidth(soFar);
	this->setFixedHeight(HEIGHT);

	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
	connect(m_lineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(sliderTextEdited(const QString &)));

	//m_userStillWriting = false;

	//m_valueBackup = editText();
	//m_indexBackup = itemIndex(m_valueBackup);
}

void ZoomSlider::setValue(qreal value) {
	QString newText = QString("%1").arg(qRound(value));
	m_lineEdit->setText(newText);
	sliderTextEdited(newText);
}

qreal ZoomSlider::value() {
	return m_lineEdit->text().toDouble();
}

void ZoomSlider::minusClicked() {
	step(-STEP);
}

void ZoomSlider::plusClicked() {
	step(STEP);
}

void ZoomSlider::step(qreal inc) {
	qreal v = value() + inc;
	v = qRound(v / STEP) * STEP;
	if (v > MAX_VALUE) {
		v = MAX_VALUE;
	}
	if (v < MIN_VALUE) {
		v = MIN_VALUE;
	}
	setValue(v);
	emit zoomChanged(v);
}

void ZoomSlider::sliderValueChanged(int newValue) {
	newValue = (newValue / 10) * 10;					// make it so moving the slider outputs nice rounded values
	QString newText = QString("%1").arg(newValue);
	if (newText.compare(m_lineEdit->text()) != 0) {
		m_lineEdit->setText(newText);
		emit zoomChanged(newValue);
	}
}

void ZoomSlider::sliderTextEdited(const QString & newText) {
	int value = newText.toInt();
	if (m_slider->value() != value) {
		disconnect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
		m_slider->setValue(value);
		connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
		emit zoomChanged(value);
	}
}

/*
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

void ZoomComboBox::updateBackupFieldsIfOptionSelected(int index) {
	if(index != -1) {
		updateBackupFields();
	}
}

void ZoomComboBox::updateBackupFields() {
	QString newText = editText();
	m_valueBackup = newText;
	m_indexBackup = itemIndex(newText);
	
	disconnect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(updateBackupFieldsIfOptionSelected(int)));
	disconnect(this,SIGNAL(editTextChanged(QString)),this,SLOT(inputTextChanged()));	
	setCurrentIndex(m_indexBackup);
	
	if(m_indexBackup == -1) {
		// setCurrentIndex erases the input text value if the index is -1
		setEditText(m_valueBackup);
	}
	connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(updateBackupFieldsIfOptionSelected(int)));
	connect(this,SIGNAL(editTextChanged(QString)),this,SLOT(inputTextChanged()));
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
*/
