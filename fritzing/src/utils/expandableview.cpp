/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "expandableview.h"
#include <QLabel>
#include <QHBoxLayout>

ExpandableView::ExpandableView(const QString & text, QWidget * parent) : QGroupBox("", parent)
{
	setObjectName("expandableViewFrame");
	m_childFrame = NULL;
	m_vLayout = new QVBoxLayout(this);
	m_vLayout->setMargin(0);

	QFrame * frame = new QFrame(this);
	frame->setObjectName("expandableViewLabelFrame");
	QHBoxLayout * layout = new QHBoxLayout(frame);
	layout->setMargin(0);

	QLabel * label = new QLabel(text, frame);
	label->setObjectName("expandableViewLabel");
	layout->addWidget(label);
	layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
	m_expander = new ClickableLabel("[-]", frame);
	m_expander->setObjectName("expander");
	connect(m_expander, SIGNAL(clicked()), this, SLOT(expanderClicked()));
	layout->addWidget(m_expander);

	m_vLayout->addWidget(frame);
}

void ExpandableView::expanderClicked() {
	if (m_childFrame == NULL) return;

	m_childFrame->setVisible(!m_childFrame->isVisible());
	m_expander->setText(m_childFrame->isVisible() ? "[-]" : "[+]");
	emit expanded(m_childFrame->isVisible());
}

void ExpandableView::setChildFrame(QFrame * childFrame) {

	if (m_childFrame) {
		m_vLayout->removeWidget(m_childFrame);
	}

	m_childFrame = childFrame;
	m_vLayout->addWidget(childFrame);
}


