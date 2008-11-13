/*
 * (c) Fachhochschule Potsdam
 */

#ifndef ABSTRACTCONNECTORINFOWIDGET_H_
#define ABSTRACTCONNECTORINFOWIDGET_H_

#include <QFrame>
#include <QFile>

class AbstractConnectorInfoWidget : public QFrame {
	Q_OBJECT
	public:
		AbstractConnectorInfoWidget(QWidget *parent=0);
		virtual void setSelected(bool selected, bool doEmitChange=true);
		bool isSelected();
		QSize sizeHint();

	signals:
		void tellSistersImNewSelected(AbstractConnectorInfoWidget*); // Meant to be used in the info context
		void tellViewsMyConnectorIsNewSelected(const QString&); // Meant to be used in the info context

	protected:
		void reapplyStyle();

		volatile bool m_isSelected;
};

#endif /* ABSTRACTCONNECTORINFOWIDGET_H_ */
