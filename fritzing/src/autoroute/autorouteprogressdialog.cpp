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

#include "autorouteprogressdialog.h"
#include "../debugdialog.h"
#include "../partseditor/zoomcontrols.h"

#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLocale>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QScrollBar>

static const int ScrollAmount = 40;

ArrowButton::ArrowButton(int scrollX, int scrollY, ZoomableGraphicsView * view, const QString & path) : QLabel() {
	m_scrollX = scrollX;
	m_scrollY = scrollY;
	m_view = view;
	setPixmap(QPixmap(path));
}

void ArrowButton::mousePressEvent(QMouseEvent *event) {
	Q_UNUSED(event);
	if (m_scrollX != 0) {
		QScrollBar * scrollBar = m_view->horizontalScrollBar();
		scrollBar->setValue(scrollBar->value() + m_scrollX);
	}
	else if (m_scrollY != 0) {
		QScrollBar * scrollBar = m_view->verticalScrollBar();
		scrollBar->setValue(scrollBar->value() + m_scrollY);
	}
}


/////////////////////////////////////

AutorouteProgressDialog::AutorouteProgressDialog(ZoomableGraphicsView * view, QWidget *parent) : QDialog(parent) 
{
	Qt::WindowFlags flags = windowFlags();
	flags ^= Qt::WindowCloseButtonHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	this->setWindowTitle(QObject::tr("Autorouting Progress..."));

	QVBoxLayout * vLayout = new QVBoxLayout(this);

	m_progressBar = new QProgressBar(this);
	vLayout->addWidget(m_progressBar);

	QGroupBox * groupBox = new QGroupBox(tr("zoom and pan"));
	QHBoxLayout *lo2 = new QHBoxLayout(groupBox);
	lo2->setSpacing(1);
	lo2->setMargin(0);
	lo2->addWidget(new ZoomControls(view, groupBox));

	lo2->addSpacerItem(new QSpacerItem ( 10, 0, QSizePolicy::Expanding));

	QFrame * frame = new QFrame();
	QGridLayout *gridLayout = new QGridLayout(frame);

	QString imgPath = ":/resources/images/icons/arrowButton%1.png";
	ArrowButton * label = new ArrowButton(0, ScrollAmount, view, imgPath.arg("Up"));
	gridLayout->addWidget(label, 0, 1);

	label = new ArrowButton(ScrollAmount, 0, view, imgPath.arg("Left"));
	gridLayout->addWidget(label, 1, 0);

	label = new ArrowButton(-ScrollAmount, 0, view, imgPath.arg("Right"));
	gridLayout->addWidget(label, 1, 2);

	label = new ArrowButton(0, -ScrollAmount, view, imgPath.arg("Down"));
	gridLayout->addWidget(label, 2, 1);

	lo2->addWidget(frame);

	vLayout->addWidget(groupBox);

	QPushButton * button = new QPushButton(tr("Skip current trace"), this);
	connect(button, SIGNAL(clicked()), this, SLOT(sendSkip()));
	vLayout->addWidget(button);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Stop Now"));

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(sendCancel()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(sendStop()));

	vLayout->addWidget(buttonBox);

	this->setLayout(vLayout);
}

AutorouteProgressDialog::~AutorouteProgressDialog() {
}

void AutorouteProgressDialog::setMinimum(int minimum) {
	m_progressBar->setMinimum(minimum);
}

void AutorouteProgressDialog::setMaximum(int maximum) {
	m_progressBar->setMaximum(maximum);
}

void AutorouteProgressDialog::setValue(int value) {
	m_progressBar->setValue(value);
}

void AutorouteProgressDialog::sendSkip() {
	emit skip();
}

void AutorouteProgressDialog::sendCancel() {
	emit cancel();
}

void AutorouteProgressDialog::sendStop() {
	emit stop();
}

void AutorouteProgressDialog::closeEvent(QCloseEvent *event)
{
	sendCancel();
	QDialog::closeEvent(event);
}

