#include <QtGui>

#include "fdockwidget.h"
#include "debugdialog.h"

FDockWidget::FDockWidget( const QString & title, QWidget * parent)
	: QDockWidget(title, parent)
{
	setObjectName(title.trimmed().toLower().remove(" "));
	setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
}

void FDockWidget::saveState() {
	m_state = this->isFloating() && this->isVisible();
}

void FDockWidget::restoreState() {
	if (m_state) {
		this->setVisible(true);
	}
}

void FDockWidget::changeEvent(QEvent * event) {
	if (event) {
		if (event->type() == QEvent::ActivationChange) {
			DebugDialog::debug(QObject::tr("change activation in dock"));
			emit dockChangeActivationSignal(this);
		}
	}
	QDockWidget::changeEvent(event);
}
