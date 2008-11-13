/*
 *  ftabwidget.h
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/24/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#ifndef FTABWIDGET_H
#define FTABWIDGET_H

#include <QTabWidget>
#include "debugdialog.h"

class FTabWidget : public QTabWidget
{
	Q_OBJECT

public:
	FTabWidget(QWidget * parent = 0);

	QTabBar * tabBar();
};

#endif
