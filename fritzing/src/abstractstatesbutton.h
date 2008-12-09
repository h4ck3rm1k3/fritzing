/*
 * (c) Fachhochschule Potsdam
 */

#ifndef ABSTRACTSTATESBUTTON_H_
#define ABSTRACTSTATESBUTTON_H_

#include <QIcon>
#include <QMouseEvent>

class AbstractStatesButton {
protected:
	virtual QString imagePrefix() = 0;
	virtual QString imageSubfix() {
		return "_icon.png";
	}

	virtual void setIconAux(const QIcon & icon) = 0;
	virtual void setupIcons(const QString &imageName) {
		m_enabledIcon = QIcon(imagePrefix()+imageName+"Enabled"+imageSubfix());
		m_disabledIcon = QIcon(imagePrefix()+imageName+"Disabled"+imageSubfix());
		m_pressedIcon = QIcon(imagePrefix()+imageName+"Pressed"+imageSubfix());
	}

	void setEnabledIcon() {
		setIconAux(m_enabledIcon);
	}
	void setDisabledIcon() {
		setIconAux(m_disabledIcon);
	}
	void setPressedIcon() {
		setIconAux(m_pressedIcon);
	}

protected:
	QIcon m_enabledIcon;
	QIcon m_disabledIcon;
	QIcon m_pressedIcon;
};

#endif /* ABSTRACTSTATESBUTTON_H_ */
