/*
 * (c) Fachhochschule Potsdam
 */

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

		PaletteItem *selected() {
			// TODO Mariano
			return NULL;
		}

		bool swappingEnabled() {
			return false;
		}

	protected:
		void setModel(PaletteModel *model);
		void doClear();
		void setItemAux(ModelPart * modelPart);
		void mousePressEvent(QMouseEvent * event);
		void mouseMoveEvent(QMouseEvent * event);

	protected:
		class HtmlInfoView * m_infoView;
		QListWidgetItem * m_hoverItem;
		PaletteModel *m_model;

		QPixmap *m_pixmap;
};
#endif /* LISTVIEW_H_ */
