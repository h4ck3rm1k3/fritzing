/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#ifndef PETOOLVIEW_H
#define PETOOLVIEW_H

#include <QFrame>
#include <QRadioButton>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QListWidget>
#include <QDomDocument>
#include <QDoubleSpinBox>

class PEDoubleSpinBox : public QDoubleSpinBox
{
Q_OBJECT
public:
    PEDoubleSpinBox(QWidget * parent = 0);

    void stepBy (int steps);

signals:
    void getSpinAmount(double &);
};

class PEToolView : public QWidget
{
Q_OBJECT
public:
	PEToolView(QWidget * parent = NULL);
	~PEToolView();

    void highlightElement(class PEGraphicsItem *);
    void initConnectors(QList<QDomElement> & connectorList, bool gotZeroConnector);
    void setLock(bool);
    QDomElement currentConnector();
    void setTerminalPointCoords(QPointF);
    void setTerminalPointLimits(QSizeF);

signals:
    void switchedConnector(const QDomElement &);
    void lockChanged(bool);
    void terminalPointChanged(const QString & how);
    void terminalPointChanged(const QString & coord, double value);
    void getSpinAmount(double &);

protected slots:
    void changeUnits();
    void switchConnector(QListWidgetItem * current, QListWidgetItem * previous);
    void lockChangedSlot(bool);
    void descriptionEntry();
    void typeEntry();
    void nameEntry();
    void buttonChangeTerminalPoint();
    void terminalPointEntry();
    void getSpinAmountSlot(double &);


protected:
    QString convertUnitsStr(double);
    double convertUnits(double);
    void enableChanges(bool);

protected:
    QLabel * m_svgElement;
    QLabel * m_height;
    QLabel * m_width;
    QLabel * m_x;
    QLabel * m_y;
    QListWidget * m_connectorListWidget;
    QList<QPushButton *> m_buttons;
    QString m_units;
    class PEGraphicsItem * m_pegi;
    QList<QDomElement> m_connectorList;
    QGroupBox * m_connectorInfoGroupBox;
    QBoxLayout * m_connectorInfoLayout;
    QWidget * m_connectorInfoWidget;
    bool m_gotZeroConnector;
    QCheckBox * m_elementLock;
    QDoubleSpinBox * m_terminalPointX;
    QDoubleSpinBox * m_terminalPointY;
};

#endif
