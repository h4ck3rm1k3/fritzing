/*
 * (c) Fachhochschule Potsdam
 */

#include <QGridLayout>
#include "mismatchingconnectorwidget.h"
#include "../debugdialog.h"

QList<ItemBase::ViewIdentifier> MismatchingConnectorWidget::AllViews;

//TODO Mariano: looks like an abstracteditable, perhaps can be one
MismatchingConnectorWidget::MismatchingConnectorWidget(ItemBase::ViewIdentifier viewId, const QString &connId, QWidget *parent, bool isInView, Connector* conn)
		: AbstractConnectorInfoWidget(parent) {
	if(AllViews.size() == 0) {
		AllViews << ItemBase::BreadboardView << ItemBase::SchematicView << ItemBase::PCBView;
	}

	m_prevConn = conn;
	m_connId = connId;

	m_connIdLabel = new QLabel(m_connId+":", this);
	m_connIdLabel->setObjectName("mismatchConnId");

	m_connMsgLabel = new QLabel(viewsString(),this);
	m_connMsgLabel->setObjectName("mismatchConnMsg");

	QLabel *errorImg = new QLabel(this);
	errorImg->setPixmap(QPixmap(":/resources/images/error_x_small.png"));

	if(isInView) {
		m_missingViews << ItemBase::BreadboardView << ItemBase::SchematicView << ItemBase::PCBView;
		addViewPresence(viewId);
	} else {
		removeViewPresence(viewId);
	}

	QHBoxLayout *lo = new QHBoxLayout();
	lo->addWidget(errorImg);
	lo->addSpacerItem(new QSpacerItem(5,0));
	lo->addWidget(m_connIdLabel);
	lo->addWidget(m_connMsgLabel);
	lo->addStretch();
	lo->setMargin(3);
	lo->setSpacing(3);
	this->setLayout(lo);

	updateGeometry();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void MismatchingConnectorWidget::setSelected(bool selected, bool doEmitChange) {
	AbstractConnectorInfoWidget::setSelected(selected, doEmitChange);

	if(selected && !m_connId.isNull()) {
		emit tellViewsMyConnectorIsNewSelected(m_connId);
	}
}

bool MismatchingConnectorWidget::onlyMissingThisView(ItemBase::ViewIdentifier viewId) {
	return m_missingViews.size() == 1 && m_missingViews[0] == viewId;
}

void MismatchingConnectorWidget::addViewPresence(ItemBase::ViewIdentifier viewId) {
	m_missingViews.removeAll(viewId);
	m_connMsgLabel->setText(viewsString());
}

void MismatchingConnectorWidget::removeViewPresence(ItemBase::ViewIdentifier viewId) {
	m_missingViews << viewId;
	m_connMsgLabel->setText(viewsString());
}

bool MismatchingConnectorWidget::presentInAllViews() {
	return m_missingViews.size() == 0;
}

const QString &MismatchingConnectorWidget::connId() {
	return m_connId;
}

Connector *MismatchingConnectorWidget::prevConn() {
	return m_prevConn;
}

QList<ItemBase::ViewIdentifier> MismatchingConnectorWidget::views() {
	QList<ItemBase::ViewIdentifier> list = AllViews;
	for(int i=0; i < m_missingViews.size(); i++) {
		list.removeAll(m_missingViews[i]);
	}
	return list;
}

QString MismatchingConnectorWidget::viewsString() {
	QString retval = tr("In ");
	bool notFirst = false;
	for(int i=0; i < AllViews.size(); i++) {
		ItemBase::ViewIdentifier viewId = AllViews[i];
		if(!m_missingViews.contains(viewId)) {
			if(notFirst) {
				retval += tr("and ");
			}
			retval += ItemBase::viewIdentifierNaturalName(viewId)+" ";
			notFirst = true;
		}
	}
	retval += tr("view only");
	return retval;
}

void MismatchingConnectorWidget::mousePressEvent(QMouseEvent * event) {
	if(!isSelected()) {
		setSelected(true);
	}
	QFrame::mousePressEvent(event);
}
