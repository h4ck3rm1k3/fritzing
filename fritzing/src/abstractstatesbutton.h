/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/



#ifndef ABSTRACTSTATESBUTTON_H_
#define ABSTRACTSTATESBUTTON_H_

#include <QIcon>
#include <QMouseEvent>

class AbstractStatesButton {
public:
	virtual ~AbstractStatesButton() {}			// clears compiler warning
	void setEnabledIcon() {
		setImage(m_enabledImage);
	}
	void setDisabledIcon() {
		setImage(m_disabledImage);
	}
	void setPressedIcon() {
		setImage(m_pressedImage);
	}

protected:
	virtual QString imagePrefix() = 0;
	virtual QString imageSubfix() {
		return "_icon.png";
	}

	virtual void setImage(const QPixmap & pixmap) = 0;
	virtual void setupIcons(const QString &imageName) {
		m_enabledImage = QPixmap(imagePrefix()+imageName+"Enabled"+imageSubfix());
		m_disabledImage = QPixmap(imagePrefix()+imageName+"Disabled"+imageSubfix());
		m_pressedImage = QPixmap(imagePrefix()+imageName+"Pressed"+imageSubfix());
	}

protected:
	QPixmap m_enabledImage;
	QPixmap m_disabledImage;
	QPixmap m_pressedImage;
};

#endif /* ABSTRACTSTATESBUTTON_H_ */
