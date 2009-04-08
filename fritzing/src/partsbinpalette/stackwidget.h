/*
 * (c) Fachhochschule Potsdam
 */

#ifndef STACKWIDGET_H_
#define STACKWIDGET_H_

#include <QFrame>
#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>

class StackTabBar : public QTabBar {
	Q_OBJECT
	public:
		StackTabBar(class StackTabWidget *parent);

	signals:
		void tabDetached(QWidget *tab, const QPoint &pos);

	protected:
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent( QMouseEvent *event);

		int tabIndexAtPos(const QPoint &p) const;

		int m_pressedIndex;
		StackTabWidget* m_parent;
};

////////////////////////////////////////////////////////

class StackTabWidget : public QTabWidget {
public:
	StackTabWidget(QWidget *parent=0);
};

////////////////////////////////////////////////////////

class StackWidgetSeparator : public QFrame {
	Q_OBJECT
	public:
		StackWidgetSeparator(QWidget *parent=0);
		void setDragging(bool);

	signals:
		void setReceptor(StackWidgetSeparator*);

	protected:
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		void expand();
		void shrink();

		bool m_dragging;
};

////////////////////////////////////////////////////////

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

	public slots:
		void tabDetached(QWidget *tab, const QPoint &pos);
		void setCurrentIndex(int index);
		void setCurrentWidget(QWidget *widget);
		void setReceptor(StackWidgetSeparator* receptor);

	signals:
		void currentChanged(int index);
		void widgetRemoved(int index);

	protected:
		int closestIndexToPos(const QPoint &pos);
		StackWidgetSeparator *newSeparator();

		QVBoxLayout *m_layout;
		QWidget *m_current;
		StackWidgetSeparator *m_dropReceptor;
};

#endif /* STACKWIDGET_H_ */
