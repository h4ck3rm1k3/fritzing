#ifndef BETTERTRIGGERACTION_H
#define BETTERTRIGGERACTION_H

#include <QAction>

class BetterTriggerAction : public QAction
{
Q_OBJECT
public:
	BetterTriggerAction(const QString & text, QObject * parent);

protected slots:
	void self_triggered();
	
signals:
	void betterTriggered(QAction *);
};

class BetterTriggerViewLayerAction : public BetterTriggerAction {
Q_OBJECT

public:	
	BetterTriggerViewLayerAction(const QString & text, class ViewLayer * viewLayer, QObject * parent);
	
	class ViewLayer * viewLayer();
	
protected:
	class ViewLayer * m_viewLayer;
};

#endif
