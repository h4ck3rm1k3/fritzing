/*
 * (c) Fachhochschule Potsdam
 */

#ifndef PARTINFOWIDGET_H_
#define PARTINFOWIDGET_H_

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>

#include "addremovelistwidget.h"
#include "../modelpart.h"
#include "../modelpartstuff.h"

class PartInfoWidget : public QWidget {
	Q_OBJECT
	public:
		PartInfoWidget(QWidget *parent = 0);
		ModelPartStuff* modelPartStuff();

	public slots:
		void updateInfo(ModelPart *);

	protected:
		QLineEdit *m_version;
		QLineEdit *m_author;
		QLineEdit *m_title;
		//QLineEdit *m_taxonomy;
		QLineEdit *m_label;
		QTextEdit *m_description;

		QDateEdit *m_date;

		AddRemoveListWidget * m_tagsList;
		AddRemoveListWidget * m_propertyList;
};

#endif /* PARTINFOWIDGET_H_ */
