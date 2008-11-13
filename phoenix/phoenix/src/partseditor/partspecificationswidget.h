/*
 * (c) Fachhochschule Potsdam
 */

#ifndef PARTSPECIFICATIONSWIDGET_H_
#define PARTSPECIFICATIONSWIDGET_H_

#include <QScrollArea>

class PartSpecificationsWidget : public QScrollArea {
	Q_OBJECT
	public:
		PartSpecificationsWidget(QList<QWidget*> widgets, QWidget *parent=0);
		QSize sizeHint();

	protected slots:
		void updateLayout();

	protected:
		QFrame *m_scrollContent;
};

#endif /* PARTSPECIFICATIONSWIDGET_H_ */
