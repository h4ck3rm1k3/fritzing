#include "bettertimer.h"
#include "debugdialog.h"


BetterTimer::BetterTimer( QObject * parent ) 
	: QTimer(parent)
{
	connect(this, SIGNAL(timeout()), this, SLOT(self_timeout())   );	
}

void BetterTimer::self_timeout() {
	emit betterTimeout(this);
}

