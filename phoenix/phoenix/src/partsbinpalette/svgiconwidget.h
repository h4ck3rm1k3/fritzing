/*
 * (c) Fachhochschule Potsdam
 */

#ifndef SVGICONWIDGET_H_
#define SVGICONWIDGET_H_

#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QFrame>
#include <QToolTip>

#include "../modelpart.h"
#include "../paletteitem.h"

class SvgIconWidgetContainer;

class SvgIconWidget : public QGraphicsProxyWidget {
	Q_OBJECT
	public:
		SvgIconWidget(ModelPart *, ItemBase::ViewIdentifier, const LayerHash & viewLayers, long id, QMenu * itemMenu);
		~SvgIconWidget();
		PaletteItem *paletteItem();
		QPoint globalPos();

	protected:

		enum StyleSheetType {
			SELECTEDSTYLESHEET = 1,
			NONSELECTEDSTYLESHEET = 0
		};

		void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
		void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0);

		PaletteItem *m_paletteItem;
		QLabel *m_pixmapContainer;
		SvgIconWidgetContainer *m_container;
		StyleSheetType m_styleSheetType;
};

class SvgIconWidgetContainer : public QFrame {
	public:
		SvgIconWidgetContainer(PaletteItem *paletteItem, SvgIconWidget *parent) : QFrame() {
			m_parent = parent;
			m_paletteItem = paletteItem;
		}

	protected:
		bool event(QEvent * event) {
			if(event->type() == QEvent::ToolTip) {
				QHelpEvent *tooltipEvent = (QHelpEvent*)event;
				QToolTip::showText(m_parent->globalPos()+tooltipEvent->pos(), m_paletteItem->toolTip());
				return true;
			} else {
				return QFrame::event(event);
			}
		}

		SvgIconWidget *m_parent;
		PaletteItem *m_paletteItem;
};

#endif /* SVGICONWIDGET_H_ */
