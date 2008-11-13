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

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef ZOOMCOMBOBOX_H_
#define ZOOMCOMBOBOX_H_

#include<QComboBox>
#include<QEvent>

class ZoomComboBox : public QComboBox {
Q_OBJECT

public:
	ZoomComboBox(QWidget * parent=0);
	void zoomOut();
	void zoomIn();
	QString editText();
	void setEditText(QString newText);

protected slots:
	void itemAdded();
	void inputTextChanged();
	void emitZoomChanged();
	void updateBackupFieldsIfOptionSelected(int index);

protected:
	void addPercentageToInputText();
	void focusOutEvent ( QFocusEvent * event );
	void keyPressEvent ( QKeyEvent * event );
	void setPreviousValue();
	int itemIndex(QString value);
	void updateBackupFields();
	int findCloserIndexToCurrentValue(bool upper);

	static QStringList ZoomFactors;
	bool m_userStillWriting;
	QString m_valueBackup;
	int m_indexBackup;
	int m_lastKeyPressed;

public:
	static void loadFactors();
	static qreal ZoomStep;

signals:
	void zoomChanged(qreal newZoom);
};

#endif /* ZOOMCOMBOBOX_H_ */
