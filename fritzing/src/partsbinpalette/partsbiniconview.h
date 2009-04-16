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


#ifndef ICONVIEW_H_
#define ICONVIEW_H_

#include <QFrame>
#include <QGraphicsView>

#include "partsbinview.h"
#include "../infographicsview.h"

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class PartsBinIconView : public InfoGraphicsView, public PartsBinView
{
	Q_OBJECT
	public:
		PartsBinIconView(ReferenceModel* refModel, QWidget *parent=0);
		void loadFromModel(class PaletteModel *);
		void setPaletteModel(class PaletteModel *model, bool clear=false);
		void addPart(ModelPart * model, int position = -1);
		void removePart(const QString &moduleID);

		bool swappingEnabled(ItemBase *);

		ModelPart *selected();
		int selectedIndex();
	protected:
		void doClear();
		void moveItem(int fromIndex, int toIndex);
		int itemIndexAt(const QPoint& pos);

		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void dragMoveEvent(QDragMoveEvent* event);
		void dropEvent(QDropEvent* event);

		void setItemAux(ModelPart *, int position = -1);

		void resizeEvent(QResizeEvent * event);
		void updateSize(QSize newSize);
		void updateSize();
		void updateSizeAux(int width);
		void setupLayout();

		void showInfo(class SvgIconWidget * item);

	public slots:
		void setSelected(int position, bool doEmit=false);
		void informNewSelection();
		void itemMoved(int fromIndex, int toIndex);

	signals:
		void informItemMoved(int fromIndex, int toIndex);
		void selectionChanged(int index);

	protected:
		LayerHash m_viewLayers;

		QGraphicsWidget *m_layouter;
		class GraphicsFlowLayout *m_layout;

		QMenu *m_itemMenu;
		bool m_noSelectionChangeEmition;
};

#endif /* ICONVIEW_H_ */
