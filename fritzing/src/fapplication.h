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

#ifndef FAPPLICATION_H
#define FAPPLICATION_H

#include <QApplication>
#include <QTranslator>

class FApplication :
	public QApplication
{
	Q_OBJECT

public:
	FApplication(int & argc, char ** argv);
	~FApplication(void);

public:
	bool findTranslator(const QString & libPath);

	static void setOpenSaveFolder(const QString& path);
	static const QString openSaveFolder();
	static bool spaceBarIsPressed();

signals:
	void spaceBarIsPressedSignal(bool);

protected:
     bool eventFilter(QObject *obj, QEvent *event);

protected:
	static bool m_spaceBarIsPressed;
	static QString m_openSaveFolder;
	QTranslator m_translator;
};


#endif
