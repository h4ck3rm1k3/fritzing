/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2012 Fachhochschule Potsdam - http://fh-potsdam.de

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



#ifndef METADATAVIEW_H
#define METADATAVIEW_H

#include <QFrame>
#include <QTimer>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>

#include "../items/itembase.h"
#include "../items/wire.h"
#include "../connectors/connectoritem.h"
#include "../referencemodel/referencemodel.h"


class MetadataView : public QScrollArea
{
Q_OBJECT
public:
	MetadataView(QWidget * parent = 0);
	~MetadataView();

    void initMetadata(const QDomDocument &);

signals:
    void metadataChanged(const QString & name, const QString & value);
    void propertiesChanged(const QHash<QString, QString> &);
    void tagsChanged(const QStringList &);

protected slots:
    void titleEntry();
    void authorEntry();
    void descriptionEntry();
    void labelEntry();
    void familyEntry();
    void dateEntry();
    void propertiesEntry();
    void tagsEntry();

protected:
    class PEMainWindow * peMainWindow();

protected:
    QPointer<QLineEdit> m_titleEdit;
    QPointer<QLineEdit> m_dateEdit;
    QPointer<QLineEdit> m_authorEdit;
    QPointer<QLineEdit> m_familyEdit;
    QPointer<QLineEdit> m_labelEdit;
    QPointer<QTextEdit> m_descriptionEdit;
    QPointer<class HashPopulateWidget> m_propertiesEdit;
    QPointer<class HashPopulateWidget> m_tagsEdit;
    QPointer<QFrame> m_mainFrame;
};


class FocusOutTextEdit :public QTextEdit
{
    Q_OBJECT
public:
    FocusOutTextEdit(QWidget * parent = 0);
    ~FocusOutTextEdit();

signals:
    void focusOut();

protected:
	void focusOutEvent(QFocusEvent *);
};


#endif
