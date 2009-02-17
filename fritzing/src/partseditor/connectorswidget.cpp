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



#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "connectorswidget.h"
#include "../debugdialog.h"

ConnectorNameComboBox::ConnectorNameComboBox(int * prevSelIdx, QWidget *parent) : QComboBox(parent) {
	m_prevSelIdx = prevSelIdx;
}

void ConnectorNameComboBox::focusOutEvent( QFocusEvent * /*event*/ ) {
	this->setCurrentIndex(*m_prevSelIdx);
	this->setEditable(false);
}

ConnectorsWidget::ConnectorsWidget(QWidget *parent) : QWidget(parent) {
	m_prevComboBoxIdx = -1;

	m_newButton = new QPushButton(tr("New"));
	m_newButton->setFixedWidth(50);

	m_userDefConnectors = new ConnectorNameComboBox(&m_prevComboBoxIdx,this);

	m_connId = new QLineEdit();
	m_connId->setValidator(new ConnectorIdValidator(m_connId));

	m_connName = new QLineEdit();
	m_connDesc = new QLineEdit();
	m_connType = new QComboBox();
	m_connType->addItem(tr("Female"));
	m_connType->addItem(tr("Male"));


	m_breadConnectors = new QComboBox();
	m_schemConnectors = new QComboBox();
	m_pcbConnectors = new QComboBox();

	int row = 0;

	QGridLayout *layout = new QGridLayout();
	layout->setSpacing(2);
	layout->setMargin(3);

	layout->addWidget(m_newButton, row, 0);
	layout->addWidget(m_userDefConnectors, row++, 1);

	layout->addWidget(new QLabel(tr("ID")), row, 0);
	layout->addWidget(m_connId, row++, 1);

	layout->addWidget(new QLabel(tr("Name")), row, 0);
	layout->addWidget(m_connName, row++, 1);

	layout->addWidget(new QLabel(tr("Description")), row, 0);
	layout->addWidget(m_connDesc, row++, 1);

	layout->addWidget(new QLabel(tr("Type")), row, 0);
	layout->addWidget(m_connType, row++, 1);

	layout->addWidget(new QLabel(tr("Breadboard")), row, 0);
	layout->addWidget(m_breadConnectors, row++, 1);

	layout->addWidget(new QLabel(tr("Schematic")), row, 0);
	layout->addWidget(m_schemConnectors, row++, 1);

	layout->addWidget(new QLabel(tr("PCB")), row, 0);
	layout->addWidget(m_pcbConnectors, row++, 1);

	this->setLayout(layout);

	//this->setEnabled(false); // loads empty

	connect(m_newButton,SIGNAL(clicked()),this,SLOT(setUsrDefConnectorsEditable()));
	connect(m_userDefConnectors,SIGNAL(currentIndexChanged(int)),this,SLOT(showConnInfoIfAnyAndSavePrev(int)));
	connect(m_connId,SIGNAL(editingFinished()),this,SLOT(updateUserDefConnsComboBox()));

	m_comboBoxes[ItemBase::BreadboardView] = m_breadConnectors;
	m_comboBoxes[ItemBase::SchematicView] = m_schemConnectors;
	m_comboBoxes[ItemBase::PCBView] = m_pcbConnectors;

	//m_editingExistingConnector = false;
}

void ConnectorsWidget::showConnInfoIfAnyAndSavePrev(int idx) {
	m_userDefConnectors->setEditable(false);

	// Save Prev
	keepConnInfo();
	m_prevComboBoxIdx = idx;

	// show info
	if(m_connInfo[idx]) {
		ConnectorsWidgetHelpClass cwi = *m_connInfo[idx];
		m_connId->setText(cwi.id);
		m_connName->setText(cwi.name);
		m_connDesc->setText(cwi.description);
		m_connType->setCurrentIndex(cwi.idxTypeCB);
		m_breadConnectors->setCurrentIndex(cwi.idxBreadCB);
		m_schemConnectors->setCurrentIndex(cwi.idxSchemCB);
		m_pcbConnectors->setCurrentIndex(cwi.idxPcbCB);
	}
}

void ConnectorsWidget::keepConnInfo() {
	if(m_prevComboBoxIdx != -1) {
		ConnectorsWidgetHelpClass *cwi = m_connInfo[m_prevComboBoxIdx];
		cwi->id = m_connId->text();
		cwi->name = m_connName->text();
		cwi->description = m_connDesc->text();
		cwi->idxTypeCB = m_connType->currentIndex();
		cwi->idxBreadCB = m_breadConnectors->currentIndex();
		cwi->idxSchemCB = m_schemConnectors->currentIndex();
		cwi->idxPcbCB = m_pcbConnectors->currentIndex();
	}
}

void ConnectorsWidget::updateUserDefConnsComboBox() {
	int idx = m_userDefConnectors->currentIndex();
	if(idx != -1) {
		QString id = m_connId->text();
		m_connInfo[idx]->id = id;
		m_userDefConnectors->setItemText(idx,id);
	}
}

void ConnectorsWidget::setUsrDefConnectorsEditable() {
	m_userDefConnectors->setCurrentIndex(-1);
	m_userDefConnectors->setEditable(true);
	m_userDefConnectors->setValidator(new ConnectorIdValidator(m_userDefConnectors));
	connect(m_userDefConnectors->lineEdit(),SIGNAL(returnPressed()),this,SLOT(setUsrDefConnectorsUneditable()));
	clearChildren();
}

void ConnectorsWidget::clearChildren() {
	m_connId->setText("");
	m_connName->setText("");
	m_connDesc->setText("");
	m_connType->setCurrentIndex(-1);
	m_breadConnectors->setCurrentIndex(-1);
	m_schemConnectors->setCurrentIndex(-1);
	m_pcbConnectors->setCurrentIndex(-1);
}

void ConnectorsWidget::setUsrDefConnectorsUneditable() {
	disconnect(m_userDefConnectors->lineEdit(),SIGNAL(returnPressed()),this,SLOT(setUsrDefConnectorsUneditable()));
	m_userDefConnectors->setEditable(false);
	int idx = m_userDefConnectors->currentIndex();
	QString id = m_userDefConnectors->currentText();
	m_prevComboBoxIdx = idx;
	m_connId->setText(id);
	m_connInfo.insert(idx,new ConnectorsWidgetHelpClass(id));
}

void ConnectorsWidget::connectorsFound(ItemBase::ViewIdentifier viewId, QStringList connNames) {
	QComboBox* currCB = m_comboBoxes[viewId];
	currCB->clear();
	currCB->addItems(connNames);
}

QList<ConnectorShared*> ConnectorsWidget::connectorsInfo() {
	keepConnInfo();
	QList<ConnectorShared*> *retval = new QList<ConnectorShared*>;
	for(int i=0; i<m_userDefConnectors->count(); i++) {
		ConnectorShared* cs = new ConnectorShared();
		ConnectorsWidgetHelpClass *cwhc = m_connInfo[i];
		cs->setId(cwhc->id);
		cs->setName(cwhc->name);
		cs->setDescription(cwhc->description);
		if(cwhc->idxTypeCB != -1) {
			cs->setConnectorType(m_connType->itemText(cwhc->idxTypeCB).toLower());
		}

		// TODO: figure out which layer the connectors belong to
		if(cwhc->idxBreadCB != -1) {
			cs->addPin(ItemBase::BreadboardView,m_breadConnectors->itemText(cwhc->idxBreadCB), ViewLayer::Breadboard, "");
		}
		if(cwhc->idxSchemCB != -1) {
			cs->addPin(ItemBase::SchematicView, m_schemConnectors->itemText(cwhc->idxSchemCB), ViewLayer::Schematic, "");
		}
		if(cwhc->idxPcbCB != -1) {
			cs->addPin(ItemBase::PCBView, m_pcbConnectors->itemText(cwhc->idxPcbCB), ViewLayer::Copper0, "");
		}
		*retval << cs;
	}
	return *retval;
}

void ConnectorsWidget::updateInfo(ModelPart * modelPart) {
	modelPart->modelPartShared()->initConnectors();
	const QList<ConnectorShared *> connectors = modelPart->modelPartShared()->connectors();

	for (int i = 0; i < connectors.count(); i++) {
		addConnectorShared(connectors[i]);
	}
}

void ConnectorsWidget::addConnectorShared(ConnectorShared * connStuff) {
	ConnectorsWidgetHelpClass *cwhc = new ConnectorsWidgetHelpClass(connStuff->id());
	cwhc->name = connStuff->name();
	cwhc->description = connStuff->description();

	cwhc->idxTypeCB = itemIndex(m_connType, connStuff->connectorTypeString());

	// TODO: figure out the correct pin layer
	cwhc->idxBreadCB = itemIndex(m_breadConnectors, connStuff->pin(ItemBase::BreadboardView, ViewLayer::Breadboard));
	cwhc->idxSchemCB = itemIndex(m_schemConnectors, connStuff->pin(ItemBase::SchematicView, ViewLayer::Schematic));
	cwhc->idxPcbCB = itemIndex(m_pcbConnectors, connStuff->pin(ItemBase::PCBView, ViewLayer::Copper0));

	m_connInfo[m_userDefConnectors->count()] = cwhc;
	m_userDefConnectors->addItem(cwhc->id);
}

int ConnectorsWidget::itemIndex(QComboBox* opts, QString text) {
	for(int i=0; i < opts->count(); i++) {
		if(opts->itemText(i).toLower() == text.toLower()) {
			return i;
		}
	}
	opts->addItem(text);
	return opts->count()-1;
}
