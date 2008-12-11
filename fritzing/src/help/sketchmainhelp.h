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

$Revision: 1490 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 13:10:48 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

/*
 * Central sketch help
 */

#ifndef SKETCHMAINHELP_H_
#define SKETCHMAINHELP_H_

#include <QLabel>
#include <QFrame>
#include <QGraphicsProxyWidget>

class SketchMainHelpCloseButton : public QLabel {
	Q_OBJECT
	public:
		SketchMainHelpCloseButton(const QString &imagePath, QWidget *parent);
		void doShow();
		void doHide();

	signals:
		void clicked();

	protected:
		void mousePressEvent(QMouseEvent * event);
		QPixmap m_pixmap;
};

class SketchMainHelp;

class SketchMainHelpPrivate : public QFrame {
	Q_OBJECT

	public:
		SketchMainHelpPrivate(
				const QString &viewString, const QString &imagePath,
				const QString &htmlText, SketchMainHelp *parent);

	protected slots:
		void doClose();

	protected:
		void enterEvent(QEvent * event);
		void leaveEvent(QEvent * event);

	protected:
		friend class SketchMainHelp;

		SketchMainHelp *m_parent;
		SketchMainHelpCloseButton *m_closeButton;
		volatile bool m_shouldGetTransparent;
};

class SketchMainHelp : public QGraphicsProxyWidget {
public:
	SketchMainHelp(const QString &viewString, const QString &imagePath, const QString &htmlText);
	void doClose();
	void applyAlpha();

protected:
	SketchMainHelpPrivate *m_son;

public:
	static qreal OpacityLevel;
};

#endif /* SKETCHMAINHELP_H_ */
