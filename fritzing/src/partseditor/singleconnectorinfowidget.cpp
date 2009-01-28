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


#include "connectorsinfowidget.h"
#include "singleconnectorinfowidget.h"
#include "connectorinforemovebutton.h"
#include "../debugdialog.h"

const QString ConnectorTypeWidget::FemaleSymbol = QString("%1").arg(QChar(0x2640));
const QString ConnectorTypeWidget::MaleSymbol = QString("%1").arg(QChar(0x2642));

ConnectorTypeWidget::ConnectorTypeWidget(Connector::ConnectorType type, QWidget *parent)
	: QLabel(parent)
{
	m_isSelected = false;
	setType(type);
}

Connector::ConnectorType ConnectorTypeWidget::type() {
	if(text()==FemaleSymbol) {
		return Connector::connectorTypeFromName("female");
	} else if(text()==MaleSymbol) {
		return Connector::connectorTypeFromName("male");
	}
	return Connector::Unknown;
}

const QString &ConnectorTypeWidget::typeAsStr() {
	return Connector::connectorNameFromType(type());
}

void ConnectorTypeWidget::setType(Connector::ConnectorType type) {
	if(m_isInEditionMode) {
		m_typeBackUp = this->type();
	}
	if(type == Connector::Female) {
		setText(FemaleSymbol);
	} else if(type == Connector::Male) {
		setText(MaleSymbol);
	}
	setToolTip(Connector::connectorNameFromType(type));
}

void ConnectorTypeWidget::cancel() {
	setType(m_typeBackUp);
}

void ConnectorTypeWidget::mousePressEvent(QMouseEvent * event) {
	toggleValue();
	QLabel::mousePressEvent(event);
}

void ConnectorTypeWidget::toggleValue() {
	if(m_isSelected && m_isInEditionMode) {
		if(type() == Connector::Female) {
			setType(Connector::Male);
		} else if(type() == Connector::Male) {
			setType(Connector::Female);
		}
	}
}



SingleConnectorInfoWidget::SingleConnectorInfoWidget(ConnectorsInfoWidget *topLevelContainer, WaitPushUndoStack *undoStack, Connector* connector, QWidget *parent)
	: AbstractConnectorInfoWidget(topLevelContainer,parent)
{
	static QString EMPTY_CONN_NAME = QObject::tr("no name yet");
	static QString EMPTY_CONN_DESC = QObject::tr("no description yet");
	static Connector::ConnectorType EMPTY_CONN_TYPE = Connector::Female;

	QString name;
	QString description;
	Connector::ConnectorType type;

	m_undoStack = undoStack;
	m_connector = connector;

	if(connector && connector->connectorStuff()) {
		ConnectorStuff *connStuff = connector->connectorStuff();
		name = connStuff->name().isNull() || connStuff->name().isEmpty() ? EMPTY_CONN_NAME : connStuff->name();
		description = connStuff->description().isNull() || connStuff->description().isEmpty()? EMPTY_CONN_DESC : connStuff->description();
		type = connStuff->connectorType() == Connector::Unknown ? EMPTY_CONN_TYPE : connStuff->connectorType();
	} else {
		name = EMPTY_CONN_NAME;
		description = EMPTY_CONN_DESC;
		type = EMPTY_CONN_TYPE;
	}

	m_nameLabel = new QLabel(name,this);
	m_nameDescSeparator = new QLabel(" - ",this);
	m_descLabel = new QLabel(description,this);
	m_descLabel->setObjectName("description");

	m_type = new ConnectorTypeWidget(type, this);
	m_nameEdit = NULL;
	m_descEdit = NULL;

	m_acceptButton = NULL;
	m_cancelButton = NULL;

	setSelected(false);


	QLayout *layout = new QVBoxLayout(this);
	layout->setMargin(1);

	toStandardMode();
}

QString SingleConnectorInfoWidget::id() {
	return m_connector->connectorStuffID();
}
QString SingleConnectorInfoWidget::name() {
	return m_nameLabel->text();
}

QString SingleConnectorInfoWidget::description() {
	return m_descLabel->text();
}

QString SingleConnectorInfoWidget::type() {
	return m_type->typeAsStr();
}

void SingleConnectorInfoWidget::startEdition() {
	if(!m_nameEdit) m_nameEdit = new QLineEdit(this);
	m_nameEdit->setText(m_nameLabel->text());
	if(!m_descEdit) m_descEdit = new QTextEdit(this);
	m_descEdit->setText(m_descLabel->text());
	toEditionMode();

	emit editionStarted();
}

void SingleConnectorInfoWidget::editionCompleted() {
	if(m_isInEditionMode) {
		m_undoStack->push(new QUndoCommand("Dummy parts editor command"));
		m_nameLabel->setText(m_nameEdit->text());
		m_descLabel->setText(m_descEdit->toPlainText());
		toStandardMode();

		emit editionFinished();
	}
}

void SingleConnectorInfoWidget::editionCanceled() {
	m_type->cancel();
	toStandardMode();

	emit editionFinished();
}


void SingleConnectorInfoWidget::toStandardMode() {
	hide();

	setInEditionMode(false);

	hideIfNeeded(m_nameEdit);
	hideIfNeeded(m_descEdit);
	hideIfNeeded(m_acceptButton);
	hideIfNeeded(m_cancelButton);

	QHBoxLayout *hbLayout = new QHBoxLayout();
	hbLayout->addWidget(m_type);
	hbLayout->addSpacerItem(new QSpacerItem(10,0));
	hbLayout->addWidget(m_nameLabel);
	m_nameLabel->show();
	hbLayout->addWidget(m_nameDescSeparator);
	m_nameDescSeparator->show();
	hbLayout->addWidget(m_descLabel);
	m_descLabel->show();
	hbLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	hbLayout->addWidget(m_removeButton);

	QVBoxLayout *layout = (QVBoxLayout*)this->layout();
	layout->addLayout(hbLayout);

	setFixedHeight(SingleConnectorHeight);
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	updateGeometry();

	show();
	setFocus();
}

void SingleConnectorInfoWidget::toEditionMode() {
	hide();

	setInEditionMode(true);

	hideIfNeeded(m_nameLabel);
	hideIfNeeded(m_nameDescSeparator);
	hideIfNeeded(m_descLabel);

	// first row
	QHBoxLayout *firstRowLayout = new QHBoxLayout();
	firstRowLayout->addWidget(m_type);
	firstRowLayout->addSpacerItem(new QSpacerItem(10,0));
	if(!m_nameEdit) m_nameEdit = new QLineEdit(this);
	firstRowLayout->addWidget(m_nameEdit);
	firstRowLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	firstRowLayout->addWidget(m_removeButton);
	m_nameEdit->show();


	// second row
	if(!m_descEdit) m_descEdit = new QTextEdit(this);
	m_descEdit->show();


	// third row
	if(!m_acceptButton) {
		m_acceptButton = new QPushButton(QObject::tr("Accept"),this);
		connect(m_acceptButton,SIGNAL(clicked()),this,SLOT(editionCompleted()));
	}
	m_acceptButton->show();

	if(!m_cancelButton) {
		m_cancelButton = new QPushButton(QObject::tr("Cancel"),this);
		connect(m_cancelButton,SIGNAL(clicked()),this,SLOT(editionCanceled()));
	}
	m_cancelButton->show();

	QHBoxLayout *thirdRowLayout = new QHBoxLayout();
	thirdRowLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
	thirdRowLayout->addWidget(m_acceptButton);
	thirdRowLayout->addWidget(m_cancelButton);


	QVBoxLayout *layout = (QVBoxLayout*)this->layout();
	layout->addLayout(firstRowLayout);
	layout->addWidget(m_descEdit);
	layout->addLayout(thirdRowLayout);


	setFixedHeight(SingleConnectorHeight*3);
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	updateGeometry();

	show();
	setFocus();

	emit editionStarted();
}

void SingleConnectorInfoWidget::hideIfNeeded(QWidget* w) {
	if(w) {
		w->hide();
		layout()->removeWidget(w);
	}
}


QSize SingleConnectorInfoWidget::maximumSizeHint() const {
	return sizeHint();
}

QSize SingleConnectorInfoWidget::minimumSizeHint() const {
	return sizeHint();
}

QSize SingleConnectorInfoWidget::sizeHint() const {
	QSize retval;
	if(m_isInEditionMode) {
		retval = QSize(width(),SingleConnectorHeight*3);
	} else {
		retval = QSize(width(),SingleConnectorHeight);
	}

	return retval;
}

void SingleConnectorInfoWidget::setSelected(bool selected, bool doEmitChange) {
	AbstractConnectorInfoWidget::setSelected(selected, doEmitChange);
	m_type->m_isSelected = selected;

	if(selected && m_connector) {
		emit tellViewsMyConnectorIsNewSelected(m_connector->connectorStuffID());
	}
}

void SingleConnectorInfoWidget::setInEditionMode(bool inEditionMode) {
	m_isInEditionMode = inEditionMode;
	m_type->m_isInEditionMode = inEditionMode;
}

bool SingleConnectorInfoWidget::isInEditionMode() {
	return m_isInEditionMode;
}

void SingleConnectorInfoWidget::mousePressEvent(QMouseEvent * event) {
	if(isSelected() && !isInEditionMode()) {
		startEdition();
	} else if(!isSelected()) {
		setSelected(true);
	}
	QFrame::mousePressEvent(event);
}

Connector *SingleConnectorInfoWidget::connector() {
	return m_connector;
}

MismatchingConnectorWidget *SingleConnectorInfoWidget::toMismatching(ItemBase::ViewIdentifier missingViewId) {
	MismatchingConnectorWidget *mcw = new MismatchingConnectorWidget(m_topLevelContainer,missingViewId, m_connector->connectorStuffID(), (QWidget*)parent(), false, m_connector);
	return mcw;
}

