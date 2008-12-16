/*
 * (c) Fachhochschule Potsdam
 */

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

	protected slots:
		void updateState(ViewSwitcherButton* clickedOne, bool doEmit=true);
		void viewSwitchedTo(int);

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
