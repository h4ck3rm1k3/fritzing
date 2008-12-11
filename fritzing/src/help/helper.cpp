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

$Revision: 1490 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 13:10:48 +0100 (Thu, 13 Nov 2008) $

********************************************************************/


#include "helper.h"

QString Helper::BreadboardHelpText = tr(
	"Sketches and Prototypes usally start in the <b>Breadboard View</b>."
	"<br/>"
	"Begin by dragging out a Bradboard Part from the PArts Bin."
	"Then populate the breadboard with the components, just like they are arranged in the real world."
	"<br/>"
	"The Arduino part wil turn into an Arduino Shield.");
QString Helper::BreadboardHelpImage = ":/resources/images/helpImageBreadboard.png";

Helper::Helper(MainWindow *owner) : QObject(owner) {
	m_breadMainHelp = new SketchMainHelp(BreadboardHelpImage,BreadboardHelpText);
	owner->m_breadboardGraphicsView->scene()->addItem(m_breadMainHelp);
}
