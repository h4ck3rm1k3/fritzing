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

$Revision: 1714 $:
$Author: merunga $:
$Date: 2008-12-03 15:00:18 +0100 (Wed, 03 Dec 2008) $

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

void TripleNavigator::addView(MiniViewContainer * miniViewContainer) 
{
	QFrame * frame = new QFrame(this);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	frame->setLayout(layout);
	layout->addWidget(miniViewContainer);
	TripleNavigatorLabel * tripleNavigatorLabel = new TripleNavigatorLabel;
	tripleNavigatorLabel->setText("hello");
	layout->addWidget(tripleNavigatorLabel);
	m_splitter->addWidget(frame);
}
