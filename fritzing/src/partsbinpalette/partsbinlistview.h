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


#ifndef LISTVIEW_H_
#define LISTVIEW_H_

#include <QListWidget>
#include <QMouseEvent>

#include "partsbinview.h"

class PartsBinListView : public QListWidget, public PartsBinView {
	Q_OBJECT
	public:
		PartsBinListView(QWidget * parent = 0);
		~PartsBinListView();
		void setInfoView(class HtmlInfoView *);
		void removePart(const QString &moduleID);

		ModelPart *selected();
		bool swappingEnabled();

	protected:
		void setModel(PaletteModel *model);
		void doClear();
		void setItemAux(ModelPart * modelPart);
		void mousePressEvent(QMouseEvent * event);
		void mouseMoveEvent(QMouseEvent * event);
		ModelPart *itemModelPart(const QListWidgetItem *item);
		const QString& itemModuleID(const QListWidgetItem *item);

		void showInfo(QListWidgetItem * item);

	protected:
		class HtmlInfoView * m_infoView;
		QListWidgetItem * m_hoverItem;

};
#endif /* LISTVIEW_H_ */
