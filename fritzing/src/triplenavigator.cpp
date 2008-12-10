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

#include "triplenavigator.h"
#include "debugdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

TripleNavigator::TripleNavigator( QWidget * parent )
	: QFrame(parent)
{
	m_splitter = new QSplitter(Qt::Horizontal, this);
	m_splitter->setChildrenCollapsible(false);
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(m_splitter);
	layout->setMargin(1);
    this->setLayout(layout);
}

void TripleNavigator::addView(MiniViewContainer * miniViewContainer, const QString & title) 
{
	TripleNavigatorFrame * frame = new TripleNavigatorFrame(miniViewContainer, title, this);

	m_splitter->addWidget(frame);
	for (int i = 0; i < m_splitter->count(); i++) {
		((TripleNavigatorFrame *) m_splitter->widget(i))->hook(miniViewContainer);
	}


}

///////////////////////////////////////////

TripleNavigatorLabel::TripleNavigatorLabel(QWidget * parent) : QLabel(parent) 
{
	m_miniViewContainer = NULL;
}

void TripleNavigatorLabel::mousePressEvent(QMouseEvent * event) {
	Q_UNUSED(event);
	if (m_miniViewContainer) {
		m_miniViewContainer->miniViewMousePressedSlot();
	}
}

void TripleNavigatorLabel::setMiniViewContainer(MiniViewContainer * miniViewContainer) 
{
	m_miniViewContainer = miniViewContainer;
}

void TripleNavigatorLabel::navigatorMousePressedSlot(MiniViewContainer * miniViewContainer) {
	if (miniViewContainer == m_miniViewContainer) {
		setStyleSheet("#tripleNavigatorLabel { height: 13; color: #ffffff; }");
	}
	else {
		setStyleSheet("#tripleNavigatorLabel { height: 13; color: #000000; }");
	}
}

////////////////////////////////////

TripleNavigatorFrame::TripleNavigatorFrame(MiniViewContainer * miniViewContainer, const QString & title, QWidget * parent) : QFrame(parent)
{
	m_miniViewContainer = miniViewContainer;
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	this->setLayout(layout);
	layout->addWidget(miniViewContainer);
	m_tripleNavigatorLabel = new TripleNavigatorLabel(this);
	m_tripleNavigatorLabel->setMiniViewContainer(miniViewContainer);
	m_tripleNavigatorLabel->setObjectName("tripleNavigatorLabel");
	m_tripleNavigatorLabel->setText(title);
	m_tripleNavigatorLabel->setFixedHeight(15);
	m_tripleNavigatorLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(m_tripleNavigatorLabel);
}

void TripleNavigatorFrame::hook(MiniViewContainer * miniViewContainer) {
	//connect(miniViewContainer, SIGNAL(), this->
}