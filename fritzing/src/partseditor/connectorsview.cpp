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

$Revision$:
$Author$:
$Date$

********************************************************************/


/**************************************

TODO:

    would be nice to have a change all radios function

**************************************/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QRadioButton>
#include <QMessageBox>
#include <QMutexLocker>

#include "connectorsview.h"

//////////////////////////////////////

bool GotZeroConnector = false;

bool byID(QDomElement & c1, QDomElement & c2)
{
    int c1id = -1;
    int c2id = -1;
	int ix = IntegerFinder.indexIn(c1.attribute("id"));
    if (ix > 0) c1id = IntegerFinder.cap(0).toInt();
    ix = IntegerFinder.indexIn(c2.attribute("id"));
    if (ix > 0) c2id = IntegerFinder.cap(0).toInt();

    if (c1id == 0 || c2id == 0) GotZeroConnector = true;
	
	return c1id <= c2id;
}

//////////////////////////////////////

ConnectorsView::ConnectorsView(QWidget * parent) : QScrollArea(parent) 
{
    m_mainFrame = NULL;
	this->setWidgetResizable(true);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFile styleSheet(":/resources/styles/newpartseditor.qss");
    if (!styleSheet.open(QIODevice::ReadOnly)) {
        DebugDialog::debug("Unable to open :/resources/styles/newpartseditor.qss");
    } else {
    	this->setStyleSheet(styleSheet.readAll());
    }
}

ConnectorsView::~ConnectorsView() {

}

void ConnectorsView::initConnectors(const QDomDocument & doc) 
{
    if (m_mainFrame) {
        this->setWidget(NULL);
        delete m_mainFrame;
        m_mainFrame = NULL;
    }

    QDomElement root = doc.documentElement();
    QDomElement connectors = root.firstChildElement("connectors");
    QDomElement connector = connectors.firstChildElement("connector");
    QList<QDomElement> connectorList;
    while (!connector.isNull()) {
        connectorList.append(connector);
        connector = connector.nextSiblingElement("connector");
    }

    qSort(connectorList.begin(), connectorList.end(), byID);
    m_connectorCount = connectorList.size();

	m_mainFrame = new QFrame(this);
	m_mainFrame->setObjectName("connectors");
	QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );

    QLabel *explanation = new QLabel(tr("This is where you edit the connector metadata for the part"));
    mainLayout->addWidget(explanation);

    QFrame * numberFrame = new QFrame();
    QHBoxLayout * numberLayout = new QHBoxLayout();

    QLabel * label = new QLabel(tr("number of connectors:"));
    numberLayout->addWidget(label);

    QLineEdit * numberEdit = new QLineEdit();
    numberEdit->setText(QString::number(m_connectorCount));
    QValidator *validator = new QIntValidator(1, 999, this);
    numberEdit->setValidator(validator);
    numberLayout->addWidget(numberEdit);
    connect(numberEdit, SIGNAL(editingFinished()), this, SLOT(connectorCountEntry()));

    numberLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    numberFrame->setLayout(numberLayout);
    mainLayout->addWidget(numberFrame);

    int ix = 0;
    foreach (QDomElement connector, connectorList) {
        QWidget * widget = makeConnectorForm(connector, ix++);
        mainLayout->addWidget(widget);
    }

    m_mainFrame->setLayout(mainLayout);

    this->setWidget(m_mainFrame);
}

void ConnectorsView::nameEntry() {
    changeConnectors();
}

void ConnectorsView::typeEntry() {
    changeConnectors();
}

void ConnectorsView::descriptionEntry() {
    changeConnectors();
}

void ConnectorsView::connectorCountEntry() {
    if (!m_mutex.tryLock(1)) return;            // need the mutex because multiple editingFinished() signals can be triggered more-or-less at once

    m_mutex.unlock();
    QMutexLocker locker(&m_mutex);

    QLineEdit * lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit == NULL) return;

    int newCount = lineEdit->text().toInt();
    if (newCount == m_connectorCount) return;

    QString message;
    if (newCount < m_connectorCount) {
        message = tr("Connectors will be removed from the end of the list, so you may lose some work. ");
    }
    else {
        message = tr("Connectors will be added the end of the list. ");
    }
    message += tr("Rather than changing the number of connectors here, "
                    "it may be better to begin with a part that already has the right number of connectors.\n\n");
    message += tr("Change to %n connectors?", "", newCount);


	QMessageBox messageBox(NULL);
	messageBox.setWindowTitle(tr("Change Connectors Warning"));
	messageBox.setText(message);
	messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	messageBox.setDefaultButton(QMessageBox::Cancel);
	messageBox.setIcon(QMessageBox::Warning);
	messageBox.setWindowModality(Qt::WindowModal);
	messageBox.setButtonText(QMessageBox::Ok, tr("Change Connector Count"));
	messageBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
	QMessageBox::StandardButton answer = (QMessageBox::StandardButton) messageBox.exec();

	if (answer != QMessageBox::Ok) {
        lineEdit->setText(QString::number(m_connectorCount));
		return;
	}

}

QWidget * ConnectorsView::makeConnectorForm(const QDomElement & connector, int index) {
    QFrame * frame = new QFrame();
    frame->setObjectName(index % 2 == 0 ? "connector0Frame" : "connector1Frame");
    QVBoxLayout * mainLayout = new QVBoxLayout();

    QFrame * nameFrame = new QFrame();
    QHBoxLayout * nameLayout = new QHBoxLayout();

    QString id("    ");
    int ix = IntegerFinder.indexIn(connector.attribute("id"));
    if (ix >= 0) {
        if (GotZeroConnector) {
            int cid = IntegerFinder.cap(0).toInt();
            id = QString::number(cid + 1);
        }
        else {
            id = IntegerFinder.cap(0);
        }
    }

    QLabel * numberLabel = new QLabel("<b>" + id + ".</b>");
	numberLabel->setObjectName("PartsEditorLabel");
    numberLabel->setStatusTip(tr("Connector number"));
    nameLayout->addWidget(numberLabel);
    nameLayout->addSpacing(10);

    QLabel * justLabel = new QLabel(tr("<b>Type:</b>"));
	justLabel->setObjectName("PartsEditorLabel");
    nameLayout->addWidget(justLabel);

    Connector::ConnectorType ctype = Connector::Male;
    if (connector.attribute("type").compare("female", Qt::CaseInsensitive) == 0) ctype = Connector::Female;
    else if (connector.attribute("type").compare("pad", Qt::CaseInsensitive) == 0) ctype = Connector::Pad;

    QRadioButton * radioButton = new QRadioButton(MaleSymbolString); 
	connect(radioButton, SIGNAL(clicked()), this, SLOT(typeEntry()));
    radioButton->setObjectName("PartsEditorRadio");
    if (ctype == Connector::Male) radioButton->setChecked(true); 
    radioButton->setProperty("value", Connector::Male);
    radioButton->setProperty("index", index);
    radioButton->setProperty("type", "radio");
    nameLayout->addWidget(radioButton);

    radioButton = new QRadioButton(FemaleSymbolString); 
	connect(radioButton, SIGNAL(clicked()), this, SLOT(typeEntry()));
    radioButton->setObjectName("PartsEditorRadio");
    if (ctype == Connector::Female) radioButton->setChecked(true); 
    radioButton->setProperty("value", Connector::Female);
    radioButton->setProperty("index", index);
    radioButton->setProperty("type", "radio");
    nameLayout->addWidget(radioButton);

    radioButton = new QRadioButton(tr("SMD-pad")); 
	connect(radioButton, SIGNAL(clicked()), this, SLOT(typeEntry()));
    radioButton->setObjectName("PartsEditorRadio");
    if (ctype == Connector::Pad) radioButton->setChecked(true); 
    radioButton->setProperty("value", Connector::Pad);
    nameLayout->addWidget(radioButton);
    radioButton->setProperty("index", index);
    radioButton->setProperty("type", "radio");
    nameLayout->addSpacing(10);

    justLabel = new QLabel(tr("<b>Name:</b>"));
	justLabel->setObjectName("PartsEditorLabel");
    nameLayout->addWidget(justLabel);

    QLineEdit * nameEdit = new QLineEdit();
    nameEdit->setText(connector.attribute("name"));
	connect(nameEdit, SIGNAL(editingFinished()), this, SLOT(nameEntry()));
	nameEdit->setObjectName("PartsEditorLineEdit");
    nameEdit->setStatusTip(tr("Set the connectors's title"));
    nameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    nameEdit->setProperty("index", index);
    nameEdit->setProperty("type", "name");
    nameEdit->setProperty("id", connector.attribute("id"));
    nameLayout->addWidget(nameEdit);

    nameFrame->setLayout(nameLayout);
    mainLayout->addWidget(nameFrame);

    QFrame * descriptionFrame = new QFrame();
    QHBoxLayout * descriptionLayout = new QHBoxLayout();

    justLabel = new QLabel(tr("<b>Description:</b>"));
	justLabel->setObjectName("PartsEditorLabel");
    descriptionLayout->addWidget(justLabel);

    QLineEdit * descriptionEdit = new QLineEdit();
    QDomElement description = connector.firstChildElement("description");
    descriptionEdit->setText(description.text());
	connect(descriptionEdit, SIGNAL(editingFinished()), this, SLOT(descriptionEntry()));
	descriptionEdit->setObjectName("PartsEditorLineEdit");
    descriptionEdit->setStatusTip(tr("Set the connectors's description"));
    descriptionEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    descriptionEdit->setProperty("index", index);
    descriptionEdit->setProperty("type", "description");
    descriptionLayout->addWidget(descriptionEdit);

    descriptionFrame->setLayout(descriptionLayout);
    mainLayout->addWidget(descriptionFrame);
    frame->setLayout(mainLayout);
    return frame;
}

void ConnectorsView::changeConnectors() {
    QList<ConnectorMetadata *> connectorMetadataList;

    for (int i = 0; i < m_connectorCount; i++) {
        connectorMetadataList.append(new ConnectorMetadata());
    }

    QList<QWidget *> widgets = m_mainFrame->findChildren<QWidget *>();
    foreach (QWidget * widget, widgets) {
        bool ok;
        int index = widget->property("index").toInt(&ok);
        if (!ok) continue;

        if (index >= m_connectorCount) continue;

        QString type = widget->property("type").toString();
        if (type == "name") {
            QLineEdit * lineEdit = qobject_cast<QLineEdit *>(widget);
            if (lineEdit == NULL) continue;

            connectorMetadataList.at(index)->connectorName = lineEdit->text();
            connectorMetadataList.at(index)->connectorID = widget->property("id").toString();
        }
        else if (type == "radio") {
            QRadioButton * radioButton = qobject_cast<QRadioButton *>(widget);
            if (radioButton == NULL) continue;
            if (!radioButton->isChecked()) continue;

            connectorMetadataList.at(index)->connectorType = (Connector::ConnectorType) radioButton->property("value").toInt();
        }
        else if (type == "description") {
            QLineEdit * lineEdit = qobject_cast<QLineEdit *>(widget);
            if (lineEdit == NULL) continue;

            connectorMetadataList.at(index)->connectorDescription = lineEdit->text();
        }

    }

    // make sure to used Qt::DirectConnection, since we will delete the list items after the emit
    emit connectorsChanged(connectorMetadataList);

    foreach (ConnectorMetadata * cm, connectorMetadataList) {
        delete cm;
    }

}
