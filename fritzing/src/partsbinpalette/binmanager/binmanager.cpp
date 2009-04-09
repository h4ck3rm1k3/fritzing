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

$Revision: 2776 $:
$Author: merunga $:
$Date: 2009-04-02 13:54:08 +0200 (Thu, 02 Apr 2009) $

********************************************************************/


#include <QVBoxLayout>
#include "binmanager.h"
#include "stacktabwidget.h"
#include "../../debugdialog.h"

BinManager::BinManager(QWidget *parent)
	: QFrame(parent) {
	m_widget = new StackWidget(this);
	m_widget->setAcceptDrops(true);
	//m_activeBinTabWidget = new QTabWidget(m_widget);
	//m_widget->addWidget(m_activeBinTabWidget);

	for(int i=0; i < 4; i++) {
		StackTabWidget *tb = new StackTabWidget(m_widget);
		for(int j=0; j < 6; j++) {
			QFrame *f = new QFrame(m_widget);
			f->setFixedWidth(300);
			f->setFixedHeight(500);
			f->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
			tb->addTab(f,QString("tab %1%2").arg(i).arg(j));
// this functions are only available on 4.5.0 or later
#if QT_VERSION >= 0x040500
			tb->setTabsClosable(true);
			//tb->setMovable(true);
#endif
		}
		m_widget->addWidget(tb);
	}
	QVBoxLayout *lo = new QVBoxLayout(this);
	lo->addWidget(m_widget);
	setMaximumHeight(500);
}

BinManager::~BinManager() {

}

