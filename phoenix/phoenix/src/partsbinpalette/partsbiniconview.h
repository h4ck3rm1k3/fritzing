/*
 * (c) Fachhochschule Potsdam
 */

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

		void createItemMenu(QList<QAction*> &actions){
			m_itemMenu = new QMenu(QObject::tr("Icon"), this);

			foreach(QAction* action, actions) {
				m_itemMenu->addAction(action);
			}
		}

		bool swappingEnabled();

		bool alreadyIn(QString moduleID);

		PaletteItem *selected();
	protected:
		void setModel(PaletteModel *model);
		void doClear();
		void mousePressEvent(QMouseEvent *event);
		void setItemAux(ModelPart *);

		void resizeEvent(QResizeEvent * event);
		void updateSize(QSize newSize);
		void updateSize();
		void updateSizeAux(int width);
		void setupLayout();

	protected:
		LayerHash m_viewLayers;
		PaletteModel * m_model;

		QGraphicsWidget *m_layouter;
		GraphicsFlowLayout *m_layout;

		QHash<QString /*moduleId*/,SvgIconWidget*> m_iconHash;

		QMenu *m_itemMenu;
};

#endif /* ICONVIEW_H_ */
