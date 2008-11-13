#include "console.h"
#include "debugdialog.h"


Console::Console( QWidget * parent )
	: QTextEdit(parent)
{
	// TODO: need to hook this up to more useful stuff
	//DebugDialog::connectToBroadcast(this, SLOT(receiveDebugBroadcast(const QString &, QObject *)));
}

void Console::receiveDebugBroadcast(const QString & message, QObject * ancestor) {
	bool gotOne = false;
	if (ancestor == NULL) {
		gotOne = this->isActiveWindow();
	}
	else {
		while (ancestor->parent() != NULL) {
			ancestor = ancestor->parent();
		}

		for (QObject * pa = this; pa; pa = pa->parent()) {
			if (pa == ancestor) {
				gotOne = true;
				break;
			}
		}
	}

	if (!gotOne) return;

	this->append(message);
}
