#include "eventeater.h"
#include "debugdialog.h"


EventEater::EventEater(QObject * parent) : QObject(parent) {
}

bool EventEater::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type()) {
		case QEvent::KeyPress:
		case QEvent::KeyRelease:			
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:			
		case QEvent::MouseButtonDblClick:
		case QEvent::HoverEnter:
		case QEvent::HoverLeave:
		case QEvent::WinEventAct:
		case QEvent::GraphicsSceneHoverEnter:
		case QEvent::GraphicsSceneHoverLeave:
		case QEvent::GraphicsSceneHoverMove:
		case QEvent::GraphicsSceneMouseDoubleClick:
		case QEvent::GraphicsSceneMouseMove:
		case QEvent::GraphicsSceneMousePress:
		case QEvent::GraphicsSceneMouseRelease:
		case QEvent::NonClientAreaMouseButtonDblClick:
		case QEvent::NonClientAreaMouseButtonPress:
		case QEvent::NonClientAreaMouseButtonRelease:
		case QEvent::NonClientAreaMouseMove:
			{
				bool gotOne = false;
				foreach (QWidget * widget, m_allowedWidgets) {
					for (QObject * parentObj = obj; parentObj != NULL; parentObj = parentObj->parent()) {
						if (parentObj == widget) {
							gotOne = true;
							break;
						}
					}
				}
				if (!gotOne) {
					// filter out the event
					return true;
				}
			}
			break;
		 default:
			 break;
    }

	return QObject::eventFilter(obj, event);
}

void EventEater::allowEventsIn(QWidget * widget) {
	m_allowedWidgets.append(widget);
}
