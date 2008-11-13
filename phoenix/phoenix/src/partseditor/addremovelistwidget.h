/*
 * (c) Fachhochschule Potsdam
 */

#ifndef ADDREMOVELISTWIDGET_H_
#define ADDREMOVELISTWIDGET_H_

#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>

class AddRemoveListWidget : public QGroupBox {
	Q_OBJECT

	public:
		AddRemoveListWidget(QString title, QWidget *parent=0);
		int count();
		QListWidgetItem* itemAt(int rowIdx);
		QStringList& getItemsText();
		void setItemsText(const QStringList& texts);

	protected slots:
		void addItem();
		void addItem(QString itemText);
		void removeSelectedItems();

	protected:
		QLabel *m_label;

		QPushButton *m_addButton;
		QPushButton *m_removeButton;

		QListWidget *m_list;
};

#endif /* ADDREMOVELISTWIDGET_H_ */
