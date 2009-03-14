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


#include <QLabel>
#include <QScrollArea>
#include <QFont>
#include <QChar>
#include <QTime>
#include <QTimer>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QScrollBar>
#include <QMessageBox>


#include "../itembase.h"
#include "../pcbsketchwidget.h"
//#include "sketchwidget.h"
#include "fritzing2eagle.h"



Fritzing2Eagle* Fritzing2Eagle::singleton = NULL;

Fritzing2Eagle::Fritzing2Eagle(PCBSketchWidget *m_pcbGraphicsView)
{
	QList <ItemBase*> partList;
	
	m_pcbGraphicsView->collectParts(partList);
	
	singleton = this;
}

/*
void Fritzing2Eagle::showOutputInfo(PCBSketchWidget m_pcbGraphicsView) 
{
	if (singleton == NULL) {
		new Fritzing2Eagle();
	}
	
}
*/


