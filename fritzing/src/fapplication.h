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
#include <QPixmap>

class FApplication :
	public QApplication
{
	Q_OBJECT

public:
	FApplication(int & argc, char ** argv);
	~FApplication(void);

public:
	int startup(int & argc, char ** argv);

	static void setOpenSaveFolder(const QString& path);
	static const QString openSaveFolder();
	static bool spaceBarIsPressed();
	
signals:
	void spaceBarIsPressedSignal(bool);
	
public slots:
	void preferences();


protected:
    bool eventFilter(QObject *obj, QEvent *event);
	bool event(QEvent *event);
	bool findTranslator(const QString & libPath);
	void loadNew(QString path);
	void loadOne(class MainWindow *, QString path, int loaded);
	void initSplash(class FSplashScreen & splash, int & progressIndex, QPixmap & pixmap);
	
protected:
	static bool m_spaceBarIsPressed;
	static QString m_openSaveFolder;
	static QTranslator m_translator;
	static class ReferenceModel * m_referenceModel;
	static class PaletteModel * m_paletteBinModel;
	static bool m_started;
	static QList<QString> m_filesToLoad;
	static QString m_libPath;

};


#endif
