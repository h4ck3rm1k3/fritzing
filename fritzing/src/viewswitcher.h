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

$Revision: 1490 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 13:10:48 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

#ifndef VIEWSWITCHER_H_
#define VIEWSWITCHER_H_

#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

#include "help/inotseeninminiview.h"

class ViewSwitcherPrivate;

class ViewSwitcherButton : public QLabel {
	Q_OBJECT

	public:
		ViewSwitcherButton(const QString &view, int index, ViewSwitcherPrivate *parent);
		void setFocus(bool active);
		void setActive(bool selected);
		void setHover(bool hover);
		int index();

	signals:
		void clicked(ViewSwitcherButton*);

	protected:
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void updateImage();

	protected:
		bool m_focus;
		bool m_active;
		bool m_hover;
		int m_index;
		QString m_resourcePath;
		ViewSwitcherPrivate *m_parent;

	protected:
		static QString ResourcePathPattern;
};

class ViewSwitcherPrivate : public QFrame {
	Q_OBJECT
	public:
		ViewSwitcherPrivate();

	signals:
		void viewSwitched(int index);

	public slots:
		void updateHoverState(ViewSwitcherButton* hoverOne = NULL);
		void viewSwitchedTo(int);

	protected slots:
		void updateState(ViewSwitcherButton* clickedOne, bool doEmit=true);

	protected:
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);

		ViewSwitcherButton *createButton(const QString &view);

	protected:
		QHBoxLayout *m_layout;
		QList<ViewSwitcherButton*> m_buttons;
};

class ViewSwitcher : public QGraphicsProxyWidget, public INotSeenInMiniView {
public:
	ViewSwitcher(QWidget *parent);
};

#endif /* VIEWSWITCHER_H_ */
