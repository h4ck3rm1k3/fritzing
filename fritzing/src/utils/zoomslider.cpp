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

$Revision$:
$Author$:
$Date$

********************************************************************/

#include <QLineEdit>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QHBoxLayout>
#include <QValidator>
#include <QLabel>
#include <limits>

#include "zoomslider.h"

qreal ZoomSlider::ZoomStep;
QList<qreal> ZoomSlider::ZoomFactors;

static const int MIN_VALUE = 10;
static const int MAX_VALUE = 2010;
static const int STARTING_VALUE = 100;
static const int HEIGHT = 16;
static const int STEP = 100;

ZoomSlider::ZoomSlider(QWidget * parent) : QFrame(parent) 
{
	// layout doesn't seem to work: the slider appears too far down in the status bar
	// because the status bar layout is privileged for the message text

	if (ZoomFactors.size() == 0) {
		loadFactors();
	}

	this->setStyleSheet("border:0px; margin:0px; padding:0px;");

	int soFar = 0;
	QLabel * label = new QLabel(tr("Zoom %:"), this);
	label->setObjectName("ZoomSliderLabel");
	label->setGeometry(soFar, 0, 50, HEIGHT);
	label->setAlignment(Qt::AlignRight);
	soFar += 50 + 2;

	m_lineEdit = new QLineEdit(this);
	m_lineEdit->setGeometry(soFar, -1, 35, HEIGHT - 1);
	m_lineEdit->setText(QString("%1").arg(STARTING_VALUE));
	m_lineEdit->setValidator(new QIntValidator(MIN_VALUE, MAX_VALUE, this));
	soFar += 35 + 5;

	QPixmap temp(":/resources/images/icons/zoomSliderMinus.png");
	m_minusButton = new QPushButton(this);
	m_minusButton->setAutoRepeat(true);
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
	m_plusButton->setAutoRepeat(true);
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

void ZoomSlider::loadFactors() {
	QFile file(":/resources/zoomfactors.txt");
	file.open(QFile::ReadOnly);
	QTextStream stream( &file );
	int lineNumber = 0;
	while(!stream.atEnd()) {
		QString line = stream.readLine();
		if(lineNumber != 0) {
			ZoomFactors << line.toDouble();
		} 
		else 
		{
			ZoomStep = line.toDouble();
		}
		lineNumber++;
	}
	file.close();
}


void ZoomSlider::setValue(qreal value) {
	QString newText = QString("%1").arg(qRound(value));
	m_lineEdit->setText(newText);
	sliderTextEdited(newText, false);
}

qreal ZoomSlider::value() {
	return m_lineEdit->text().toDouble();
}

void ZoomSlider::minusClicked() {
	step(-1);
}

void ZoomSlider::plusClicked() {
	step(1);
}

void ZoomSlider::step(int direction) {
	int minIndex = 0;
	qreal minDiff = std::numeric_limits<double>::max();
	qreal v = value();
	for (int i = 0; i < ZoomFactors.count(); i++) {
		qreal f = ZoomFactors[i];
		if (qAbs(f - v) < minDiff) {
			minDiff = qAbs(f - v);
			minIndex = i;
		}
	}

	minIndex += direction;
	if (minIndex < 0) {
		minIndex = 0;
	}
	else if (minIndex >= ZoomFactors.count()) {
		minIndex = ZoomFactors.count() - 1;
	}

	setValue(ZoomFactors[minIndex]);
	emit zoomChanged(ZoomFactors[minIndex]);
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
	sliderTextEdited(newText, true);
}

void ZoomSlider::sliderTextEdited(const QString & newText, bool doEmit) 
{
	int value = newText.toInt();
	if (m_slider->value() != value) {
		disconnect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
		m_slider->setValue(value);
		connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
		if (doEmit) emit zoomChanged(value);
	}
}

void ZoomSlider::zoomIn () {
	plusClicked();
}

void ZoomSlider::zoomOut () {
	minusClicked();
}


/*


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

*/
