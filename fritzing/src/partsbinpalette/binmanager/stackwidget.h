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


#ifndef STACKWIDGET_H_
#define STACKWIDGET_H_

#include <QFrame>
#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QHash>

class StackTabWidget;

class StackWidget : public QFrame {
	Q_OBJECT
	public:
		StackWidget(QWidget *parent=0);

		int addWidget(QWidget *widget);
		int count() const;
		int currentIndex() const;
		QWidget *currentWidget() const;
		int indexOf(QWidget *widget) const;
		void insertWidget(int index, QWidget *widget);
		void removeWidget(QWidget *widget);
		QWidget *widget(int index) const;
		bool contains(QWidget *widget) const;

		void setDock(class FDockWidget*);

	public slots:
		void setCurrentIndex(int index);
		void setCurrentWidget(QWidget *widget);

	signals:
		void currentChanged(int index);
		void widgetRemoved(int index);

	protected:
		QVBoxLayout *m_layout;
		QWidget *m_current;


};

#endif /* STACKWIDGET_H_ */
