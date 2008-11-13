/*
 * (c) Fachhochschule Potsdam
 */

#ifndef PARTCONNECTORSWIDGET_H_
#define PARTCONNECTORSWIDGET_H_

#include <QFrame>

class PartConnectorsWidget : public QFrame {
	Q_OBJECT
	public:
		PartConnectorsWidget(QList<QWidget*> widgets, QWidget *parent=0);
};

#endif /* PARTCONNECTORSWIDGET_H_ */
