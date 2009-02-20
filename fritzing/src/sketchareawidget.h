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



#ifndef SKETCHAREAWIDGET_H_
#define SKETCHAREAWIDGET_H_

#include <QFrame>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMainWindow>

#include "itembase.h"
#include "sketchwidget.h"
#include "zoomcombobox.h"


class SketchAreaWidget : public QFrame {
public:
	SketchAreaWidget(SketchWidget *graphicsView, QMainWindow *parent);
	virtual ~SketchAreaWidget();

	ItemBase::ViewIdentifier viewIdentifier();
	SketchWidget* graphicsView();
	ZoomComboBox *zoomComboBox();

	void setContent(QList<QWidget*> buttons, ZoomComboBox *zoomComboBox);
	void addStatusBar(QStatusBar *);
	static QWidget *separator(QWidget* parent);

protected:
	void createLayout();

public:
	static const QString RoutingStateLabelName;

protected:
	SketchWidget *m_graphicsView;
	ZoomComboBox *m_zoomComboBox;

	QFrame *m_toolbar;
	QHBoxLayout *m_buttonsContainer;
	QVBoxLayout *m_labelContainer;
	QHBoxLayout *m_zoomContainer;
	QFrame *m_statusBarArea;
};

#endif /* SKETCHAREAWIDGET_H_ */
