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



#ifndef SKETCHAREAWIDGET_H_
#define SKETCHAREAWIDGET_H_

#include <QFrame>
#include <QHBoxLayout>

#include "itembase.h"
#include "sketchwidget.h"
#include "zoomcombobox.h"

class SketchAreaWidget : public QFrame {
public:
	SketchAreaWidget(SketchWidget *graphicsView, QWidget *parent=0);
	virtual ~SketchAreaWidget();

	ItemBase::ViewIdentifier viewIdentifier();
	SketchWidget* graphicsView();

	void setContent(QList<QWidget*> buttons, ZoomComboBox *zoomComboBox);

protected:
	void createLayout();

protected:
	SketchWidget *m_graphicsView;

	QFrame *m_toolbar;
	QHBoxLayout *m_buttonsContainer;
	QHBoxLayout *m_labelContainer;
	QHBoxLayout *m_zoomContainer;
};

#endif /* SKETCHAREAWIDGET_H_ */
