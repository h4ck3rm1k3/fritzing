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
#include "misc.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>


const QString labelStyle = "#tripleNavigatorLabel { color: %1; font-weight: %2; }";
const QString pressedStyleColor = "#ffffff";
const QString pressedStyleWeight = "bold";
const QString normalStyleColor = "#000000";
const QString normalStyleWeight = "normal";
const QString hoverStyleColor = "#e5e5e5";
const QString hoverStyleWeight = "normal";

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

	for (int i = 0; i < m_splitter->count(); i++) {
		frame->hook(((TripleNavigatorFrame *) m_splitter->widget(i))->miniViewContainer());
	}

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

void TripleNavigatorLabel::setMiniViewContainer(MiniViewContainer * miniViewContainer) 
{
	m_miniViewContainer = miniViewContainer;
}

void TripleNavigatorLabel::navigatorMousePressedSlot(MiniViewContainer * miniViewContainer) {
	if (miniViewContainer == m_miniViewContainer) {
		setStyleSheet(labelStyle.arg(pressedStyleColor).arg(pressedStyleWeight));
		m_currentStyleColor = pressedStyleColor;
		m_currentStyleWeight = pressedStyleWeight;
		m_miniViewContainer->hideHandle(false);
	}
	else {
		setStyleSheet(labelStyle.arg(normalStyleColor).arg(normalStyleWeight));
		m_currentStyleColor = normalStyleColor;
		m_currentStyleWeight = normalStyleWeight;
		m_miniViewContainer->hideHandle(true);
	}
}

void TripleNavigatorLabel::navigatorMouseEnterSlot(MiniViewContainer * miniViewContainer) {
	if (miniViewContainer == m_miniViewContainer) {
		if (m_currentStyleColor != pressedStyleColor) {
			setStyleSheet(labelStyle.arg(hoverStyleColor).arg(hoverStyleWeight));
		}
	}
}

void TripleNavigatorLabel::navigatorMouseLeaveSlot(MiniViewContainer * miniViewContainer) {
	if (miniViewContainer == m_miniViewContainer) {
		setStyleSheet(labelStyle.arg(m_currentStyleColor).arg(m_currentStyleWeight));
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
	m_tripleNavigatorLabel->setFixedHeight(11);
	m_tripleNavigatorLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
	layout->addWidget(m_tripleNavigatorLabel);
	installEventFilter(this);
}

void TripleNavigatorFrame::hook(MiniViewContainer * miniViewContainer) {
	connect(miniViewContainer, SIGNAL(navigatorMousePressedSignal(MiniViewContainer *)), 
		    this->m_tripleNavigatorLabel, SLOT(navigatorMousePressedSlot(MiniViewContainer *)) );
	connect(miniViewContainer, SIGNAL(navigatorMouseEnterSignal(MiniViewContainer *)), 
		    this->m_tripleNavigatorLabel, SLOT(navigatorMouseEnterSlot(MiniViewContainer *)) );
	connect(miniViewContainer, SIGNAL(navigatorMouseLeaveSignal(MiniViewContainer *)), 
		    this->m_tripleNavigatorLabel, SLOT(navigatorMouseLeaveSlot(MiniViewContainer *)) );
}

MiniViewContainer * TripleNavigatorFrame::miniViewContainer() {
	return m_miniViewContainer;
}

bool TripleNavigatorFrame::eventFilter(QObject *obj, QEvent *event)
{
	if (m_miniViewContainer) {
		switch (event->type()) {
			case QEvent::MouseButtonPress:
			case QEvent::NonClientAreaMouseButtonPress:
				if (obj == this || isParent(this, obj)) {
					m_miniViewContainer->miniViewMousePressedSlot();
				}
				break;
			case QEvent::Enter:
				if (obj == this || isParent(this, obj)) {
					m_miniViewContainer->miniViewMouseEnterSlot();
				}
				break;
			case QEvent::Leave:
				if (obj == this || isParent(this, obj)) {
					m_miniViewContainer->miniViewMouseLeaveSlot();
				}
				break;
			default:
				break;
		}
	}

	return QObject::eventFilter(obj, event);
}
