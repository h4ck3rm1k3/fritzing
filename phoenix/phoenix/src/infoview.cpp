#include "infoview.h"
#include "infographicsview.h"
#include "debugdialog.h"
#include "connectorstuff.h"
#include "connector.h"

InfoView::InfoView( QWidget * parent )
	: QTextEdit(parent)
{
	QFile styleSheet(":/resources/styles/infoview.css");

	if (!styleSheet.open(QIODevice::ReadOnly)) {
		qWarning("Unable to open :/resources/styles/infoview.css");
	} else {	
		document()->setDefaultStyleSheet(styleSheet.readAll());
	}
}

void InfoView::hoverEnterItem(ModelPart * modelPart) {
	QString s;
	s += "<table>";
	s += appendItemStuff(modelPart, 0);
	s += "</table>";
	setText(s);
}

void InfoView::hoverEnterItem(InfoGraphicsView * , QGraphicsSceneHoverEvent *, ItemBase * item) {
	QString s;
	s += "<table>";
	s += appendItemStuff(item, item->id());
	s += appendViewGeometry(item, false);
	s += appendCurrentGeometry(item, false);
	s += "</table>";
	setText(s);
}


void InfoView::hoverLeaveItem(ModelPart * modelPart) {
	Q_UNUSED(modelPart);
	//clear();
}


void InfoView::hoverLeaveItem(InfoGraphicsView * , QGraphicsSceneHoverEvent *, ItemBase * ){
	//clear();
}


void InfoView::hoverEnterConnectorItem(InfoGraphicsView * , QGraphicsSceneHoverEvent * , ConnectorItem * item) {
	Connector * connector = item->connector();
	if (connector == NULL) return;

	ConnectorStuff * connectorStuff = connector->connectorStuff();
	if (connectorStuff == NULL) return;

	QString s;
	s += "<table>";
	// Todo: make nice connector info here!
	s += QString("<tr><td>%1</td><td>%2, %3</td></tr>").arg(connectorStuff->id()).arg(connectorStuff->name()).arg(Connector::connectorNameFromType(connector->connectorType())) ;
	s += QString("<tr><td>conn.</td><td>connected to %1 item(s)</td></tr>").arg(item->connectionsCount())  ;

	if (item->attachedTo() != NULL) {
		s += appendItemStuff(item->attachedTo(), item->attachedToID());
	}
	s += "</table>";
	setText(s);
}

void InfoView::hoverLeaveConnectorItem(InfoGraphicsView * , QGraphicsSceneHoverEvent * , ConnectorItem * ) {
	//clear();
}


QString InfoView::appendViewGeometry(ItemBase * base, bool doLine) {
	if (base == NULL) return "missing base";

	ViewGeometry & viewGeometry = base->getViewGeometry();
	QString s = QString("<tr><td class='subhead' colspan='2'>Position</td></tr>") +
				QString("<tr><td class='label'>x</td><td>%1</td></tr>").arg(viewGeometry.loc().x()) + 
				QString("<tr><td class='label'>y</td><td>%1</td></tr>").arg(viewGeometry.loc().y());

	if (doLine) {
		s +=	QString("<tr><td class='subhead' colspan='2'>line</td></tr>") + 
				QString("<tr><td class='label'>x1 / y1</td><td>%1 / %2</td></tr>").arg(viewGeometry.line().x1()).
																				   arg(viewGeometry.line().y1());
				QString("<tr><td class='label'>x2 / y2</td><td>%1 / %2</td></tr>").arg(viewGeometry.line().x2()).
																				   arg(viewGeometry.line().y2());
	}
	
	s += 		QString("<tr><td class='label'>z</td><td>%1</td></tr>").arg(viewGeometry.z());
	s += 		QString("<tr><td class='label'>view</td><td>%1</td></tr>").arg(base->viewIdentifierName());
	
	return s;
}

QString InfoView::appendCurrentGeometry(ItemBase * item, bool doLine) {
	Q_UNUSED(doLine);

	if (item == NULL) return "missing item";

	QPointF loc = item->pos();
	QString s = QString("<tr><td class='subhead' colspan='2'>current geometry</td></tr>") +
				QString("<tr><td class='label'>x</td><td>%1</td></tr>").arg(loc.x()) +
				QString("<tr><td class='label'>y</td><td>%1</td></tr>").arg(loc.y());

	if (item->hasLine()) {
		QLineF line = item->line();
		s += QString("<tr><td class='subhead' colspan='2'>line</td></tr>") + 
			 QString("<tr><td class='label'>x1 / y1</td><td>%1,%2</td></tr>").arg(line.x1()).
		 																	  arg(line.y1());
		s += QString("<tr><td class='label'>x2 / y2</td><td>%1,%2</td></tr>").arg(line.x2()).
		 																	  arg(line.y2());	
	}

	s += 	 QString("<tr><td class='label'>z</td><td>%1</td></tr>").arg(item->zValue());
	return s;
}

QString InfoView::appendItemStuff(ItemBase* base, long id) {
	if (base == NULL) return "missing base";

	return appendItemStuff(base->modelPart(), id);
}

QString InfoView::appendItemStuff(ModelPart * modelPart, long id) {
	if (modelPart == NULL) return "missing modelpart";
	if (modelPart->modelPartStuff() == NULL) return "missing modelpart stuff";

	QString s = QString("<tr><td colspan='2' class='parttitle'>%1</td></tr>").
			arg(modelPart->modelPartStuff()->title());
	s += QString("<tr><td>%1</td><td>%2, id:%3</td></tr>")
			.arg(ModelPart::itemTypeName(modelPart->itemType()))
			.arg(modelPart->modelPartStuff()->moduleID())
			.arg(id);
	s += QString("<tr><td colspan='2'>%1</td></tr>").
			arg(modelPart->modelPartStuff()->path());
	if(!modelPart->modelPartStuff()->tags().isEmpty()) {
		s += QString("<tr><td class='subhead' colspan='2'>Tags</td></tr>") +
			 QString("<tr><td colspan='2'>%1</td></tr>").arg(modelPart->modelPartStuff()->tags().join(", "));
	}



	return s;
}

