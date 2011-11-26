/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 5380 $:
$Author: cohen@irascible.com $:
$Date: 2011-08-10 00:15:35 +0200 (Wed, 10 Aug 2011) $

********************************************************************/

#include "pinlabeldialog.h"
#include "../debugdialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QPushButton>

// TODO:
//
//		undo
//		disable save and save-as until first undoable change
//		swap 
//		how does an edit work?

PinLabelDialog::PinLabelDialog(const QStringList & labels, bool singleRow, const QString & chipLabel, QWidget *parent)
	: QDialog(parent)
{
	m_labels = labels;
	this->setWindowTitle(QObject::tr("Pin Label Editor"));

	QVBoxLayout * vLayout = new QVBoxLayout(this);

	QScrollArea * scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QFrame * frame = new QFrame(this);
	QHBoxLayout * hLayout = new QHBoxLayout(frame);

	QFrame * labelsFrame = initLabels(labels, singleRow, chipLabel);

	QFrame * textFrame = new QFrame();
	QVBoxLayout * textLayout = new QVBoxLayout(frame);

	QLabel * label = new QLabel("<html><body>" +
								tr("<p><h2>Pin Label Editor</h2></p>") +
								tr("<p>Click on a label next to a pin to rename that pin.") + " " +
								tr("You can use the tab key to go through the labels.") + " " +
								tr("You can also use Ctrl/Cmd-Z for Undo.</p>") +
								tr("<p>Save vs. Save As...</p>") +		
								"</body></html>");
	label->setMaximumWidth(150);
	label->setWordWrap(true);

	textLayout->addWidget(label);
	textLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
	textFrame->setLayout(textLayout);

	hLayout->addWidget(labelsFrame);
	hLayout->addSpacing(15);
	hLayout->addWidget(textFrame);
	frame->setLayout(hLayout);

	scrollArea->setWidget(frame);	

	QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::SaveAll | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
	buttonBox->button(QDialogButtonBox::SaveAll)->setText(tr("Save as new part"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vLayout->addWidget(scrollArea);
	vLayout->addWidget(buttonBox);
	this->setLayout(vLayout);
}

PinLabelDialog::~PinLabelDialog()
{
}

QFrame * PinLabelDialog::initLabels(const QStringList & labels, bool singleRow, const QString & chipLabel)
{
	QFrame * frame = new QFrame();
	QVBoxLayout * vLayout = new QVBoxLayout();

	QLabel * label = new QLabel("<h2>" + chipLabel + "</h2>");
	label->setAlignment(Qt::AlignCenter);

	QFrame * subFrame = new QFrame();
	if (singleRow) {
		QGridLayout * gridLayout = new QGridLayout();
		gridLayout->setMargin(0);
		gridLayout->setSpacing(3);

		for (int i = 0; i < labels.count(); i++) {
			makeOnePinEntry(i, labels.at(i), Qt::AlignLeft, i, gridLayout);
		}

		subFrame->setLayout(gridLayout);
	}
	else {
		QHBoxLayout * hLayout = new QHBoxLayout();

		QFrame * lFrame = new QFrame();
		QGridLayout * lLayout = new QGridLayout;
		lLayout->setMargin(0);
		lLayout->setSpacing(3);
		for (int i = 0; i < labels.count() / 2; i++) {
			makeOnePinEntry(i, labels.at(i), Qt::AlignLeft, i, lLayout);
		}
		lFrame->setLayout(lLayout);

		QFrame * rFrame = new QFrame();
		QGridLayout * rLayout = new QGridLayout;
		rLayout->setMargin(0);
		rLayout->setSpacing(3);
		int row = labels.count() - 1;
		for (int i = labels.count() / 2; i < labels.count(); i++) {
			makeOnePinEntry(i, labels.at(i), Qt::AlignRight, row--, rLayout);
		}
		rFrame->setLayout(rLayout);

		hLayout->addWidget(lFrame);
		hLayout->addSpacing(15);
		hLayout->addWidget(rFrame);

		subFrame->setLayout(hLayout);
	}

	vLayout->addWidget(label);
	vLayout->addWidget(subFrame);
	frame->setLayout(vLayout);

	return frame;
}

void PinLabelDialog::makeOnePinEntry(int index, const QString & text, Qt::Alignment alignment, int row, QGridLayout * gridLayout) 
{
	QLineEdit * label = new QLineEdit();
	label->setText(QString::number(index + 1));
	label->setMaximumWidth(20);
	label->setMinimumWidth(20);
	label->setFrame(false);
	label->setEnabled(false);

	QLineEdit * lEdit = new QLineEdit();
	lEdit->setMaximumWidth(65);
	lEdit->setAlignment(alignment);
	lEdit->setText(text);
	lEdit->setProperty("index", index);
	connect(lEdit, SIGNAL(editingFinished()), this, SLOT(labelChanged()));

	if (alignment == Qt::AlignLeft) {
		label->setAlignment(Qt::AlignRight);
		gridLayout->addWidget(label, row, 0);
		gridLayout->addWidget(lEdit, row, 1);
	}
	else {
		label->setAlignment(Qt::AlignLeft);
		gridLayout->addWidget(lEdit, row, 0);
		gridLayout->addWidget(label, row, 1);
	}
}

void PinLabelDialog::labelChanged() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>(sender());
	if (lineEdit == NULL) return;

	bool ok;
	int index = lineEdit->property("index").toInt(&ok);
	if (!ok) return;

	if (index < 0) return;
	if (index >= m_labels.count()) return;

	m_labels.replace(index, lineEdit->text());
}

const QStringList & PinLabelDialog::labels() {
	return m_labels;
}
