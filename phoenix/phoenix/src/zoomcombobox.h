/*
 * (c) Fachhochschule Potsdam
 */

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
