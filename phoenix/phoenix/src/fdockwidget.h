#ifndef FDOCKWIDGET_H
#define FDOCKWIDGET_H

#include <QDockWidget>
#include <QEvent>
#include <QSettings>

class FDockWidget : public QDockWidget
{
Q_OBJECT
public:
	FDockWidget(const QString & title, QWidget * parent = 0);
	//QSize sizeHint() const;

	void saveState();
	void restoreState();

protected:
	void changeEvent(QEvent *event);

signals:
	void dockChangeActivationSignal(FDockWidget *);

protected:
	bool m_state;
	QString m_name;
};
#endif
