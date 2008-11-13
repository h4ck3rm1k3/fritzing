#include "bettertriggeraction.h"
#include "debugdialog.h"
#include "viewlayer.h"

BetterTriggerAction::BetterTriggerAction( const QString & text, QObject * parent ) 
	: QAction(text, parent)
{
	connect(this, SIGNAL(triggered()), this, SLOT(self_triggered())   );	
}

void BetterTriggerAction::self_triggered() {
	emit betterTriggered(this);
}


BetterTriggerViewLayerAction::BetterTriggerViewLayerAction(const QString & text, ViewLayer * viewLayer, QObject * parent)
	: BetterTriggerAction(text, parent)
{
	m_viewLayer = viewLayer;
}

ViewLayer * BetterTriggerViewLayerAction::viewLayer() {
	return m_viewLayer;
}

