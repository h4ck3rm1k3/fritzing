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

/*
 * Class that manage all the interactive helps in the main window
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <QObject>

#include "sketchmainhelp.h"
#include "../mainwindow.h"

class Helper : public QObject {
	Q_OBJECT
	public:
		Helper(MainWindow *owner);

	protected slots:
		void init();
		void viewResized(const QSize& oldSize, const QSize& newSize);
		void somethingDroppedIntoView();

	protected:
		void addAndCenterItemInView(SketchMainHelp *item, SketchWidget* view);
		void centerItemInView(SketchMainHelp *item, SketchWidget* view);
		void moveItemBy(SketchMainHelp *item, qreal dx, qreal dy);

	protected:
		MainWindow *m_owner;

		SketchMainHelp *m_breadMainHelp;
		SketchMainHelp *m_schemMainHelp;
		SketchMainHelp *m_pcbMainHelp;

		bool m_stillWaitingFirstDrop;

	protected:
		static QString BreadboardHelpText;
		static QString SchematicHelpText;
		static QString PCBHelpText;

		static QString BreadboardHelpImage;
		static QString SchematicHelpImage;
		static QString PCBHelpImage;
};

#endif /* HELPER_H_ */
