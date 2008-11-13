/*
 * (c) Fachhochschule Potsdam
 */

#ifndef EDITABLELABEL_H_
#define EDITABLELABEL_H_

#include <QWidget>
#include <QLabel>

class EditableLabel : public QLabel {
	Q_OBJECT

	public:
		EditableLabel(QWidget *parent=0);
		EditableLabel(const QString & text, QWidget *parent=0);

	public slots:
		void editionCompleted(QString newText);

	signals:
		void editionStarted(QString);

	protected:
		void mousePressEvent(QMouseEvent *event);
};

#endif /* EDITABLELABEL_H_ */
