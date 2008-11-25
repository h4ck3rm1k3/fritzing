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
		ModelPart *modelPart() const;
		const QString &moduleID() const;
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
		QString m_moduleId;
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
