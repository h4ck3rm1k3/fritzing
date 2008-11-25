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


#ifndef ICONVIEW_H_
#define ICONVIEW_H_

#include <QFrame>
#include <QGraphicsView>

#include "partsbinview.h"
#include "svgiconwidget.h"
#include "graphicsflowlayout.h"
#include "../paletteitem.h"
#include "../palettemodel.h"
#include "../viewgeometry.h"
#include "../infographicsview.h"

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class PartsBinIconView : public InfoGraphicsView, public PartsBinView
{
	Q_OBJECT
	public:
		PartsBinIconView(QWidget *parent=0);
		void loadFromModel(PaletteModel *);
		void setPaletteModel(PaletteModel *model, bool clear=false);
		void addPart(ModelPart * model);
		void removePart(const QString &moduleID);

		bool swappingEnabled();

		ModelPart *selected();
	protected:
		void doClear();
		void mousePressEvent(QMouseEvent *event);
		void setItemAux(ModelPart *);

		void resizeEvent(QResizeEvent * event);
		void updateSize(QSize newSize);
		void updateSize();
		void updateSizeAux(int width);
		void setupLayout();

		void setFirstSelected();

	protected:
		LayerHash m_viewLayers;

		QGraphicsWidget *m_layouter;
		GraphicsFlowLayout *m_layout;

		QMenu *m_itemMenu;
};

#endif /* ICONVIEW_H_ */
