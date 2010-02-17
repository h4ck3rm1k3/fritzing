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

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef ZOOMSLIDER_H_
#define ZOOMSLIDER_H_

#include <QEvent>
#include <QFrame>
#include <QString>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>

class ZoomSlider: public QFrame {
Q_OBJECT

public:
	ZoomSlider(QWidget * parent=0);

	void setValue(qreal);
	qreal value();

	void zoomOut();
	void zoomIn();

protected slots:
	//void emitZoomChanged();
	//void updateBackupFieldsIfOptionSelected(int index);
	void sliderValueChanged(int newValue);
	void sliderTextEdited(const QString & newText);
	void minusClicked();
	void plusClicked();

protected:
	void step(int direction);
	void sliderTextEdited(const QString & newValue, bool doEmit);
	static void loadFactors();

protected:
	//int itemIndex(QString value);
	//void updateBackupFields();

	static QList<qreal> ZoomFactors;
	//bool m_userStillWriting;
	//QString m_valueBackup;
	//int m_indexBackup;
	QSlider * m_slider;
	QLineEdit * m_lineEdit;
	QPushButton * m_plusButton;
	QPushButton * m_minusButton;

public:
	static qreal ZoomStep;

signals:
	void zoomChanged(qreal newZoom);
};

#endif /* ZOOMSLIDER_H_ */
