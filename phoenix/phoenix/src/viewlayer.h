#ifndef VIEWLAYER_H
#define VIEWLAYER_H

#include <QString>
#include <QAction>
#include <QHash>

#include "misc.h"

class ViewLayer
{
public:
	enum ViewLayerID {
		Icon,
		BreadboardBreadboard,
		Breadboard,
		BreadboardWire,
		BreadboardRuler,
		Schematic,
		SchematicWire,
		SchematicRuler,
		Board,
		Copper1,
		Copper0,
		Keepout,
		Vias,
		Soldermask,
		Silkscreen,
		Outline,
		Jumperwires,
		Ratsnest,
		PcbRuler,
		UnknownLayer,
		ViewLayerCount
	};

protected:
	static qreal zIncrement;
	static QHash<ViewLayerID, StringPair *> names;

public:
	ViewLayer(ViewLayerID, bool visible, qreal initialZ);

	void setAction(QAction *);
	QAction* action();
	QString & displayName();
	bool visible();
	void setVisible(bool);
	qreal nextZ();
	ViewLayerID viewLayerID();
	qreal incrementZ(qreal);

	static ViewLayerID viewLayerIDFromString(const QString &);
	static ViewLayerID viewLayerIDFromXmlString(const QString &);
	static const QString & viewLayerNameFromID(ViewLayerID);
	static const QString & viewLayerXmlNameFromID(ViewLayerID);
	static void initNames();
	static qreal getZIncrement();

protected:
	bool m_visible;
	ViewLayerID m_viewLayerID;
	QAction* m_action;
	qreal m_nextZ;
};


typedef QHash<ViewLayer::ViewLayerID, ViewLayer *> LayerHash;
#endif
