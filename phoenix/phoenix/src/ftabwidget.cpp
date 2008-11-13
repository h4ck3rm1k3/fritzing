/*
 *  ftabwidget.cpp
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/24/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include "ftabwidget.h"

FTabWidget::FTabWidget(QWidget * parent)
	: QTabWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QTabBar * FTabWidget::tabBar() {
	return QTabWidget::tabBar();
}
