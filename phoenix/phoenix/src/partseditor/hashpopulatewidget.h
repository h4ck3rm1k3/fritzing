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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/



#ifndef HASHPOPULATEWIDGET_H_
#define HASHPOPULATEWIDGET_H_

#include <QFrame>
#include <QLabel>
#include <QHash>
#include <QLineEdit>
#include <QGridLayout>
#include <QUndoStack>

class HashLineEdit : public QLineEdit {
	Q_OBJECT
	public:
		HashLineEdit(QUndoStack *undoStack, const QString &text, bool defaultValue = false, QWidget *parent = 0);
		bool hasChanged();
		QString textIfSetted();

	protected slots:
		void updateObjectName();
		void updateStackState();

	protected:
		void mousePressEvent(QMouseEvent * event);
		void focusOutEvent(QFocusEvent * event);

		QString m_firstText;
		bool m_isDefaultValue;
		QUndoStack *m_undoStack;
};

class HashRemoveButton : public QLabel {
	Q_OBJECT
	public:
		HashRemoveButton(HashLineEdit* label, HashLineEdit* value, QWidget *parent) : QLabel(parent) {
			m_enterIcon = QPixmap(":/resources/images/remove_prop_enter.png");
			m_leaveIcon = QPixmap(":/resources/images/remove_prop_leave.png");
			m_label = label;
			m_value = value;
			setPixmap(m_leaveIcon);
		}

		HashLineEdit *label() {return m_label;}
		HashLineEdit *value() {return m_value;}

	signals:
		void clicked(HashRemoveButton*);

	protected:
		void mousePressEvent(QMouseEvent * event) {
			emit clicked(this);
			QLabel::mousePressEvent(event);
		}

		void enterEvent(QEvent * event) {
			setPixmap(m_enterIcon);
			QLabel::enterEvent(event);
		}

		void leaveEvent(QEvent * event) {
			setPixmap(m_leaveIcon);
			QLabel::leaveEvent(event);
		}

	protected:
		HashLineEdit *m_label;
		HashLineEdit *m_value;

		QPixmap m_enterIcon;
		QPixmap m_leaveIcon;
};

class HashPopulateWidget : public QFrame {
	Q_OBJECT
	public:
		HashPopulateWidget(QString title, QHash<QString,QString> &initValues, const QStringList &readOnlyKeys, QUndoStack *undoStack, QWidget *parent = 0);
		const QHash<QString,QString> & hash();

	protected slots:
		void lastRowEditionCompleted();
		void removeRow(HashRemoveButton*);

	signals:
		void editionStarted();

	protected:
		void addRow(QGridLayout *layout = 0);
		QGridLayout* gridLayout();
		HashLineEdit* lineEditAt(int row, int col);
		HashRemoveButton *createRemoveButton(HashLineEdit* label, HashLineEdit* value);

		QHash<QString,QString> m_hash;

		HashLineEdit *m_lastLabel;
		HashLineEdit *m_lastValue;

		int m_currRow;
		QUndoStack *m_undoStack;
};

#endif /* HASHPOPULATEWIDGET_H_ */
