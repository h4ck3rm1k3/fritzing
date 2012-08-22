/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 6140 $:
$Author: cohen@irascible.com $:
$Date: 2012-07-04 15:38:22 +0200 (Wed, 04 Jul 2012) $

********************************************************************/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>

#include "metadataview.h"
#include "hashpopulatewidget.h"

//////////////////////////////////////

FocusOutTextEdit::FocusOutTextEdit(QWidget * parent) : QTextEdit(parent)
{

}

FocusOutTextEdit::~FocusOutTextEdit()
{
}

void FocusOutTextEdit::focusOutEvent(QFocusEvent * e) {
	QTextEdit::focusOutEvent(e);
    if (isUndoRedoEnabled()) {
        emit focusOut();
    }
}

//////////////////////////////////////

MetadataView::MetadataView(QWidget * parent) : QScrollArea(parent) 
{
	this->setWidgetResizable(true);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QFrame * mainFrame = new QFrame(this);
	mainFrame->setObjectName("metadataMainFrame");

	QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);

    QLabel *explanation = new QLabel(tr("This is where you edit the metadata for the part ..."));
    mainLayout->addWidget(explanation);

    QFormLayout * formLayout = new QFormLayout();
    QFrame * formFrame = new QFrame;
    mainLayout->addWidget(formFrame);

    QLineEdit * titleEdit = new QLineEdit();
	connect(titleEdit, SIGNAL(editingFinished()), this, SLOT(titleEntry()));
	titleEdit->setObjectName("PartsEditorLineEdit");
    titleEdit->setStatusTip(tr("Set the part's title"));
    formLayout->addRow(tr("Title"), titleEdit);

    QLineEdit * authorEdit = new QLineEdit();
	connect(authorEdit, SIGNAL(editingFinished()), this, SLOT(authorEntry()));
	authorEdit->setObjectName("PartsEditorLineEdit");
    authorEdit->setStatusTip(tr("Set the part's author"));
    formLayout->addRow(tr("Author"), authorEdit);

    QTextEdit * descrEdit = new FocusOutTextEdit();
	connect(descrEdit, SIGNAL(focusOut()), this, SLOT(descrEntry()));
	descrEdit->setObjectName("PartsEditorTextEdit");
    descrEdit->setStatusTip(tr("Set the part's description--you can use simple html (as defined by Qt's Rich Text)"));
    formLayout->addRow(tr("Description"), descrEdit);

    QLineEdit * familyEdit = new QLineEdit();
	connect(familyEdit, SIGNAL(editingFinished()), this, SLOT(familyEntry()));
	familyEdit->setObjectName("PartsEditorLineEdit");
    descrEdit->setStatusTip(tr("Set the part's family--what other parts is this part related to"));
    formLayout->addRow(tr("Family"), familyEdit);

	QStringList readOnlyKeys;
	readOnlyKeys;
	QHash<QString,QString> initValues;

    HashPopulateWidget * propertiesEdit = new HashPopulateWidget("test", initValues, readOnlyKeys, NULL, this);
	propertiesEdit->setObjectName("PartsEditorPropertiesEdit");
    propertiesEdit->setStatusTip(tr("Set the part's properties"));
    formLayout->addRow(tr("Properties"), propertiesEdit);

    formFrame->setLayout(formLayout);
    mainFrame->setLayout(mainLayout);
}

MetadataView::~MetadataView() {

}

void MetadataView::titleEntry() {
    DebugDialog::debug("title entry");
}

void MetadataView::authorEntry() {
    DebugDialog::debug("author entry");
}

void MetadataView::descrEntry() {
    DebugDialog::debug("descr entry");
}


