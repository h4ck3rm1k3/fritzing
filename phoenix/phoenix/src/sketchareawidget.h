/*
 * (c) Fachhochschule Potsdam
 */

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
	QHBoxLayout *m_zoomContainer;
};

#endif /* SKETCHAREAWIDGET_H_ */
